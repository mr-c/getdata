/* Attempt to read from before the BOF */
#include "../src/getdata.h"

#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>

int main(void)
{
  const char* filedir = __TEST__ "dirfile";
  const char* format = __TEST__ "dirfile/format";
  const char* data = __TEST__ "dirfile/data";
  const char* format_data = "/FRAMEOFFSET 100\ndata RAW UINT8 8\n";
  unsigned char c[8];
  unsigned char data_data[256];
  int fd, i, r = 0;

  memset(c, 0, 8);
  mkdir(filedir, 0777);

  for (fd = 0; fd < 256; ++fd)
    data_data[fd] = (unsigned char)fd;

  fd = open(format, O_CREAT | O_EXCL | O_WRONLY, 0666);
  write(fd, format_data, strlen(format_data));
  close(fd);

  fd = open(data, O_CREAT | O_EXCL | O_WRONLY, 0666);
  write(fd, data_data, 256);
  close(fd);

  DIRFILE* D = dirfile_open(filedir, GD_RDONLY | GD_VERBOSE);
  int n = getdata(D, "data", 5, 0, 1, 0, GD_UINT8, c);
  int error = get_error(D);

  dirfile_close(D);

  unlink(data);
  unlink(format);
  rmdir(filedir);

  if (error) {
    fprintf(stderr, "error=%i\n", error);
    r = 1;
  }
  if (n != 8) {
    fprintf(stderr, "n=%i\n", n);
    r = 1;
  }
  for (i = 0; i < 8; ++i)
    if (c[i] != 0) {
      fprintf(stderr, "c[%i]=%i\n", i, c[i]);
      r = 1;
    }

  return r;
}