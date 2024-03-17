/* Public domain.  */
#include <stddef.h>

__attribute__(( section( ".text$B" ) ))
void *
memset (void *dest, int val, size_t len)
{
  unsigned char *ptr = dest;
  while (len-- > 0)
    *ptr++ = val;
  return dest;
}
