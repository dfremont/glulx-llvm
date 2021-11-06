
#include <errno.h>
#include <math.h>

int isinteger(double arg) {
	return (__ceil(arg) == arg);
}

double ldexp(double arg, int exp) {
	// TODO improve
	if (exp >= 0) {
		while (exp--)
			arg *= 2;
	} else {
		while (exp++)
			arg /= 2;
	}
	return arg;
}

double frexp(double arg, int *exp) {
	*exp = 0;
	if (arg == 0) {
		return 0;
	} else if (isinf(arg) || isnan(arg)) {
		return arg;
	} else if (arg > 0) {
		while (arg > 1) {
			arg /= 2;
			*exp += 1;
		}
		while (arg < 0.5) {
			arg *= 2;
			*exp -= 1;
		}
	} else {
		while (arg < -1) {
			arg /= 2;
			*exp += 1;
		}
		while (arg > -0.5) {
			arg *= 2;
			*exp -= 1;
		}
	}
	return arg;
}

double sqrt(double arg) {
	double res = __sqrt(arg);
	if (arg < 0)
		errno = EDOM;
	return res;
}

double exp(double arg) {
	return __exp(arg);
}

double log(double arg) {
	double res = __log(arg);
	if (arg < 0)
		errno = EDOM;
	return res;
}

double pow(double base, double exponent) {
	double res = __pow(base, exponent);
	if (isfinite(base) && base < 0 &&
		isfinite(exponent) && !isinteger(exponent))
		errno = EDOM;
	return res;
}

double sin(double arg) {
	double res = __sin(arg);
	if (isinf(arg))
		errno = EDOM;
	return res;
}

double cos(double arg) {
	double res = __cos(arg);
	if (isinf(arg))
		errno = EDOM;
	return res;
}

double tan(double arg) {
	double res = __tan(arg);
	if (isinf(arg))
		errno = EDOM;
	return res;
}

double asin(double arg) {
	double res = __asin(arg);
	if (arg < -1.0 || arg > 1.0)
		errno = EDOM;
	return res;
}

double acos(double arg) {
	double res = __acos(arg);
	if (arg < -1.0 || arg > 1.0)
		errno = EDOM;
	return res;
}

double atan(double arg) {
	return __atan(arg);
}

double atan2(double x, double y) {
	return __atan2(x, y);
}

double ceil(double arg) {
	return __ceil(arg);
}

double floor(double arg) {
	return __floor(arg);
}
