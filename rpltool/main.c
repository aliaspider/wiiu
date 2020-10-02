
#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

#include "options.h"
#include "rpl.h"

static option_def_t main_options_defs[] = { { 'o', true, "out", "output file name" },
                                            { 'p', false, "plain", "do not compress sections" },
                                            { 'e', false, "header", "display elf header" },
                                            { 'S', false, "sections", "display sections (all)" },
                                            { 'i', false, "file-info", "display file info" },
                                            { 't', false, "string-table", "display string table" },
                                            { 'v', false, "version", "print version" },
                                            { 0 } };

typedef struct {
   const char *filename_out;
   const char *plain;
   const char *display_header;
   const char *display_sections;
   const char *display_file_info;
   const char *display_string_table;
   const char *print_version;
   const char *filename;
} main_options_t;

int main(int argc, char **argv) {
   main_options_t *opts = (main_options_t *)parse_options(argc, argv, main_options_defs);

   if (!opts)
      return 1;

   if (opts->print_version) {
      printf("rpltool version 0.01\n");
      return 0;
   }

   if (!opts->filename)
      return 0;

   Elf *elf = read_elf(opts->filename);

   if (opts->filename_out) {
      write_elf(elf, opts->filename_out, opts->plain);
#if 1
      if (elf->is_rpl && !opts->plain) {
         FILE *fp = fopen(opts->filename, "rb");
         fseek(fp, 0, SEEK_END);
         int sz = ftell(fp);

         void *src = malloc(sz);
         fseek(fp, 0, SEEK_SET);
         fread(src, 1, sz, fp);
         fclose(fp);

         fp = fopen(opts->filename_out, "rb");
         fseek(fp, 0, SEEK_END);
         assert(sz == ftell(fp));

         void *dst = malloc(sz);
         fseek(fp, 0, SEEK_SET);
         fread(dst, 1, sz, fp);
         fclose(fp);

         assert(!memcmp(src, dst, sz));
         free(src);
         free(dst);
      }
#endif
   }

   if (opts->display_header)
      elf_print_header(elf);

   if (opts->display_string_table) {
      printf("shstrtab:\n");
      elf_print_strtab(elf->shstrtab);
      printf("strtab:\n");
      elf_print_strtab(get_section_by_name(elf, ".strtab")->data);
   }

   if (opts->display_sections)
      elf_print_sections(elf);

   if (opts->display_file_info)
      elf_print_file_info(&elf->info);

   free_elf(elf);
   free(opts);

   return 0;
}
