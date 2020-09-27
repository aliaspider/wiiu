#ifndef OPTIONS_H
#define OPTIONS_H

#include <stdbool.h>

typedef struct
{
   const char id;
   const bool has_arg;
   const char* id_long;
   const char* help;
} option_def_t;

char **parse_options(int argc, char **argv, option_def_t* options);

#endif // OPTIONS_H
