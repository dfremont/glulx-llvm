
#include <stdlib.h>
#include <string.h>

_Noreturn void exit(int exit_code) {
	__quit();
}

void *malloc(size_t size) {
	size_t *data = __malloc(sizeof(size_t) + size);
	if (!data) return NULL;
	data[0] = size;		// need to keep track of this so we can realloc
	return &data[1];
}

void *calloc(size_t num, size_t size) {
	void *ptr = malloc(num * size);
	memset(ptr, 0, num * size);
	return ptr;
}

void *realloc(void *ptr, size_t new_size) {
	if (!ptr) return malloc(new_size);
	size_t *data = ptr;
	size_t old_size = data[-1];
	void *new = malloc(new_size);
	memcpy(ptr, new, old_size);
	free(ptr);
	return new;
}

void free(void *ptr) {
	size_t *data = ptr;
	__mfree(&data[-1]);
}

// I'm using mergesort here since it's simple to implement among algorithms
// with good worst-case performance. I haven't bothered making it in-place;
// pull requests with improvements are welcome!

static void qsort_helper(void *ptr, void *tmp, size_t size,
						 int (*comp)(const void *, const void *),
						 size_t i, size_t j) {
	size_t count = j - i;
	if (count == 1)
		return;

	size_t halfway = i + (count / 2);
	qsort_helper(ptr, tmp, size, comp, i, halfway);
	qsort_helper(ptr, tmp, size, comp, halfway, j);

	void *left = ptr + (i * size);
	void *right = ptr + (halfway * size);
	void *left_end = right;
	void *right_end = ptr + (j * size);
	void *out = tmp + (i * size);
	while (left < left_end && right < right_end) {
		if (comp(left, right) <= 0) {
			memcpy(out, left, size);
			left += size;
		} else {
			memcpy(out, right, size);
			right += size;
		}
		out += size;
	}
	while (left < left_end) {
		memcpy(out, left, size);
		left += size;
		out += size;
	}
	while (right < right_end) {
		memcpy(out, right, size);
		right += size;
		out += size;
	}
	memcpy(ptr + (i * size), tmp + (i * size), count * size);
}

void qsort(void *ptr, size_t count, size_t size,
           int (*comp)(const void *, const void *)) {
	if (count <= 1)
		return;

	size_t total_size = count * size;
	void *tmp = __malloc(count * size);
	qsort_helper(ptr, tmp, size, comp, 0, count);
	__mfree(tmp);
}
