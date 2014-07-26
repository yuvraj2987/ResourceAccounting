

#include <assert.h>

#ifdef __cplusplus
extern "C"
#endif
char daemon();

int main() {
#if defined (__stub_daemon) || defined (__stub___daemon)
  fail fail fail
#else
  daemon();
#endif

  return 0;
}
