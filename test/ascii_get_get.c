/* Attempt to read UINT8 */
#include "test.h"

#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>

int main(void)
{
  const char* filedir = __TEST__ "dirfile";
  const char* format = __TEST__ "dirfile/format";
  const char* data = __TEST__ "dirfile/data.txt";
  const char* format_data = "data RAW UINT8 8\n";
  unsigned char c[8], d[8];
  int fd, i, n, error, n2, error2, r = 0;
  DIRFILE *D;
  FILE* stream;

  memset(c, 0, 8);
  mkdir(filedir, 0777);

  fd = open(format, O_CREAT | O_EXCL | O_WRONLY, 0666);
  write(fd, format_data, strlen(format_data));
  close(fd);

  stream = fopen(data, "w" FOPEN_TEXT);
  for (i = 0; i < 256; ++i)
    fprintf(stream, "%i\n", i);
  fclose(stream);

  D = gd_open(filedir, GD_RDONLY | GD_VERBOSE);
  n = gd_getdata(D, "data", 5, 0, 1, 0, GD_UINT8, c);
  error = gd_error(D);

  n2 = gd_getdata(D, "data", 5, 0, 1, 0, GD_UINT8, d);
  error2 = gd_error(D);

  gd_close(D);

  unlink(data);
  unlink(format);
  rmdir(filedir);

  CHECKI(error, 0);
  CHECKI(n, 8);
  CHECKI(error2, 0);
  CHECKI(n2, 8);
  for (i = 0; i < 8; ++i) {
    CHECKIi(i,c[i], 40 + i);
    CHECKIi(i+8,d[i], 40 + i);
  }

  return r;
}