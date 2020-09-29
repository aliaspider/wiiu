#ifndef COMMON_H
#define COMMON_H

#include <ctype.h>
#include <stdint.h>
#include <stdio.h>

typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;

#define GET_ENUM_NAME(prefix, name, val) \
   case prefix##name: \
      return #name;

#define EMIT_ENUM_TO_STR_FUNC(name, cases) \
   static inline const char *name##_to_str(name type) { \
      switch (type) { cases } \
      return "unknown"; \
   }

#define GET_FLAG_NAME(prefix, name, val) \
   case prefix##name: \
      pos += snprintf(buffer + pos, sizeof(buffer) - pos, "%s%s", #name, seperator); \
      flags &= ~(1u << i); \
      break;

#define EMIT_FLAGS_TO_STR_FUNC(name, cases) \
   static inline const char *name##_to_str(int flags, const char *seperator) { \
      int i; \
      static char buffer[32 * 32]; \
      int pos = 0; \
      for (i = 0; i < 32; i++) \
         if ((1u << i) & flags) \
            switch (1u << i) { cases } \
      if (flags) \
         snprintf(buffer + pos, sizeof(buffer) - pos, "0x%08X", flags); \
      else if (pos) \
         buffer[pos - strlen(seperator)] = '\0'; \
      else \
         return "NULL"; \
      return buffer; \
   }

#define MAKE_MAGIC(a, b, c, d) (((a) << 24) | ((b) << 16) | ((c) << 8) | ((d) << 0))

#define log_var(X) printf("%-35s: 0x%0*X (%u)\n", #X, sizeof(X) * 2, (u32)(X), (u32)(X))
#define log_svar(X) printf("%-35s: 0x%0*X (%i)\n", #X, sizeof(X) * 2, (u32)(X), (s32)(X))
#define log_array(X) debug_array(X, sizeof(X), #X)
#define log_enum(type, X) printf("%-35s: 0x%X (%s)\n", #X, (X), type##_to_str(X))
#define log_flags(type, X) printf("%-35s: 0x%08X (%s)\n", #X, (X), type##_to_str(X, " | "))
#define log_magic(X) \
   printf("%-35s: '%c''%c''%c''%c' (0x%08X)\n", #X, (u32)(X) >> 24, (u32)(X) >> 16, (u32)(X) >> 8, (u32)(X), (u32)(X))
#define log_carray(X) debug_char_array(X, sizeof(X), #X)
#define log_str(X) printf("%-35s: \"%s\"\n", #X, (char *)(X))
#define log_off_str(X, data) printf("%-35s: 0x%08X \"%s\"\n", #X, (X), (char *)data + (X))
#define log_off_strs(X, data) \
   do { \
      printf("%-35s: 0x%08X ", #X, (X)); \
      const char *ptr = (char *)data + (X); \
      while (*ptr) { \
         ptr += printf("\"%s\" ", ptr) - 2; \
      } \
      printf("\n"); \
   } while (0)

static inline void dump_array(uint32_t *src, int size);
static inline void debug_array(uint32_t *src, int size, const char *name) {
   printf("%-35s:", name);
   dump_array(src, size);
}
static inline void debug_char_array(uint8_t *src, int size, const char *name) {
   int i;
   printf("%-35s: ", name);
   //   for (i = 0; i < size; i++)
   //      printf("%-2c ", isalnum(src[i])?src[i]: '?');
   //   printf("\n%-35s: ", "");
   for (i = 0; i < size; i++)
      printf("%02X ", src[i]);
   printf("\n");
}

static inline void dump_array(uint32_t *src, int size) {
   int i;
   size = (size / 4) - 1;
   printf("\n{");
   for (i = 0; i < size; i++) {
      if (!(i & 3))
         printf("\n   ");

      printf("0x%08X, ", src[i]);
   }

   if (!(i & 3))
      printf("\n   ");

   printf("0x%08X\n}\n", src[i]);
}

static inline u32 align_up(u32 x, u32 align) { return (x + (align - 1)) & ~(align - 1); }
static inline u32 align_down(u32 x, u32 align) { return x & ~(align - 1); }

#endif // COMMON_H
