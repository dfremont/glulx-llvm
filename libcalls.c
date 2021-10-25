
#include <stdint.h>
#include <limits.h>

// Unsigned integer division and remainder.

unsigned int __umodsi3(unsigned int a, unsigned int b) {
	if (__builtin_expect(b > INT_MAX, 0))
		return (a >= b ? a - b : a);
	int ib = (int) b;
	if (__builtin_expect(a > INT_MAX, 0)) {
		unsigned m = __mod((int) (a - INT_MAX - 1), ib);
		m += __mod(INT_MAX, ib) + 1;
		if (m >= ib)
			m -= ib;
		return m;
	} else {
		return __mod((int) a, ib);
	}
}

unsigned int __udivsi3(unsigned int a, unsigned int b) {
	if (__builtin_expect(b > INT_MAX, 0))
		return (a >= b ? 1 : 0);
	int ib = (int) b;
	if (__builtin_expect(a > INT_MAX, 0)) {
		unsigned d = __div((int) (a - INT_MAX - 1), ib);
		d += __div(INT_MAX, ib);
		if ((a - (d * b)) >= b)
			d += 1;
		return d;
	} else {
		return __div((int) a, ib);
	}
}
