

#include <time.h>
#include <stdio.h>

int main(int argc, char* argv[])
{
    printf("--- Get Time starts ----\n");
    struct timespec first, second;
    /*clock_gettime(CLOCK_MONOTONIC, &first);
    clock_gettime(CLOCK_MONOTONIC, &second);
    long diff = second.tv_nsec - first.tv_nsec;
    printf("Time diff: %ld in nano seconds\n", diff);
    long micro_diff = diff/1000;
    printf("Time diff: %ld in micro seconds\n", micro_diff);
    */
    register short int count = 0;
    clock_gettime(CLOCK_MONOTONIC, &first);
    clock_gettime(CLOCK_MONOTONIC, &second);
    /*
    while(second.tv_nsec - first.tv_nsec< 1000000)
    {
        clock_gettime(CLOCK_MONOTONIC, &second);
        count++; 
    }
  
    printf("For 1ms count is:%d\n", count);*/

    printf("%lld.%.9ld", (long long) first.tv_sec, first.tv_nsec);
}
