#include <assert.h>
#include <ctype.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <zlib.h>

#include "common.h"
#include "rpl.h"

void free_elf(Elf *elf) {
   Section *section = elf->sections;
   while (section = section->next)
      free(section->data);
   free(elf->sections);
   free(elf);
}

void read_section(FILE *fp, Section *sec) {
   if (sec->data)
      return;

   if (!sec->header.offset || !sec->header.size || sec->header.type == SHT_NOBITS) {
      sec->header.offset = 0;
      return;
   }

   sec->data = malloc(sec->header.size * (sec->header.type == SHT_RELA ? 2 : 1));
   fseek(fp, sec->header.offset, SEEK_SET);
   fread(sec->data, sec->header.size, 1, fp);

   if (sec->header.flags & SHF_RPL_ZLIB) {
      sec->header.flags &= ~SHF_RPL_ZLIB;
      CompressedData *cmpr = (CompressedData *)sec->data;
      assert(cmpr->deflated_size);
      void *data_out = malloc(cmpr->deflated_size * (sec->header.type == SHT_RELA ? 2 : 1));

      z_stream zs = {};
      zs.next_in = cmpr->compressed_data;
      zs.avail_in = sec->header.size - 4;
      zs.next_out = data_out;
      zs.avail_out = cmpr->deflated_size;

      inflateInit(&zs);
      inflate(&zs, Z_FINISH);
      inflateEnd(&zs);
      assert(!zs.avail_in);
      assert(!zs.avail_out);

      sec->header.size = cmpr->deflated_size;

      free(sec->data);
      sec->data = data_out;
   }
}

Elf *read_elf(const char *filename) {
   FILE *fp = fopen(filename, "rb");
   Elf *elf = calloc(1, sizeof(*elf));
   fread(elf, sizeof(elf->header), 1, fp);
   elf->is_rpl = elf->header.os_abi[0] == 0xCA && elf->header.os_abi[1] == 0xFE;

   elf->info.magic = 0xCAFE;
   elf->info.version = 0x0402u;
   elf->info.SizeCoreStacks = 0x10000u;
   elf->info.SysHeapBytes = 0x8000u;
   elf->info.minVersion = 0x5078u;
   elf->info.compressionLevel = 6;

   elf->sections = calloc(elf->header.shnum + 2, sizeof(*elf->sections));

   assert(elf->header.shentsize == sizeof(elf->sections->header));
   for (int i = 0; i < elf->header.shnum; i++) {
      Section *sec = elf->sections + i;
      fseek(fp, elf->header.shoff + i * elf->header.shentsize, SEEK_SET);
      fread(sec, 1, elf->header.shentsize, fp);
   }

   read_section(fp, &elf->sections[elf->header.shstrndx]);
   elf->shstrtab = elf->sections[elf->header.shstrndx].data;

   Symbol *got = NULL;
   Section *relas = NULL;

   Section *last_data = NULL;

   for (int i = 0; i < elf->header.shnum; i++) {
      Section *sec = elf->sections + i;

      sec->name = elf->shstrtab + sec->header.name;
      if (sec->header.link)
         sec->link = elf->sections + sec->header.link;

      switch (sec->header.type) {
      case SHT_PROGBITS:
      case SHT_NOBITS:
         if (!sec->header.addr)
            continue;

         read_section(fp, sec);
         last_data = sec;

         if (sec->header.addr >= 0x10000000 && sec->header.addr < 0xC0000000) {
            sec->header.flags |= SHF_WRITE;
            sec->header.flags &= ~SHF_CODE;
         }
         if (sec->header.flags & SHF_CODE) {
            assert(sec->header.addr >= 0x02000000);
            if (elf->info.TextSize < sec->header.addr + sec->header.size - 0x02000000)
               elf->info.TextSize = sec->header.addr + sec->header.size - 0x02000000;
            if (elf->info.TextAlign < sec->header.align)
               elf->info.TextAlign = sec->header.align;
         } else {
            if (elf->info.DataSize < sec->header.addr + sec->header.size - 0x10000000) {
               assert(sec->header.addr >= 0x10000000);
               elf->info.DataSize = sec->header.addr + sec->header.size - 0x10000000;
            }
            if (elf->info.DataAlign < sec->header.align)
               elf->info.DataAlign = sec->header.align;
         }

         if (sec->header.flags & SHF_TLS) {
            sec->header.flags &= ~SHF_TLS;
            sec->header.flags |= SHF_RPL_TLS;
            elf->info.Flags |= FIF_TLS;
            if ((1 << elf->info.tlsAlignShift) < sec->header.align)
               elf->info.tlsAlignShift = log2(sec->header.align);
         }

         break;
      case SHT_RELA: {
         Section *symtab = elf->sections + sec->header.link;
         Section *target = elf->sections + sec->header.info;
         Section *strtab = elf->sections + symtab->header.link;

         sec->link2 = target;
         if (!target->header.addr || target->header.type == SHT_RPL_IMPORTS)
            continue;

         sec->header.flags &= ~SHF_INFO_LINK;

         if (!relas)
            relas = sec;
         else {
            Section *rela = relas;
            while (rela->next)
               rela = rela->next;
            rela->next = sec;
            sec->prev = rela;
         }
         read_section(fp, sec);
         read_section(fp, symtab);
         read_section(fp, strtab);
         read_section(fp, target);

         assert(sizeof(Relocation) == sec->header.entsize);
         assert(!(sec->header.size % sizeof(Relocation)));
         for (int i = 0; i < sec->header.size / sizeof(Relocation); i++) {
            Relocation *rel = (Relocation *)sec->data + i;
            switch (rel->type) {
            case R_PPC_NONE:
            case R_PPC_ADDR32:
            case R_PPC_ADDR16_LO:
            case R_PPC_ADDR16_HI:
            case R_PPC_ADDR16_HA:
            case R_PPC_REL24:
            case R_PPC_REL14:
            case R_PPC_DTPMOD32:
            case R_PPC_DTPREL32:
            case R_PPC_EMB_SDA21:
            case R_PPC_EMB_RELSDA:
            case R_PPC_DIAB_SDA21_LO:
            case R_PPC_DIAB_SDA21_HI:
            case R_PPC_DIAB_SDA21_HA:
            case R_PPC_DIAB_RELSDA_LO:
            case R_PPC_DIAB_RELSDA_HI:
            case R_PPC_DIAB_RELSDA_HA:
            case R_PPC_GHS_REL16_HA:
            case R_PPC_GHS_REL16_HI:
            case R_PPC_GHS_REL16_LO: break;

            case R_PPC_REL32: {
               Relocation *new_rel = (Relocation *)((u8 *)sec->data + sec->header.size);
               sec->header.size += sizeof(Relocation);
               rel->type = R_PPC_GHS_REL16_HI;
               new_rel->type = R_PPC_GHS_REL16_LO;
               new_rel->index = rel->index;
               new_rel->offset = rel->offset + 2;
               new_rel->addend = rel->addend + 2;
               break;
            }

            case R_PPC_TLSGD:
            case R_PPC_GOT_TLSGD16: {
               if (!got) {
                  for (int i = 0; i < symtab->header.size / sizeof(Symbol); i++) {
                     Symbol *sym = (Symbol *)symtab->data + i;
                     if (!strcmp(strtab->data + sym->name, "_GLOBAL_OFFSET_TABLE_")) {
                        got = sym;
                        break;
                     }
                  }
               }
               assert(got && got->shndx == sec->header.info);

               s16_be *tls_index_offset = (s16_be *)(target->data + rel->offset - target->header.addr);
               if (rel->type == R_PPC_GOT_TLSGD16) {
                  rel->type = R_PPC_DTPMOD32;
                  rel->offset = got->value + tls_index_offset->val;
                  //                  u32_be *moduleIndex = (u32_be *)(target->data + rel->offset -
                  //                  target->header.addr); moduleIndex->val = elf->info.tlsModuleIndex; /* not needed
                  //                  */
               } else {
                  tls_index_offset--;
                  rel->type = R_PPC_DTPREL32;
                  rel->offset = got->value + tls_index_offset->val + 4;
                  //                  u32_be *symValue = (u32_be *)(target->data + rel->offset - target->header.addr);
                  //                  symValue->val = ((Symbol *)symtab->data + rel->index)->value; /* not needed */
               }
               break;
            }

            default:
               fprintf(stderr, "unsupported relocation #%i(%s)\n", rel->type, ElfRelocation_to_str(rel->type));
               exit(1);
            }
         }

         elf->info.TempSize += align_up(sec->header.size, 32);
         continue;
      }
      case SHT_STRTAB:
      case SHT_SYMTAB:
         read_section(fp, sec);
         if (!sec->header.addr) {
            assert(sec->header.align);
            sec->header.flags |= SHF_ALLOC;
            sec->header.addr = align_up(elf->info.LoaderSize + 0xC0000000, sec->header.align);
            elf->info.LoaderSize = sec->header.addr + sec->header.size - 0xC0000000;
            if (elf->info.LoaderAlign < sec->header.align)
               elf->info.LoaderAlign = sec->header.align;
         }
         break;
      case SHT_RPL_IMPORTS:
      case SHT_RPL_EXPORTS:
         read_section(fp, sec);
         if (elf->info.LoaderSize < align_up(sec->header.addr + sec->header.size - 0xC0000000, 0x40))
            elf->info.LoaderSize = align_up(sec->header.addr + sec->header.size - 0xC0000000, 0x40);
         if (elf->info.LoaderAlign < sec->header.align)
            elf->info.LoaderAlign = sec->header.align;
         break;
      case SHT_RPL_CRCS:
      case SHT_RPL_FILEINFO: read_section(fp, sec); break;
      default: break;
      }
      //      read_section(fp, sec);

      if (i > 0) {
         Section *last = elf->sections;
         while (last->next)
            last = last->next;
         last->next = sec;
         sec->prev = last;
      }
   }
   fclose(fp);

      elf->info.TextSize *= 2;
      elf->info.LoaderSize *= 2;
      elf->info.DataSize *= 2;
      elf->info.TempSize *= 2;

   if (!relas) {
      assert(!elf->is_rpl);
      fprintf(stderr, "Relocations missing, recompile with -Wl,--emit-relocs\n");
      exit(1);
   }

   Section *sec = relas;
   while (sec->next)
      sec = sec->next;
   sec->next = last_data->next;
   sec->next->prev = sec;

   relas->prev = last_data;
   last_data->next = relas;

   sec = elf->sections;
   while (sec = sec->next) {
      if (sec->link)
         sec->header.link = get_sid(sec->link);
      if (sec->link2)
         sec->header.info = get_sid(sec->link2);
   }

   int new_ids[elf->header.shnum];
   for (int i = 0; i < elf->header.shnum; i++)
      new_ids[i] = get_sid(elf->sections + i);

   sec = elf->sections;
   while (sec = sec->next) {
      if (sec->header.type != SHT_SYMTAB)
         continue;

      assert(sec->header.entsize == sizeof(Symbol));
      for (int i = 0; i < (sec->header.size / sizeof(Symbol)); i++) {
         Symbol *sym = (Symbol *)sec->data + i;
         if (sym->shndx < elf->header.shnum)
            sym->shndx = new_ids[sym->shndx];
      }
   }

   if (elf->is_rpl) {
      return elf;
   }

   fflush(stdout);

   int SDAStart = 0;
   int SDA2Start = 0;
   Section *sdata = get_section_by_name(elf, ".sdata");
   Section *sdata2 = get_section_by_name(elf, ".sdata2");
   Section *sbss = get_section_by_name(elf, ".sbss");

   if (true || sdata || sdata2 || sbss) // || __rpx_entry || main && !__rpl_entry
   {
      elf->info.Flags |= FIF_RPX;
      SDAStart = sdata ? sdata->header.addr : sbss ? sbss->header.addr : (0x10000000 + elf->info.DataSize);
      SDA2Start = sdata2 ? sdata2->header.addr : 0x10000000;
   }

   elf->info.SDABase = SDAStart + 0x8000;
   elf->info.SDA2Base = SDA2Start + 0x8000;

   Section *crcs = elf->sections + elf->header.shnum++;
   Section *info_section = elf->sections + elf->header.shnum++;

   Section *last = elf->sections;
   while (last->next)
      last = last->next;

   last->next = crcs;
   last->next->prev = last;
   crcs->next = info_section;
   crcs->next->prev = crcs;

   info_section->header.type = SHT_RPL_FILEINFO;
   info_section->header.align = 4;
   info_section->header.size = sizeof(FileInfo);
   info_section->data = calloc(1, info_section->header.size);
   memcpy(info_section->data, &elf->info, sizeof(FileInfo));
   info_section->name = "";

   crcs->header.type = SHT_RPL_CRCS;
   crcs->header.align = 4;
   crcs->header.entsize = 4;
   crcs->header.size = get_section_count(elf) * crcs->header.entsize;
   crcs->data = calloc(1, crcs->header.size);
   crcs->name = "";

   sec = elf->sections;
   u32_be *crc = (u32_be *)crcs->data + 1;
   while (sec = sec->next)
      (crc++)->val = (sec->header.type == SHT_RPL_CRCS) ? 0 : crc32(0, sec->data, sec->header.size);

   printf("crcs updated !\n");
   return elf;
}

void write_sections(FILE *fp, Section *sec, SectionType type, SectionFlags required_flags) {
   assert(type != SHT_NOBITS);
   while (sec = sec->next) {
      if (sec->header.type == type && ((sec->header.flags & required_flags) == required_flags)) {
         assert(!sec->header.offset);
         if (!sec->header.size)
            continue;
         sec->header.offset = ftell(fp);
         fwrite(sec->data, 1, sec->header.size, fp);
         while (ftell(fp) & 0x3F)
            fputc('\0', fp);
      }
   }
}

void write_elf(Elf *elf, const char *filename, bool plain) {
   Section *sec = elf->sections;
   while (sec = sec->next) {
      sec->header.offset = 0;
      if (!sec->data) {
         assert(!(sec->header.flags & SHF_RPL_ZLIB));
         continue;
      }

      bool compressed = !plain && sec->header.size > 0x18;
      compressed = compressed && sec->header.type != SHT_RPL_FILEINFO;
      compressed = compressed && sec->header.type != SHT_RPL_CRCS;
      compressed = compressed && sec != &elf->sections[elf->header.shstrndx];

      if (compressed) {
         sec->header.flags |= SHF_RPL_ZLIB;
         CompressedData *data_out = malloc(sec->header.size + 4);
         data_out->deflated_size = sec->header.size;

         z_stream s = { 0 };
         s.next_in = sec->data;
         s.avail_in = sec->header.size;
         s.next_out = data_out->compressed_data;
         s.avail_out = sec->header.size;

         deflateInit(&s, elf->info.compressionLevel);
         deflate(&s, Z_FINISH);
         deflateEnd(&s);
         assert(!s.avail_in);
         assert(s.avail_out);

         sec->header.size = (u8 *)s.next_out - (u8 *)data_out;

         free(sec->data);
         sec->data = (u8 *)data_out;
      } else
         assert(!(sec->header.flags & SHF_RPL_ZLIB));
   }

   ElfHeader header = {};
   header.magic = ELF_MAGIC;
   header.elf_class = EC_32BIT;
   header.data_encoding = ED_2MSB;
   header.elf_version = EV_CURRENT;
   header.os_abi[0] = 0xCA;
   header.os_abi[1] = 0xFE;
   header.type = ET_CAFE;
   header.machine = EM_PPC;
   header.version = EV_CURRENT;
   header.entry = elf->header.entry;
   header.shoff = align_up(sizeof(header), 0x40);
   header.ehsize = sizeof(header);
   header.shentsize = sizeof(SectionHeader);
   header.shstrndx = get_sid(get_section_by_name(elf, ".shstrtab"));
   header.shnum = get_section_count(elf);
   if (elf->is_rpl)
      assert(!memcmp(&elf->header, &header, sizeof(header)));
   else
      elf->header = header;

   FILE *fp = fopen(filename, "wb");
   fwrite(elf, sizeof(elf->header), 1, fp);

   fseek(fp, elf->header.shoff + elf->header.shnum * elf->header.shentsize, SEEK_SET);
   write_sections(fp, elf->sections, SHT_RPL_CRCS, 0);
   write_sections(fp, elf->sections, SHT_RPL_FILEINFO, 0);
   write_sections(fp, elf->sections, SHT_PROGBITS, SHF_ALLOC | SHF_WRITE);
   write_sections(fp, elf->sections, SHT_RPL_EXPORTS, SHF_ALLOC);
   write_sections(fp, elf->sections, SHT_RPL_IMPORTS, SHF_ALLOC);
   write_sections(fp, elf->sections, SHT_SYMTAB, SHF_ALLOC);
   write_sections(fp, elf->sections, SHT_STRTAB, SHF_ALLOC);
   write_sections(fp, elf->sections, SHT_PROGBITS, SHF_ALLOC | SHF_CODE);
   write_sections(fp, elf->sections, SHT_RELA, 0);

   fseek(fp, elf->header.shoff, SEEK_SET);
   sec = elf->sections;
   do
      fwrite(sec, elf->header.shentsize, 1, fp);
   while (sec = sec->next);

   fclose(fp);
   printf("output written to %s\n", filename);
}
