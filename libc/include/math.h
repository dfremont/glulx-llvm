
#include <errno.h>

#define MATH_ERRNO 1
#define MATH_ERREXCEPT 2
#define math_errhandling MATH_ERRNO

#define INFINITY (__builtin_inff())
#define HUGE_VALF INFINITY
#define HUGE_VAL INFINITY
#define NAN (__builtin_nanf(""))

#define isinf(arg) __builtin_isinf(arg)
#define isfinite(arg) __builtin_isfinite(arg)
#define isnan(arg) __builtin_isnan(arg)

double ldexp(double arg, int exp);
double frexp(double arg, int *exp);

double sqrt(double arg);
double exp(double arg);
double log(double arg);
double pow(double base, double exponent);

double sin(double arg);
double cos(double arg);
double tan(double arg);
double asin(double arg);
double acos(double arg);
double atan(double arg);
double atan2(double x, double y);

double ceil(double arg);
double floor(double arg);
