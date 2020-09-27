#ifndef TABLE_H
#define TABLE_H

typedef enum
{
   TABLE_ENTRY_INVALID,
   TABLE_ENTRY_CSTR,
   TABLE_ENTRY_CSTR_RIGHT_ALIGNED,
   TABLE_ENTRY_HEX,
   TABLE_ENTRY_HEX_ZERO_PAD,
   TABLE_ENTRY_INT,
   TABLE_ENTRY_ID,
} table_entry_enum;

typedef struct
{
   table_entry_enum type;
   const char* header;
} table_format_t;

typedef union
{
   int i;
   unsigned u;
   void* ptr;
   char* cstr;

} table_entry_t;

typedef struct
{
   table_format_t* format;
   int* cw;
   int* vw;
   int columns;
   struct
   {
      table_entry_t** data;
      int count;
      int max;
   } rows;
} table_t;

table_t* table_create(table_format_t* format);
void* table_add_row(table_t* table, ...);
void table_print_header(table_t* table);
void table_print(table_t* table);
void table_free(table_t* table);

#endif // TABLE_H
