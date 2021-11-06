
#ifdef NDEBUG
#define assert(condition) ((void)0)
#else
#define assert(condition) _assert(condition, #condition, __func__, __FILE__, __LINE__)

void _assert(int condition, const char *cond_str,
	const char *func, const char *file, int line);
#endif
