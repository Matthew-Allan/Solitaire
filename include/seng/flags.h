#ifndef FLAGS_H
#define FLAGS_H

#include <stdint.h>

typedef uint8_t flags8;
typedef uint16_t flags16;
typedef uint32_t flags32;
typedef uint64_t flags64;

// Check if all flags in set are set in flags.
#define FLAGS_SET(flags, set) (((flags) & (set)) == (set))

// Check if all flags in unset are unset in flags.
#define FLAGS_UNSET(flags, unset) FLAGS_SET(~flags, unset)

// Check if all flags in set are set and all flags in unset are unset.
#define FLAGS_COMBO(flags, set, unset) (FLAGS_SET(flags, set) && FLAGS_UNSET(flags, unset)) 

// Check if a flag is set given its index.
#define FLAG_INDEX_SET(flags, index) ((flags) >> (index) & 1)

// Create a flag given an offset
#define FLAG(offset) (1 << (offset)) 

#endif

// Flag enum generation. Only happens if FLAG_MODULE_NAME is defined
#ifdef FLAG_MODULE_NAME

// If no FLAG_LIST is specified, an error is displayed.
#ifndef FLAG_LIST
#error No flags specified by the FLAG_LIST macro before importing core/flags.h
#endif

// Concatonating macros.
#define _JOIN_UND(a, b) a ## _ ## b
#define _JOIN3_UND(a, b, c) a ## _ ## b ## _ ## c

#define JOIN_UND(a, b) _JOIN_UND(a, b)
#define JOIN3_UND(a, b, c) _JOIN3_UND(a, b, c)

// Create an enum from the FLAG_LIST.
#define FLAG_ENUM(name) \
    typedef enum name { \
        FLAG_LIST \
    } name;

// Create an enum that stores the bitwise offset of each flag.
#define FLAG_ELEMENT(name) JOIN3_UND(FLAG_MODULE_NAME, name, OFFSET),
FLAG_ENUM(JOIN_UND(FLAG_MODULE_NAME, Offsets))
#undef FLAG_ELEMENT

// Create an enum that stores the flag correctly offset.
#define FLAG_ELEMENT(name) JOIN_UND(FLAG_MODULE_NAME, name) = FLAG(JOIN3_UND(FLAG_MODULE_NAME, name, OFFSET)),
FLAG_ENUM(JOIN_UND(FLAG_MODULE_NAME, Flags))
#undef FLAG_ELEMENT

#undef FLAG_LIST
#undef FLAG_MODULE_NAME

#endif