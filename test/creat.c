/* Creating a dirfile should succeed cleanly */
#include "../src/getdata.h"

#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

int main(void)
{
  const char* filedir = __TEST__ "dirfile";
  const char* format = __TEST__ "dirfile/format";

  DIRFILE* D = dirfile_open(filedir, GD_RDWR | GD_CREAT);
  int error = D->error;
  dirfile_close(D);

  if (unlink(format))
    return 1;

  if (rmdir(filedir))
    return 1;

  return (error != GD_E_OK);
}