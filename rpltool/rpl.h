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

#undef ENUM_VAL
#define ENUM_VAL(prefix, name, val) prefix##name = val,

typedef enum { SECTION_TYPE_LIST } section_type;
typedef enum { SECTION_FLAGS_LIST } section_flags;
typedef enum { ELF_CLASS_LIST } elf_class;
typedef enum { ELF_DATAENCODING_LIST } elf_data_encoding;
typedef enum { ELF_VERSION_LIST } elf_version;
typedef enum { ELF_RELOCATION_LIST } elf_relocation;
#undef ENUM_VAL
#define ENUM_VAL GET_ENUM_NAME

EMIT_ENUM_TO_STR_FUNC(section_type, SECTION_TYPE_LIST)
EMIT_ENUM_TO_STR_FUNC(elf_class, ELF_CLASS_LIST)
EMIT_ENUM_TO_STR_FUNC(elf_data_encoding, ELF_DATAENCODING_LIST)
EMIT_ENUM_TO_STR_FUNC(elf_version, ELF_VERSION_LIST)
EMIT_ENUM_TO_STR_FUNC(elf_relocation, ELF_RELOCATION_LIST)

#undef ENUM_VAL
#define ENUM_VAL GET_FLAG_NAME

EMIT_FLAGS_TO_STR_FUNC(section_flags, SECTION_FLAGS_LIST)

#pragma scalar_storage_order big - endian

typedef struct {
   u32 val;
} u32_be;

#define FI_IS_RPL 0
#define FI_IS_RPX 2

typedef struct fileinfo_t {
   u16 magic;
   u16 version;
   struct {
      u32 Text;
      u32 TextAlign;
      u32 Data;
      u32 DataAlign;
      u32 LoaderInfo;
      u32 LoaderInfoAlign;
      u32 Temp;
   } RegBytes;
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
   u32 tlsModuleIndex;
   u32 tlsAlignShift;
   // version 0x0402
} fileinfo_t;

typedef struct section_header_t {
   u32 name;      /* Section name (string tbl index) */
   u32 type;      /* Section type */
   u32 flags;     /* Section flags */
   u32 addr;      /* Section virtual addr at execution */
   u32 offset;    /* Section file offset */
   u32 size;      /* Section size in bytes */
   u32 link;      /* Link to another section */
   u32 info;      /* Additional section information */
   u32 addralign; /* Section alignment */
   u32 entsize;   /* Entry size if section holds table */
} section_header_t;

typedef struct {
   u32 deflated_size;
   u8 compressed_data[];
} compressed_data_t;

#define ELF_MAGIC 0x7F454C46
typedef struct elf_header_t {
   u32 magic;
   u8 elf_class;
   u8 data_encoding;
   u8 elf_version;
   u8 os_abi[9];
   u16 type;      /* Object file type */
   u16 machine;   /* Architecture */
   u32 version;   /* Object file version */
   u32 entry;     /* Entry point virtual address */
   u32 phoff;     /* Program header table file offset */
   u32 shoff;     /* Section header table file offset */
   u32 flags;     /* Processor-specific flags */
   u16 ehsize;    /* ELF header size in bytes */
   u16 phentsize; /* Program header table entry size */
   u16 phnum;     /* Program header table entry count */
   u16 shentsize; /* Section header table entry size */
   u16 shnum;     /* Section header table entry count */
   u16 shstrndx;  /* Section header string table index */
} elf_header_t;

/* Symbol table entry.  */

typedef struct {
   u32 name;     /* Symbol name (string tbl index) */
   u32 value;    /* Symbol value */
   u32 size;     /* Symbol size */
   u8 info; /* Symbol type and binding */
   u8 other;     /* Symbol visibility */
   u16 shndx;    /* Section index */
} symbol_t;

/* Relocation table entry with addend (in section of type SHT_RELA).  */

typedef struct {
   u32 offset; /* Address */
   struct {
      u32 index : 24; /* symbol index */
      u32 type : 8;   /* Relocation type */
   };
   s32 addend; /* Addend */
} relocation_t;

#pragma scalar_storage_order default

typedef struct section_t {
   section_header_t header;
   struct section_t *prev;
   struct section_t *next;
   struct section_t *link;
   struct section_t *link2;
   const char* name;
   void *data;
} section_t;

typedef struct {
   elf_header_t header;
   fileinfo_t *info;
   section_t* sections;
   const char *shstrtab;
   const char *strtab;
   u32_be *crcs;
   bool is_rpl;
} elf_t;

void elf_print_header(elf_t *elf);
void elf_print_sections(elf_t *elf);
void elf_print_strtab(const char *strtab);
void elf_print_file_info(fileinfo_t *info);
elf_t *read_elf(const char *filename);
void write_elf(elf_t *elf, const char *filename, bool plain);
void free_elf(elf_t *elf);

static inline int get_sid(section_t *s) {
   int id = 0;
   while (s = s->prev)
      id++;
   return id;
}

static inline int get_section_count(elf_t *elf) {
   section_t *s = elf->sections;

   int count = 0;
   do
      count++;
   while (s = s->next);
   return count;
}

static inline section_t *get_section(elf_t *elf, int id) {
   section_t *s = elf->sections;
   while (id--)
      if (s)
         s = s->next;
   return s;
}

static inline section_t *get_section_by_name(elf_t *elf, const char *name) {
   section_t *s = elf->sections;
   while (s = s->next)
      if (!strcmp(s->name, name))
         return s;

   return NULL;
}

#endif // RPL_H
