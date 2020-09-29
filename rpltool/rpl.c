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

u32 get_region_size(Elf *elf, u32 min_addr, u32 max_addr) {
   u32 size = 0;
   Section *s = elf->sections;
   while (s = s->next) {
      if (s->header.addr >= min_addr && (s->header.addr + s->header.size) < max_addr)
         if (size < s->header.addr + s->header.size - min_addr)
            size = s->header.addr + s->header.size - min_addr;
   }
   return size;
}

u32 get_region_align(Elf *elf, u32 min_addr, u32 max_addr) {
   u32 align = 1;
   Section *s = elf->sections;
   while (s = s->next) {
      if (s->header.addr >= min_addr && (s->header.addr + s->header.size) < max_addr)
         if (align < s->header.addralign)
            align = s->header.addralign;
   }
   return align;
}

void free_elf(Elf *elf) {
   Section *section = elf->sections;
   while (section = section->next)
      free(section->data);
   free(elf->sections);
   free(elf);
}

void read_section(FILE *fp, Section *section) {
   if (!section->header.offset || !section->header.size) {
      section->header.offset = 0;
      return;
   }

   section->data = malloc(section->header.size * (section->header.type == SHT_RELA ? 2 : 1));
   fseek(fp, section->header.offset, SEEK_SET);
   fread(section->data, section->header.size, 1, fp);

   if (section->header.flags & SHF_RPL_ZLIB) {
      CompressedData *cmpr = section->data;
      assert(cmpr->deflated_size);
      void *data_out = malloc(cmpr->deflated_size * (section->header.type == SHT_RELA ? 2 : 1));

      z_stream s = {};
      s.next_in = cmpr->compressed_data;
      s.avail_in = section->header.size - 4;
      s.next_out = data_out;
      s.avail_out = cmpr->deflated_size;

      inflateInit(&s);
      inflate(&s, Z_FINISH);
      inflateEnd(&s);
      assert(!s.avail_in);
      assert(!s.avail_out);

      section->header.size = cmpr->deflated_size;

      free(section->data);
      section->data = data_out;
   }
}

Elf *read_elf(const char *filename) {
   FILE *fp = fopen(filename, "rb");
   Elf *elf = calloc(1, sizeof(*elf));
   fread(elf, sizeof(elf->header), 1, fp);
   elf->is_rpl = elf->header.os_abi[0] == 0xCA && elf->header.os_abi[1] == 0xFE;

   elf->sections = calloc(elf->header.shnum + 2, sizeof(*elf->sections));

   assert(elf->header.shentsize == sizeof(elf->sections->header));
   for (int i = 0; i < elf->header.shnum; i++) {
      fseek(fp, elf->header.shoff + i * elf->header.shentsize, SEEK_SET);
      fread(&elf->sections[i], 1, elf->header.shentsize, fp);
   }

   read_section(fp, &elf->sections[elf->header.shstrndx]);
   elf->shstrtab = elf->sections[elf->header.shstrndx].data;
   for (int i = 0; i < elf->header.shnum + 2; i++) {
      elf->sections[i].name = elf->shstrtab + elf->sections[i].header.name;
   }

   for (int i = 0; i < elf->header.shnum - 1; i++) {
      elf->sections[i].next = &elf->sections[i + 1];
      elf->sections[i].next->prev = &elf->sections[i];
   }

   Section *sec = elf->sections;
   while (sec = sec->next) {
      if (sec->header.link)
         sec->link = get_section(elf, sec->header.link);
      if (sec->header.type == SHT_RELA)
         sec->link2 = get_section(elf, sec->header.info);
   }

   Section *relas = NULL;
   Section *last_rela = NULL;
   u32 loader_max_addr = 0;
   bool has_tls = false;
   sec = elf->sections;
   while (sec = sec->next) {
      if (!memcmp(sec->name, ".debug", 6) || !memcmp(sec->name, ".rela.debug", 11) ||
          !memcmp(sec->name, ".rela.fimport", 12) || !memcmp(sec->name, ".rela.dimport", 12)) {
         sec->prev->next = sec->next;
         if (sec->next)
            sec->next->prev = sec->prev;
         sec->prev = NULL;
         continue;
      }

      if (!sec->data)
         read_section(fp, sec);

      if (sec->header.addr > 0xC0000000 && loader_max_addr < align_up(sec->header.addr + sec->header.size, 64))
         loader_max_addr = align_up(sec->header.addr + sec->header.size, 64);

      if (sec->header.addr >= 0x10000000 && sec->header.addr < 0xC0000000) {
         if (elf->is_rpl)
            assert(sec->header.flags & SHF_WRITE);
         else {
            sec->header.flags |= SHF_WRITE;
            sec->header.flags &= ~SHF_CODE;
         }
      }

      if (sec->header.flags & SHF_TLS) {
         sec->header.flags &= ~SHF_TLS;
         sec->header.flags |= SHF_RPL_TLS;
         has_tls = true;
      }

      switch (sec->header.type) {
      case SHT_RPL_FILEINFO: elf->info = sec->data; break;
      case SHT_RPL_CRCS: elf->crcs = sec->data; break;
      case SHT_RELA: {
         sec->header.flags = 0;
         sec->prev->next = sec->next;
         if (sec->next)
            sec->next->prev = sec->prev;
         if (last_rela) {
            last_rela->next = sec;
            sec->prev = last_rela;
            last_rela = sec;
         } else {
            last_rela = relas = sec;
            last_rela->prev = NULL;
         }
         break;
      }
      case SHT_STRTAB:
      case SHT_SYMTAB:
         if (elf->is_rpl)
            assert(sec->header.addr);
         else {
            assert(sec->header.addralign);
            sec->header.addr = align_up(loader_max_addr, sec->header.addralign);
            loader_max_addr = sec->header.addr + sec->header.size;
            sec->header.flags |= SHF_ALLOC;
         }
         break;
      default: break;
      }
   }
   fclose(fp);

   elf->strtab = get_section_by_name(elf, ".strtab")->data;

   if (!relas) {
      assert(!elf->is_rpl);
      printf("Relocations missing, recompile with -Wl,--emit-relocs\n");
      exit(1);
   }

   Section *last_data = NULL;
   Section *first_import = NULL;

   sec = elf->sections;
   while (sec = sec->next) {
      if (sec->header.addr >= 0x2000000 && sec->header.addr < 0xC0000000) {
         last_data = sec;
      } else if (sec->header.type == SHT_RPL_IMPORTS && !first_import) {
         first_import = sec;
      }
   }
   if (!first_import)
      first_import = last_data->next;

   last_data->next = relas;
   relas->prev = last_data;

   first_import->prev = last_rela;
   last_rela->next = first_import;

   sec = elf->sections;
   while (sec = sec->next) {
      if (sec->link)
         sec->header.link = get_sid(sec->link);
      if (sec->link2)
         sec->header.info = get_sid(sec->link2);
   }
   int section_lut[elf->header.shnum];
   for (int i = 0; i < elf->header.shnum; i++)
      section_lut[i] = get_sid(elf->sections + i);

   Section *symtab = get_section_by_name(elf, ".symtab");
   for (int i = 0; i < (symtab->header.size / symtab->header.entsize); i++) {
      Symbol *sym = (Symbol *)symtab->data + i;
      if (sym->shndx < elf->header.shnum)
         sym->shndx = section_lut[sym->shndx];
   }

   if (elf->is_rpl) {
      assert(elf->crcs);
      assert(elf->info);
      return elf;
   }

   sec = elf->sections;
   while (sec = sec->next) {
      if (sec->header.type != SHT_RELA)
         continue;

      assert(sizeof(Relocation) == sec->header.entsize);
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
         case R_PPC_GHS_REL16_HI:
         case R_PPC_GHS_REL16_LO: break;
         case R_PPC_REL32: {
            Relocation *new_rel = (Relocation *)((u8 *)sec->data + sec->header.size);
            sec->header.size += sizeof(Relocation);
            *new_rel = *rel;
            rel->type = R_PPC_GHS_REL16_HI;
            new_rel->type = R_PPC_GHS_REL16_LO;
            new_rel->offset += 2;
            new_rel->addend += 2;
            break;
         }
         case R_PPC_TLSGD:
         case R_PPC_GOT_TLSGD16: {
            Section *text = get_section(elf, sec->header.info);
            assert(!strcmp(text->name, ".text"));
            u32 got_addr = 0;
            for (int i = 0; i < symtab->header.size / sizeof(Symbol); i++) {
               Symbol *sym = (Symbol *)symtab->data + i;
               if (!strcmp(elf->strtab + sym->name, "_GLOBAL_OFFSET_TABLE_")) {
                  got_addr = sym->value;
                  assert(sym->shndx == get_sid(text));
                  break;
               }
            }
            assert(got_addr);

            s16_be *got_offset = (s16_be *)((u8 *)text->data + rel->offset - text->header.addr);
            if (rel->type == R_PPC_GOT_TLSGD16) {
               rel->type = R_PPC_DTPMOD32;
               rel->offset = got_addr + got_offset->val;
               // *(u32_be *)((u8 *)text->data + rel->offset - text->header.addr)->val = info->tlsModuleIndex;
            } else {
               got_offset--;
               rel->type = R_PPC_DTPREL32;
               rel->offset = got_addr + got_offset->val + 4;
               // *(u32_be *)((u8 *)text->data + rel->offset - text->header.addr)->val = symtab[rel.index].value;
            }
            break;
         }
         case R_PPC_TLS:
         case R_PPC_TPREL16:
         case R_PPC_TPREL16_LO:
         case R_PPC_TPREL16_HI:
         case R_PPC_TPREL16_HA:
         case R_PPC_TPREL32:
         case R_PPC_DTPREL16:
         case R_PPC_DTPREL16_LO:
         case R_PPC_DTPREL16_HI:
         case R_PPC_DTPREL16_HA:
         case R_PPC_GOT_TLSGD16_LO:
         case R_PPC_GOT_TLSGD16_HI:
         case R_PPC_GOT_TLSGD16_HA:
         case R_PPC_GOT_TLSLD16:
         case R_PPC_GOT_TLSLD16_LO:
         case R_PPC_GOT_TLSLD16_HI:
         case R_PPC_GOT_TLSLD16_HA:
         case R_PPC_GOT_TPREL16:
         case R_PPC_GOT_TPREL16_LO:
         case R_PPC_GOT_TPREL16_HI:
         case R_PPC_GOT_TPREL16_HA:
         case R_PPC_GOT_DTPREL16:
         case R_PPC_GOT_DTPREL16_LO:
         case R_PPC_GOT_DTPREL16_HI:
         case R_PPC_GOT_DTPREL16_HA:
         case R_PPC_TLSLD:
            printf("unsupported TLS relocation: %s\n"
                   " - define '__thread' as '__thread __attribute((tls_model(\"global-dynamic\")))'\n"
                   " - recompile with '-ftls-model=global-dynamic'\n"
                   " - link with '-Wl,--no-tls-optimize'\n",
                   ElfRelocation_to_str(rel->type));
            exit(1);
            break;
         default: printf("unsupported relocation: %s\n", ElfRelocation_to_str(rel->type)); exit(1);
         }
      }
   }

   fflush(stdout);

   Section *crcs = elf->sections + elf->header.shnum++;
   Section *file_info = elf->sections + elf->header.shnum++;

   Section *last = get_section(elf, get_section_count(elf) - 1);
   last->next = crcs;
   last->next->prev = last;
   crcs->next = file_info;
   crcs->next->prev = crcs;

   crcs->header.type = SHT_RPL_CRCS;
   crcs->header.addralign = 4;
   crcs->header.entsize = 4;
   crcs->header.size = get_section_count(elf) * crcs->header.entsize;
   crcs->data = calloc(1, crcs->header.size);
   elf->crcs = crcs->data;

   file_info->header.type = SHT_RPL_FILEINFO;
   file_info->header.addralign = 4;
   file_info->header.size = sizeof(FileInfo);
   file_info->data = calloc(1, file_info->header.size);
   elf->info = file_info->data;

   elf->info->magic = 0xCAFE;
   elf->info->version = 0x0402u;
   elf->info->SizeCoreStacks = 0x10000u;
   elf->info->SysHeapBytes = 0x8000u;
   elf->info->minVersion = 0x5078u;
   elf->info->compressionLevel = 6;

   Section *text = get_section_by_name(elf, ".text");
   elf->info->RegBytes.Text = align_up(text->header.size, 16);
   elf->info->RegBytes.TextAlign = text->header.addralign;

   elf->info->RegBytes.Data = get_region_size(elf, 0x10000000, 0xC0000000);
   elf->info->RegBytes.DataAlign = get_region_align(elf, 0x10000000, 0xC0000000);

   elf->info->RegBytes.LoaderInfo = get_region_size(elf, 0xC0000000, 0xE0000000);
   elf->info->RegBytes.LoaderInfoAlign = get_region_align(elf, 0xC0000000, 0xE0000000);

   sec = elf->sections;
   while (sec = sec->next) {
      if (sec->header.type == SHT_RELA)
         elf->info->RegBytes.Temp += align_up(sec->header.size, 32);
   }
   elf->info->RegBytes.Temp = align_up(elf->info->RegBytes.Temp, 0x1000);

   int SDAStart = 0;
   int SDA2Start = 0;
   Section *sdata = get_section_by_name(elf, ".sdata");
   Section *sdata2 = get_section_by_name(elf, ".sdata2");
   Section *sbss = get_section_by_name(elf, ".sbss");

   if (true || sdata || sdata2 || sbss) // || __rpx_entry || main && !__rpl_entry
   {
      elf->info->Flags |= FIF_RPX;
      SDAStart = sdata ? sdata->header.addr : sbss ? sbss->header.addr : (0x10000000 + elf->info->RegBytes.Data);
      SDA2Start = sdata2 ? sdata2->header.addr : 0x10000000;
   }

   elf->info->SDABase = SDAStart + 0x8000;
   elf->info->SDA2Base = SDA2Start + 0x8000;

   int tlsAlign = 0;
   if (has_tls) {
      elf->info->Flags |= FIF_TLS;
      tlsAlign = 1;
      sec = elf->sections;
      while (sec = sec->next)
         if ((sec->header.flags & SHF_RPL_TLS) && (tlsAlign < sec->header.addralign))
            tlsAlign = sec->header.addralign;
   }
   elf->info->tlsAlignShift = log2(tlsAlign);

   sec = elf->sections;
   while (sec = sec->next)
      if (sec->header.type != SHT_RPL_CRCS)
         elf->crcs[get_sid(sec)].val = crc32(0, sec->data, sec->header.size);

   return elf;
}

int add_sections(Elf *elf, int pos, SectionType type, SectionFlags required_flags) {
   Section *section = elf->sections;
   assert(type != SHT_NOBITS);
   while (section = section->next) {
      if (section->header.type == type && ((section->header.flags & required_flags) == required_flags)) {
         assert(!section->header.offset);
         if (section->header.size) {
            section->header.offset = pos;
            pos += section->header.size;
            pos = align_up(pos, 0x40);
         }
      }
   }
   return pos;
}

void write_elf(Elf *elf, const char *filename, bool plain) {
   Section *section = elf->sections;
   while (section = section->next) {
      section->header.offset = 0;
      if (!section->data)
         continue;

      bool compressed = !plain && section->header.size > 0x18;
      compressed = compressed && section->header.type != SHT_RPL_FILEINFO;
      compressed = compressed && section->header.type != SHT_RPL_CRCS;

      if (compressed) {
         section->header.flags |= SHF_RPL_ZLIB;
         CompressedData *data_out = malloc(section->header.size + 4);
         data_out->deflated_size = section->header.size;

         z_stream s = { 0 };
         s.next_in = section->data;
         s.avail_in = section->header.size;
         s.next_out = data_out->compressed_data;
         s.avail_out = section->header.size;

         deflateInit(&s, elf->info->compressionLevel);
         deflate(&s, Z_FINISH);
         deflateEnd(&s);
         assert(!s.avail_in);
         assert(s.avail_out);

         section->header.size = (u8 *)s.next_out - (u8 *)data_out;

         free(section->data);
         section->data = data_out;
      } else
         section->header.flags &= ~SHF_RPL_ZLIB;
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

   int pos = elf->header.shoff + elf->header.shnum * elf->header.shentsize;
   pos = add_sections(elf, pos, SHT_RPL_CRCS, 0);
   pos = add_sections(elf, pos, SHT_RPL_FILEINFO, 0);
   pos = add_sections(elf, pos, SHT_PROGBITS, SHF_ALLOC | SHF_WRITE);
   pos = add_sections(elf, pos, SHT_RPL_EXPORTS, SHF_ALLOC);
   pos = add_sections(elf, pos, SHT_RPL_IMPORTS, SHF_ALLOC);
   pos = add_sections(elf, pos, SHT_SYMTAB, SHF_ALLOC);
   pos = add_sections(elf, pos, SHT_STRTAB, SHF_ALLOC);
   pos = add_sections(elf, pos, SHT_PROGBITS, SHF_ALLOC | SHF_CODE);
   pos = add_sections(elf, pos, SHT_RELA, 0);

   fseek(fp, elf->header.shoff, SEEK_SET);
   section = elf->sections;
   do
      fwrite(section, elf->header.shentsize, 1, fp);
   while (section = section->next);

   section = elf->sections;
   while (section = section->next) {
      if (section->header.offset) {
         fseek(fp, section->header.offset, SEEK_SET);
         fwrite(section->data, 1, section->header.size, fp);
         int extra = (section->header.offset + section->header.size) & 0x1F;
         while (extra++ & 0xF)
            fputc('\0', fp);
      }
   }

   fclose(fp);
   printf("output written to %s\n", filename);
}
