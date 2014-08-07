
#include <Python.h>
#include <time.h>
#include <stdio.h>

static PyObject * py_get_time(PyObject *self, PyObject *args)
{
  struct timespec first;
  clock_gettime(CLOCK_MONOTONIC, &first);
  return Py_BuildValue("(ll)", first.tv_sec, first.tv_nsec);
}


static PyMethodDef timeModule_methods[] = {
  {"time", py_get_time, METH_NOARGS, "Get monotonic time clock from c function"},
  { NULL, NULL, 0, NULL}
};

PyMODINIT_FUNC inittimeModule(void)
{
  (void) Py_InitModule("timeModule", timeModule_methods);
}



/*
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
  /*  register short int count = 0;
    clock_gettime(CLOCK_MONOTONIC, &first);
    clock_gettime(CLOCK_MONOTONIC, &second);
 
    while(second.tv_nsec - first.tv_nsec< 1000000)
    {
        clock_gettime(CLOCK_MONOTONIC, &second);
        count++; 
    }

    printf("For 1ms count is:%d\n", count);
}*/
