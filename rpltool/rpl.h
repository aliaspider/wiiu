#ifndef RPL_H
#define RPL_H

#include "common.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#define SECTION_TYPE_LIST \
   ENUM_VAL(SHT_, NULL, 0) \
   ENUM_VAL(SHT_, PROGBITS, 1) \
   ENUM_VAL(SHT_, SYMTAB, 2) \
   ENUM_VAL(SHT_, STRTAB, 3) \
   ENUM_VAL(SHT_, RELA, 4) \
   ENUM_VAL(SHT_, NOBITS, 8) \
   ENUM_VAL(SHT_, RPL_EXPORTS, 0x80000001) \
   ENUM_VAL(SHT_, RPL_IMPORTS, 0x80000002) \
   ENUM_VAL(SHT_, RPL_CRCS, 0x80000003) \
   ENUM_VAL(SHT_, RPL_FILEINFO, 0x80000004)

#define SECTION_FLAGS_LIST \
   ENUM_VAL(SHF_, NULL, 0) \
   ENUM_VAL(SHF_, WRITE, 1u << 0) \
   ENUM_VAL(SHF_, ALLOC, 1u << 1) \
   ENUM_VAL(SHF_, CODE, 1u << 2) \
   ENUM_VAL(SHF_, MERGE, 1u << 4) \
   ENUM_VAL(SHF_, STRINGS, 1u << 5) \
   ENUM_VAL(SHF_, INFO_LINK, 1u << 6) \
   ENUM_VAL(SHF_, LINK_ORDER, 1u << 7) \
   ENUM_VAL(SHF_, OS_NONCONFORMING, 1u << 8) \
   ENUM_VAL(SHF_, GROUP, 1u << 9) \
   ENUM_VAL(SHF_, TLS, 1u << 10) \
   ENUM_VAL(SHF_, COMPRESSED, 1u << 11) \
   ENUM_VAL(SHF_, RPL_TLS, 1u << 26) \
   ENUM_VAL(SHF_, RPL_ZLIB, 1u << 27) \
   ENUM_VAL(SHF_, ORDERED, 1u << 30) \
   ENUM_VAL(SHF_, EXCLUDE, 1u << 31)

#define ELF_CLASS_LIST \
   ENUM_VAL(EC_, NONE, 0) \
   ENUM_VAL(EC_, 32BIT, 1) \
   ENUM_VAL(EC_, 64BIT, 2)

#define ELF_DATAENCODING_LIST \
   ENUM_VAL(ED_, NONE, 0) \
   ENUM_VAL(ED_, 2LSB, 1) \
   ENUM_VAL(ED_, 2MSB, 2)

#define ELF_VERSION_LIST \
   ENUM_VAL(EV_, NONE, 0) \
   ENUM_VAL(EV_, CURRENT, 1)

#define ELF_RELOCATION_LIST \
   ENUM_VAL(R_, PPC_NONE, 0) \
   ENUM_VAL(R_, PPC_ADDR32, 1) \
   ENUM_VAL(R_, PPC_ADDR24, 2) \
   ENUM_VAL(R_, PPC_ADDR16, 3) \
   ENUM_VAL(R_, PPC_ADDR16_LO, 4) \
   ENUM_VAL(R_, PPC_ADDR16_HI, 5) \
   ENUM_VAL(R_, PPC_ADDR16_HA, 6) \
   ENUM_VAL(R_, PPC_ADDR14, 7) \
   ENUM_VAL(R_, PPC_ADDR14_BRTAKEN, 8) \
   ENUM_VAL(R_, PPC_ADDR14_BRNTAKEN, 9) \
   ENUM_VAL(R_, PPC_REL24, 10) \
   ENUM_VAL(R_, PPC_REL14, 11) \
   ENUM_VAL(R_, PPC_REL14_BRTAKEN, 12) \
   ENUM_VAL(R_, PPC_REL14_BRNTAKEN, 13) \
   ENUM_VAL(R_, PPC_GOT16, 14) \
   ENUM_VAL(R_, PPC_GOT16_LO, 15) \
   ENUM_VAL(R_, PPC_GOT16_HI, 16) \
   ENUM_VAL(R_, PPC_GOT16_HA, 17) \
   ENUM_VAL(R_, PPC_PLTREL24, 18) \
   ENUM_VAL(R_, PPC_JMP_SLOT, 21) \
   ENUM_VAL(R_, PPC_RELATIVE, 22) \
   ENUM_VAL(R_, PPC_LOCAL24PC, 23) \
   ENUM_VAL(R_, PPC_REL32, 26) \
   ENUM_VAL(R_, PPC_TLS, 67) \
   ENUM_VAL(R_, PPC_DTPMOD32, 68) \
   ENUM_VAL(R_, PPC_TPREL16, 69) \
   ENUM_VAL(R_, PPC_TPREL16_LO, 70) \
   ENUM_VAL(R_, PPC_TPREL16_HI, 71) \
   ENUM_VAL(R_, PPC_TPREL16_HA, 72) \
   ENUM_VAL(R_, PPC_TPREL32, 73) \
   ENUM_VAL(R_, PPC_DTPREL16, 74) \
   ENUM_VAL(R_, PPC_DTPREL16_LO, 75) \
   ENUM_VAL(R_, PPC_DTPREL16_HI, 76) \
   ENUM_VAL(R_, PPC_DTPREL16_HA, 77) \
   ENUM_VAL(R_, PPC_DTPREL32, 78) \
   ENUM_VAL(R_, PPC_GOT_TLSGD16, 79) \
   ENUM_VAL(R_, PPC_GOT_TLSGD16_LO, 80) \
   ENUM_VAL(R_, PPC_GOT_TLSGD16_HI, 81) \
   ENUM_VAL(R_, PPC_GOT_TLSGD16_HA, 82) \
   ENUM_VAL(R_, PPC_GOT_TLSLD16, 83) \
   ENUM_VAL(R_, PPC_GOT_TLSLD16_LO, 84) \
   ENUM_VAL(R_, PPC_GOT_TLSLD16_HI, 85) \
   ENUM_VAL(R_, PPC_GOT_TLSLD16_HA, 86) \
   ENUM_VAL(R_, PPC_GOT_TPREL16, 87) \
   ENUM_VAL(R_, PPC_GOT_TPREL16_LO, 88) \
   ENUM_VAL(R_, PPC_GOT_TPREL16_HI, 89) \
   ENUM_VAL(R_, PPC_GOT_TPREL16_HA, 90) \
   ENUM_VAL(R_, PPC_GOT_DTPREL16, 91) \
   ENUM_VAL(R_, PPC_GOT_DTPREL16_LO, 92) \
   ENUM_VAL(R_, PPC_GOT_DTPREL16_HI, 93) \
   ENUM_VAL(R_, PPC_GOT_DTPREL16_HA, 94) \
   ENUM_VAL(R_, PPC_TLSGD, 95) \
   ENUM_VAL(R_, PPC_TLSLD, 96) \
   ENUM_VAL(R_, PPC_EMB_SDA21, 109) \
   ENUM_VAL(R_, PPC_EMB_RELSDA, 116) \
   ENUM_VAL(R_, PPC_DIAB_SDA21_LO, 180) \
   ENUM_VAL(R_, PPC_DIAB_SDA21_HI, 181) \
   ENUM_VAL(R_, PPC_DIAB_SDA21_HA, 182) \
   ENUM_VAL(R_, PPC_DIAB_RELSDA_LO, 183) \
   ENUM_VAL(R_, PPC_DIAB_RELSDA_HI, 184) \
   ENUM_VAL(R_, PPC_DIAB_RELSDA_HA, 185) \
   ENUM_VAL(R_, PPC_GHS_REL16_HA, 251) \
   ENUM_VAL(R_, PPC_GHS_REL16_HI, 252) \
   ENUM_VAL(R_, PPC_GHS_REL16_LO, 253)

#define FILE_INFO_FLAGS_LIST \
   ENUM_VAL(FIF_, NONE, 0) \
   ENUM_VAL(FIF_, RPX, 1u << 1) \
   ENUM_VAL(FIF_, TLS, 1u << 3)

#undef ENUM_VAL
#define ENUM_VAL(prefix, name, val) prefix##name = val,
typedef enum { SECTION_TYPE_LIST } SectionType;
typedef enum { SECTION_FLAGS_LIST } SectionFlags;
typedef enum { ELF_CLASS_LIST } ElfClass;
typedef enum { ELF_DATAENCODING_LIST } ElfDataEncoding;
typedef enum { ELF_VERSION_LIST } ElfVersion;
typedef enum { ELF_RELOCATION_LIST } ElfRelocation;
typedef enum { FILE_INFO_FLAGS_LIST } FileInfoFlags;

#undef ENUM_VAL
#define ENUM_VAL GET_ENUM_NAME
EMIT_ENUM_TO_STR_FUNC(SectionType, SECTION_TYPE_LIST)
EMIT_ENUM_TO_STR_FUNC(ElfClass, ELF_CLASS_LIST)
EMIT_ENUM_TO_STR_FUNC(ElfDataEncoding, ELF_DATAENCODING_LIST)
EMIT_ENUM_TO_STR_FUNC(ElfVersion, ELF_VERSION_LIST)
EMIT_ENUM_TO_STR_FUNC(ElfRelocation, ELF_RELOCATION_LIST)

#undef ENUM_VAL
#define ENUM_VAL GET_FLAG_NAME
EMIT_FLAGS_TO_STR_FUNC(SectionFlags, SECTION_FLAGS_LIST)
EMIT_FLAGS_TO_STR_FUNC(FileInfoFlags, FILE_INFO_FLAGS_LIST)

#define ELF_MAGIC MAKE_MAGIC(0x7f, 'E', 'L', 'F')
#define ET_CAFE 0xFE01
#define EM_PPC 0x14

#pragma scalar_storage_order big - endian

typedef struct {
   u32 val;
} u32_be;

typedef struct {
   s16 val;
} s16_be;

typedef struct FileInfo {
   u16 magic;
   u16 version;
   u32 TextSize;
   u32 TextAlign;
   u32 DataSize;
   u32 DataAlign;
   u32 LoaderSize;
   u32 LoaderAlign;
   u32 TempSize;
   u32 TrampAdj;
   u32 SDABase;
   u32 SDA2Base;
   u32 SizeCoreStacks;
   u32 SrcFileNameOffset;
   // version 0x0300
   u32 Flags;
   u32 SysHeapBytes;
   u32 TagsOffset;
   // version 0x0401
   u32 minVersion;
   u32 compressionLevel;
   u32 trampAddition;
   u32 fileInfoPad;
   u32 cafeSdkVersion;
   u32 cafeSdkRevision;
   u16 tlsModuleIndex;
   u16 tlsAlignShift;
   u32 runtimeFileInfoSize;
   // version 0x0402
} FileInfo;

typedef struct SectionHeader {
   u32 name;
   u32 type;
   u32 flags;
   u32 addr;
   u32 offset;
   u32 size;
   u32 link;
   u32 info;
   u32 align;
   u32 entsize;
} SectionHeader;

typedef struct {
   u32 deflated_size;
   u8 compressed_data[];
} CompressedData;

typedef struct ElfHeader {
   u32 magic;
   u8 elf_class;
   u8 data_encoding;
   u8 elf_version;
   u8 os_abi[2];
   u8 pad[7];
   u16 type;
   u16 machine;
   u32 version;
   u32 entry;
   u32 phoff;
   u32 shoff;
   u32 flags;
   u16 ehsize;
   u16 phentsize;
   u16 phnum;
   u16 shentsize;
   u16 shnum;
   u16 shstrndx;
} ElfHeader;

typedef struct {
   u32 name;
   u32 value;
   u32 size;
   u8 info;
   u8 other;
   u16 shndx;
} Symbol;

typedef struct {
   u32 offset;
   struct {
      u32 index : 24;
      u32 type : 8;
   };
   s32 addend;
} Relocation;

#pragma scalar_storage_order default

typedef struct Section {
   SectionHeader header;
   struct Section *prev;
   struct Section *next;
   struct Section *link;
   struct Section *link2;
   const char *name;
   u8 *data;
} Section;

typedef struct {
   ElfHeader header;
   FileInfo info;
   Section *sections;
   const char *shstrtab;
   bool is_rpl;
} Elf;

void elf_print_header(Elf *elf);
void elf_print_sections(Elf *elf);
void elf_print_strtab(const char *strtab);
void elf_print_file_info(FileInfo *info);
Elf *read_elf(const char *filename);
void write_elf(Elf *elf, const char *filename, bool plain);
void free_elf(Elf *elf);

static inline int get_sid(Section *s) {
   int id = 0;
   while (s = s->prev)
      id++;
   return id;
}

static inline int get_section_count(Elf *elf) {
   Section *s = elf->sections;

   int count = 0;
   do
      count++;
   while (s = s->next);
   return count;
}

static inline Section *get_section(Elf *elf, int id) {
   Section *s = elf->sections;
   while (id--)
      if (s)
         s = s->next;
   return s;
}

static inline Section *get_section_by_name(Elf *elf, const char *name) {
   Section *s = elf->sections;
   while (s = s->next)
      if (!strcmp(s->name, name))
         return s;

   return NULL;
}

#endif // RPL_H
