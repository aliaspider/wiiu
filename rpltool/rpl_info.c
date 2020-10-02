
#include <math.h>
#include <stdio.h>
#include <string.h>

#include "common.h"
#include "rpl.h"
#include "table.h"

void elf_print_header(Elf *elf) {
   log_magic(elf->header.magic);
   log_enum(ElfClass, elf->header.elf_class);
   log_enum(ElfDataEncoding, elf->header.data_encoding);
   log_enum(ElfVersion, elf->header.elf_version);
   log_carray(elf->header.os_abi);
   log_var(elf->header.type);
   log_var(elf->header.machine);
   log_var(elf->header.version);
   log_var(elf->header.entry);
   log_var(elf->header.phoff);
   log_var(elf->header.shoff);
   log_var(elf->header.flags);
   log_var(elf->header.ehsize);
   log_var(elf->header.phentsize);
   log_var(elf->header.phnum);
   log_var(elf->header.shentsize);
   log_var(elf->header.shnum);
   log_var(elf->header.shstrndx);
}

void elf_print_sections(Elf *elf) {
   Section *sec = elf->sections;

   table_format_t elf_sections_table[] = {
      { TABLE_ENTRY_ID, "id" },
      { TABLE_ENTRY_CSTR, "name" },
      { TABLE_ENTRY_CSTR, "type" },
      { TABLE_ENTRY_CSTR, "flags" },
      { TABLE_ENTRY_HEX_ZERO_PAD, "laddr" },
      { TABLE_ENTRY_HEX_ZERO_PAD, "lsize" },
      { TABLE_ENTRY_HEX_ZERO_PAD, "foffset" },
      { TABLE_ENTRY_HEX_ZERO_PAD, "fsize" },
      { TABLE_ENTRY_INT, "ln" },
      { TABLE_ENTRY_HEX_ZERO_PAD, "info" },
      { TABLE_ENTRY_INT, "al" },
      { TABLE_ENTRY_INT, "es" },
      { TABLE_ENTRY_HEX_ZERO_PAD, "crc" },
      { TABLE_ENTRY_INVALID, NULL },

   };
   table_t *table = table_create(elf_sections_table);
   u32_be* crcs = (u32_be*)get_section(elf, get_section_count(elf) - 2)->data;

   sec = elf->sections;
   while (sec = sec->next) {
      int load = sec->header.size;
      int fsize = sec->header.size;
      if (sec->header.flags & SHF_RPL_ZLIB && sec->data)
         load = ((CompressedData *)sec->data)->deflated_size;
      if (sec->header.type == SHT_NOBITS)
         fsize = 0;

      table_add_row(table, get_sid(sec), sec->name, SectionType_to_str(sec->header.type),
                    SectionFlags_to_str(sec->header.flags, ", "), sec->header.addr, load,
                    sec->header.offset, fsize, sec->header.link, sec->header.info,
                    (unsigned)log2(sec->header.align), sec->header.entsize, crcs[get_sid(sec)]);
   }

   printf("\n");

   table_print_header(table);
   printf("\n");
   table_print(table);
   printf("\n");
   table_free(table);
}

void elf_print_strtab(const char *strtab) {
   if (!strtab)
      return;

   const char *ptr = strtab;
   ptr++;
   while (*ptr)
      ptr += printf("%s\n", ptr);
}

void elf_print_file_info(FileInfo *info) {
   log_var(info->magic);
   log_var(info->version);
   log_var(info->TextSize);
   log_var(info->TextAlign);
   log_var(info->DataSize);
   log_var(info->DataAlign);
   log_var(info->LoaderSize);
   log_var(info->LoaderAlign);
   log_var(info->TempSize);
   log_var(info->TrampAdj);
   log_var(info->SDABase);
   log_var(info->SDA2Base);
   log_var(info->SizeCoreStacks);
   if (info->TagsOffset)
      log_off_str(info->SrcFileNameOffset, info);
   else
      log_var(info->SrcFileNameOffset);
   if (info->version >= 0x0401) {
      log_flags(FileInfoFlags, info->Flags);
      log_var(info->SysHeapBytes);
      if (info->TagsOffset)
         log_off_strs(info->TagsOffset, info);
      else
         log_var(info->TagsOffset);
   }
   if (info->version >= 0x0402) {
      log_var(info->minVersion);
      log_svar(info->compressionLevel);
      log_var(info->trampAddition);
      log_var(info->fileInfoPad);
      log_var(info->cafeSdkVersion);
      log_var(info->cafeSdkRevision);
      log_var(info->tlsModuleIndex);
      log_var(info->tlsAlignShift);
      log_var(info->runtimeFileInfoSize);
   }
}
