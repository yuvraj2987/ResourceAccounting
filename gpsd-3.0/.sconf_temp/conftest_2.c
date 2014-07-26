

#include <assert.h>

#ifdef __cplusplus
extern "C"
#endif
char strlcpy();

int main() {
#if defined (__stub_strlcpy) || defined (__stub___strlcpy)
  fail fail fail
#else
  strlcpy();
#endif

  return 0;
}
