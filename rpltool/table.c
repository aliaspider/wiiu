#include "table.h"
#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

table_t *table_create(table_format_t *format) {
   int i;
   table_t *table = calloc(1, sizeof(*table));

   table->rows.max = 32;
   table->rows.data = calloc(table->rows.max, sizeof(*table->rows.data));

   while (format[table->columns].type)
      table->columns++;

   table->format = malloc(table->columns * sizeof(*format));
   memcpy(table->format, format, table->columns * sizeof(*format));

   table->cw = malloc(table->columns * sizeof(int));
   table->vw = calloc(sizeof(int), table->columns);

   for (i = 0; i < table->columns; i++)
      table->cw[i] = strlen(table->format[i].header) + ((format->type == TABLE_ENTRY_INT)? 0:1);

   return table;
}

void *table_add_row(table_t *table, ...) {
   int i;
   va_list va;

   if (table->rows.count == table->rows.max) {
      table->rows.max <<= 1;
      table->rows.data = realloc(table->rows.data, table->rows.max * sizeof(*table->rows.data));
   }

   table_entry_t *row = malloc(table->columns * sizeof(*row));
   table->rows.data[table->rows.count++] = row;

   va_start(va, table);

   for (i = 0; i < table->columns; i++) {
      int width = 0;

      switch (table->format[i].type) {
      case TABLE_ENTRY_CSTR:
      case TABLE_ENTRY_CSTR_RIGHT_ALIGNED:
         row[i].cstr = strdup(va_arg(va, const char *));
         width = strlen(row[i].cstr);
         break;

      case TABLE_ENTRY_HEX:
      case TABLE_ENTRY_HEX_ZERO_PAD:
         row[i].u = va_arg(va, unsigned);
         width = snprintf(NULL, 0, "0x%X", row[i].u);
         break;

      case TABLE_ENTRY_INT:
         row[i].i = va_arg(va, int);
         width = snprintf(NULL, 0, "%i", row[i].i);
         break;

      case TABLE_ENTRY_ID:
         row[i].i = va_arg(va, int);
         width = snprintf(NULL, 0, "[%i]", row[i].i);
         break;
      }

      if (table->vw[i] < width)
         table->vw[i] = width;
      if (table->cw[i] < width)
         table->cw[i] = width;
   }

   va_end(va);
}

void table_print_header(table_t *table) {
   int i;
   for (i = 0; i < table->columns; i++) {
      if (table->format[i].type == TABLE_ENTRY_INT)
         printf("%*s ", table->cw[i], table->format[i].header);
      else
         printf("%-*s ", table->cw[i], table->format[i].header);
   }
}
void table_print(table_t *table) {
   int i, j;

   for (j = 0; j < table->rows.count; j++) {
      table_entry_t *row = table->rows.data[j];

      for (i = 0; i < table->columns; i++) {
         switch (table->format[i].type) {
         case TABLE_ENTRY_CSTR: printf("%*s%-*s ", table->cw[i] - table->vw[i], "", table->vw[i], row[i].cstr); break;

         case TABLE_ENTRY_CSTR_RIGHT_ALIGNED:
            printf("%*s%*s ", table->cw[i] - table->vw[i], "", table->vw[i], row[i].cstr);
            break;

         case TABLE_ENTRY_HEX: printf("%*s0x%-*X ", table->cw[i] - table->vw[i], "", table->vw[i] - 2, row[i].u); break;

         case TABLE_ENTRY_HEX_ZERO_PAD:
            printf("%*s0x%0*X ", table->cw[i] - table->vw[i], "", table->vw[i] - 2, row[i].u);
            break;

         case TABLE_ENTRY_INT: printf("%*s%*i ", table->cw[i] - table->vw[i], "", table->vw[i], row[i].i); break;

         case TABLE_ENTRY_ID: printf("%*s[%*i] ", table->cw[i] - table->vw[i], "", table->vw[i] - 2, row[i].i); break;
         }
      }

      printf("\n");
   }
}

void table_free(table_t *table) {
   int i, j;

   for (j = 0; j < table->rows.count; j++) {
      for (i = 0; i < table->columns; i++) {
         switch (table->format[i].type) {
         case TABLE_ENTRY_CSTR:
         case TABLE_ENTRY_CSTR_RIGHT_ALIGNED: free(table->rows.data[j][i].cstr); break;
         }
      }
      free(table->rows.data[j]);
   }

   free(table->rows.data);
   free(table->cw);
   free(table->vw);
   free(table->format);
   free(table);
}
