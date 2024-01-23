#include <stdio.h>

void* __get_stdin () { return stdin; }
void* __get_stdout() { return stdout; }
void* __get_stderr() { return stderr; }
