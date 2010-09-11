#include "test.h"

#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

int main(void)
{
  const char* filedir = __TEST__ "dirfile";
  const char* format = __TEST__ "dirfile/format";
  const char* format_data =
    "/VERSION 8\n"
    "ar RAW UINT8 8\n"
    "FRAMEOFFSET DIVIDE ar ar\n"
    "r RECIP ar 1.\n";
  int fd, r = 0;

  mkdir(filedir, 0777);

  fd = open(format, O_CREAT | O_EXCL | O_WRONLY, 0666);
  write(fd, format_data, strlen(format_data));
  close(fd);

  DIRFILE* D = gd_open(filedir, GD_RDWR | GD_VERBOSE);
  int e = gd_dirfile_standards(D, 8);
  int q = gd_rewrite_fragment(D, 0);
  gd_close(D);

  D = gd_open(filedir, GD_RDONLY | GD_VERBOSE);
  int c = gd_dirfile_standards(D, GD_VERSION_CURRENT);
  gd_close(D);

  unlink(format);
  rmdir(filedir);

  CHECKI(e,8);
  CHECKI(q,0);
  CHECKI(c,8);

  return r;
}