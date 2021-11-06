
#include <limits.h>

typedef signed char int8_t;
typedef short int16_t;
typedef int int32_t;

typedef signed char int_fast8_t;
typedef short int_fast16_t;
typedef int int_fast32_t;

typedef signed char int_least8_t;
typedef short int_least16_t;
typedef int int_least32_t;

typedef int intmax_t;
typedef int intptr_t;

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;

typedef unsigned char uint_fast8_t;
typedef unsigned short uint_fast16_t;
typedef unsigned int uint_fast32_t;

typedef unsigned char uint_least8_t;
typedef unsigned short uint_least16_t;
typedef unsigned int uint_least32_t;

typedef unsigned int uintmax_t;
typedef unsigned int uintptr_t;

#define INTPTR_MIN INT_MIN
#define INTPTR_MAX INT_MAX
#define INTMAX_MIN INT_MIN
#define INTMAX_MAX INT_MAX

typedef float __fpmax_t;
