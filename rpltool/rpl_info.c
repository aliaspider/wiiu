
#include <math.h>
#include <stdio.h>
#include <string.h>

#include "common.h"
#include "rpl.h"
#include "table.h"

void elf_print_header(elf_t *elf) {
   log_magic(elf->header.magic);
   log_enum(elf_class, elf->header.elf_class);
   log_enum(elf_data_encoding, elf->header.data_encoding);
   log_enum(elf_version, elf->header.elf_version);
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

void elf_print_sections(elf_t *elf) {
   bool compressed = false;
   section_t *section = elf->sections;
   while (section = section->next)
      if (section->header.flags & SHF_RPL_ZLIB)
         compressed = true;

   table_format_t elf_sections_table[] = {
      { TABLE_ENTRY_ID, "id" },
      { TABLE_ENTRY_CSTR, "name" },
      { TABLE_ENTRY_CSTR, "type" },
      { TABLE_ENTRY_CSTR, "flags" },
      { TABLE_ENTRY_HEX_ZERO_PAD, "addr" },
      { TABLE_ENTRY_HEX_ZERO_PAD, "offset" },
      { TABLE_ENTRY_HEX_ZERO_PAD, "size" },
      { TABLE_ENTRY_INT, "link" },
      { TABLE_ENTRY_HEX_ZERO_PAD, "info" },
      { TABLE_ENTRY_INT, "align" },
      { TABLE_ENTRY_INT, "entry" },
      { TABLE_ENTRY_INVALID, NULL },

   };
   table_t *table = table_create(elf_sections_table);

   section = elf->sections;
   while (section = section->next) {
      compressed = (section->header.flags & SHF_RPL_ZLIB);
      table_add_row(table, get_sid(section), section->name, section_type_to_str(section->header.type),
                    section_flags_to_str(section->header.flags, ", "), section->header.addr, section->header.offset,
                    section->header.size, section->header.link, section->header.info,
                    (unsigned)log2(section->header.addralign), section->header.entsize);
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

void elf_print_file_info(fileinfo_t *info) {
   log_var(info->magic);
   log_var(info->version);
   log_var(info->RegBytes.Text);
   //   return;
   log_var(info->RegBytes.TextAlign);
   log_var(info->RegBytes.Data);
   log_var(info->RegBytes.DataAlign);
   log_var(info->RegBytes.LoaderInfo);
   log_var(info->RegBytes.LoaderInfoAlign);
   log_var(info->RegBytes.Temp);
   log_var(info->TrampAdj);
   log_var(info->SDABase);
   log_var(info->SDA2Base);
   log_var(info->SizeCoreStacks);
   if (info->TagsOffset)
      log_off_str(info->SrcFileNameOffset, info);
   if (info->version >= 0x0401) {
      log_var(info->Flags);
      log_var(info->SysHeapBytes);
      if (info->TagsOffset)
         log_off_str(info->TagsOffset, info);
   }
   if (info->version >= 0x0402) {
      log_var(info->minVersion);
      log_var(info->compressionLevel);
      log_var(info->trampAddition);
      log_var(info->fileInfoPad);
      log_var(info->cafeSdkVersion);
      log_var(info->cafeSdkRevision);
      log_var(info->tlsModuleIndex);
      log_var(info->tlsAlignShift);
      log_var(sizeof(fileinfo_t));
   }
}
