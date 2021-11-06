
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <ctype.h>

size_t strlen(const char *str) {
	size_t len = 0;
	while (*str++)
		++len;
	return len;
}

size_t strnlen(const char *str, size_t n) {
	size_t len = 0;
	if (!str)
		return 0;
	while (n-- && *str++)
		++len;
	return len;
}

int strcmp(const char *lhs, const char *rhs) {
	const unsigned char *lp = (const unsigned char *) lhs;
	const unsigned char *rp = (const unsigned char *) rhs;
	while (1) {
		int diff = *lp - *rp;
		if (diff != 0)
			return diff;
		if (!*lp)
			return 0;
		++lp;
		++rp;
	}
}

int strncmp(const char *lhs, const char *rhs, size_t n) {
	const unsigned char *lp = (const unsigned char *) lhs;
	const unsigned char *rp = (const unsigned char *) rhs;
	while (n--) {
		int diff = *lp - *rp;
		if (diff != 0)
			return diff;
		if (!*lp)
			return 0;
		++lp;
		++rp;
	}
	return 0;
}

char *strchr(const char *str, int ch) {
	do {
		if (*str == (char) ch)
			return (char *) str;
	} while(*str++);
	return NULL;
}

char *strcpy(char *restrict dst, const char *restrict src) {
	char *p = dst;
	while ((*p++ = *src++))
		;
	return dst;
}

char *strncpy(char *restrict dst, const char *restrict src, size_t n) {
	for (char *p = dst; n > 0 ; --n) {
		*p++ = *src;
		if (*src)
			++src;
	}
	return dst;
}

char *strcat(char *restrict dst, const char *restrict src) {
	char *p = dst;
	while (*p)
		++p;
	while ((*p++ = *src++))
		;
	return dst;
}

long strtol(const char *restrict str, char **restrict str_end, int base) {
	// Reject disallowed bases.
	if (base < 0 || base == 1 || base > 36) {
		errno = EINVAL;
		return 0;
	}

	const char *cur = str;

	// Skip leading whitespace.
	while (isspace(*cur))
		++cur;

	// Check for sign prefix.
	int negative = 0;
	if (*cur == '-') {
		negative = 1;
		++cur;
	} else if (*cur == '+') {
		++cur;
	}

	// Check for base prefix and decide on base.
	if ((base == 0 || base == 16) && *cur == '0' &&
		(cur[1] == 'x' || cur[1] == 'X')) {
		base = 16;
		cur += 2;
	} else if ((base == 0 || base == 8) && *cur == '0') {
		base = 8;
		++cur;
	} else if (base == 0) {
		base = 10;
	}

	// Accumulate digits.
	unsigned long value = 0;
	unsigned long max = ((unsigned long) LONG_MAX) + 1;	// -LONG_MIN
	unsigned long thresh = max / base;	// beyond which, overflow!
	int overflow = 0;
	while (1) {
		int digit = *cur;
		if (!isalnum(digit))
			break;
		if (digit >= 'a')
			digit -= 32;	// convert to uppercase
		if (digit >= 'A')
			digit -= 7;		// so that A-Z come immediately after 0-9
		digit -= '0';
		// digit is now between zero and 35
		if (digit >= base)
			break;

		// Carefully append the digit onto value, checking for overflow.
		if (value <= thresh) {
			value *= base;
			if (value <= max - digit)
				value += digit;
			else
				overflow = 1;
		} else {
			overflow = 1;
		}
		++cur;
	}

	if (str_end)
		*str_end = (char *) cur;

	// Special case to handle LONG_MIN (whose negation doesn't fit in a long).
	if (value == max) {
		if (negative)
			return LONG_MIN;
		else
			overflow = 1;
	}

	if (overflow) {
		errno = ERANGE;
		return (negative ? LONG_MIN : LONG_MAX);
	} else {
		return (negative ? -value : value);
	}
}

int atoi(const char *str) {
	return strtol(str, (char **) 0, 10);
}

int memcmp(const void *lhs, const void *rhs, size_t n) {
	const unsigned char *lp = lhs;
	const unsigned char *rp = rhs;
	while (n--) {
		int diff = *lp++ - *rp++;
		if (diff != 0)
			return diff;
	}
	return 0;
}

__attribute__((always_inline))
void *memcpy(void *restrict dst, const void *restrict src, size_t n) {
	// Drop restrict qualifier on src since @mcopy handles overlapping blocks.
	__mcopy(n, (const void *) src, dst);
	return dst;
}

__attribute__((always_inline))
void *memmove(void *dst, const void *src, size_t n) {
	__mcopy(n, src, dst);
	return dst;
}

// Backend should detect memset(dst, 0, n) and emit @mzero instead.
void *memset(void *dst, int ch, size_t n) {
	unsigned char *ptr = dst;
	while (n--)
		*ptr++ = (unsigned char) ch;
	return dst;
}
