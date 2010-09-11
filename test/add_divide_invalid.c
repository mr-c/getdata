/* Getting data from an invalid dirfile should fail cleanly */
#include "test.h"

#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

int main(void)
{
  const char* filedir = __TEST__ "dirfile";
  int r = 0;

  DIRFILE* D = gd_open(filedir, GD_RDONLY);
  gd_add_multiply(D, "new", "in1", "in2", 0);
  int error = gd_error(D);
  gd_close(D);

  CHECKI(error, GD_E_BAD_DIRFILE);
  return r;
}