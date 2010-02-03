/* Try to read LINCOM entry */
#include "../src/getdata.h"

#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <math.h>
#include <stdio.h>

int main(void)
{
  const char* filedir = __TEST__ "dirfile";
  const char* format = __TEST__ "dirfile/format";
  const char* format_data =
    "m1 CONST COMPLEX128 1.1;3.2\n"
    "b1 CONST COMPLEX64 2.2;9.3\n"
    "data LINCOM 1 in1 m1.r b1.i\n";
  int fd, r = 0;

  mkdir(filedir, 0777);

  fd = open(format, O_CREAT | O_EXCL | O_WRONLY, 0666);
  write(fd, format_data, strlen(format_data));
  close(fd);

  DIRFILE* D = dirfile_open(filedir, GD_RDONLY | GD_VERBOSE);
  gd_entry_t E;

  int n = get_entry(D, "data", &E);
  int error = get_error(D);

  dirfile_close(D);
  unlink(format);
  rmdir(filedir);

  if (error != GD_E_OK) {
    fprintf(stderr, "error = %i\n", error);
    r = 1;
  }

  if (n) {
    fprintf(stderr, "n = %i\n", n);
    r = 1;
  }

  if (strcmp(E.field, "data")) {
    fprintf(stderr, "E.field = %s\n", E.field);
    r = 1;
  }

  if (E.field_type != GD_LINCOM_ENTRY) {
    fprintf(stderr, "E.field_type = %i\n", E.field_type);
    r = 1;
  }

  if (E.n_fields != 1) {
    fprintf(stderr, "E.n_fields = %i\n", E.n_fields);
    r = 1;
  }

  if (strcmp(E.in_fields[0], "in1")) {
    fprintf(stderr, "E.in_fields[0] = %s\n", E.in_fields[0]);
    r = 1;
  }

  if (strcmp(E.scalar[0], "m1.r")) {
    fprintf(stderr, "E.scalar[0] = %s\n", E.scalar[0]);
    r = 1;
  }

  if (strcmp(E.scalar[GD_MAX_LINCOM], "b1.i")) {
    fprintf(stderr, "E.scalar[GD_MAX_LINCOM] = %s\n", E.scalar[GD_MAX_LINCOM]);
    r = 1;
  }

  if (fabs(E.m[0] - 1.1) > 1e-10) {
    fprintf(stderr, "E.m[0] = %g\n", E.m[0]);
    r = 1;
  }

  if (fabs(E.b[0] - 9.3) > 1e-10) {
    fprintf(stderr, "E.b[0] = %g\n", E.b[0]);
    r = 1;
  }

  return r;
}