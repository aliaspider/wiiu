
#include "options.h"
#include "common.h"
#include <assert.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char **parse_options(int argc, char **argv, option_def_t *options) {

  option_def_t *opt = options;
  int count = 0;

  while (options && options[count].id)
    count++;

#ifdef __GLIBC__
  optind = 0;
#else
  optind = 1;
#endif
  optopt = 0;
  optarg = NULL;

  char **vals = calloc(count + argc, sizeof(char *));

  char short_opts[1 + (count * 2) + 3 + 1];
  char *s_ptr = short_opts;
  struct option long_opts[count + 2];
  struct option *l_ptr = long_opts;
  *s_ptr++ = ':';
  while (opt && opt->id) {
    assert(opt->id != 'h');
    assert(opt->id != '?');
    *s_ptr++ = opt->id;
    if (opt->has_arg)
      *s_ptr++ = ':';

    if (opt->id_long) {
      l_ptr->name = opt->id_long;
      l_ptr->has_arg = !!opt->has_arg;
      l_ptr->flag = NULL;
      l_ptr->val = opt->id;
      l_ptr++;
    }
    opt++;
  }
  *s_ptr++ = 'h';
  *s_ptr++ = ':';
  *s_ptr++ = ':';
  *s_ptr = 0;
  l_ptr->name = "help";
  l_ptr->has_arg = optional_argument;
  l_ptr->flag = NULL;
  l_ptr->val = 'h';
  l_ptr++;
  memset(l_ptr, 0, sizeof(*l_ptr));

  int opt_id;
  while ((opt_id = getopt_long(argc, argv, short_opts, long_opts, NULL)) !=
         -1) {
    if (opt_id == 'h')
      goto print_help;

    if (opt_id == ':') {
      fprintf(stderr, "missing argument for option :%s\n", argv[optind - 1]);
      goto print_help;
    }

    if (!options || opt_id == '?') {
      if (optopt)
        fprintf(stderr, "unknown option :%c\n", optopt);
      else
        fprintf(stderr, "unknown option :%s\n", argv[optind - 1]);
      goto print_help;
    }

    int i = 0;
    while (i < count && options[i].id != opt_id)
      i++;

    assert(i < count);

    if (options[i].has_arg) {
      if (!optarg) {
        fprintf(stderr, "required argument missing for option %c",
                options[i].id);
        if (options[i].id_long)
          fprintf(stderr, "[%s]", options[i].id_long);
        fprintf(stderr, "\n");
        goto error;
      }
      vals[i] = optarg;
    } else
      vals[i] = "";
  }

  argc -= optind;
  argv += optind;
  memcpy(vals + count, argv, argc * sizeof(char *));
  vals[count + argc] = NULL;

  return vals;

print_help:
  printf("\nusage : %s%s\n\n", argv[0], options ? " [options]" : "");
  if (options) {
    int i;
    int longopt_len = 0;

    for (i = 0; i < count; i++)
      if (options[i].id_long && longopt_len < strlen(options[i].id_long))
        longopt_len = strlen(options[i].id_long);

    if (longopt_len && longopt_len < strlen("help"))
      longopt_len = strlen("help");

    for (i = 0; i < count; i++)
      printf(options[i].id_long ? "    -%c, --%-*s  %s\n" : "    -%c%-*s  %s\n",
             options[i].id, longopt_len ? longopt_len + 4 : 0,
             options[i].id_long ? options[i].id_long : "", options[i].help);

    printf(longopt_len ? "    -%c, --%-*s  %s\n" : "    -%c%-*s  %s\n", 'h',
           longopt_len ? longopt_len + 4 : 0, longopt_len ? "help" : "",
           "print help");
    printf("\n");
  }

error:
  free(vals);
  return NULL;
}
