#include <stdint.h>
int64_t data[4];
void alloc4(int64_t **p, int64_t a, int64_t b, int64_t c, int64_t d)
{
	*p = &data[0];
	**p = a;
	*(*p + 1) = b;
	*(*p + 2) = c;
	*(*p + 3) = d;
}
