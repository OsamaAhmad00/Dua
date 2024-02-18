// To suppress warnings about using fscanf
#pragma warning(disable : 4996)

#include <stdio.h>
#include <errno.h>

void* c_stdin () { return stdin; }
void* c_stdout() { return stdout; }
void* c_stderr() { return stderr; }

int c_errno() { return errno; }

int c_EOF() { return EOF; }
int c_ERANGE() { return ERANGE; }

int c_scan_str(void* stream, char* buffer, int* read_count)
{
    return fscanf(stream, "%s%n", buffer, read_count);
}