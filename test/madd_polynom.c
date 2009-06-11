/* Add a POLYNOM field */
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

  DIRFILE* D = dirfile_open(filedir, GD_RDWR | GD_CREAT | GD_VERBOSE);
  dirfile_add_phase(D, "new", "in", 3, 0);
  const char* in_field = "in";
  const double a[4] = {1, 0.3, 0.5, 1.8};
  dirfile_madd_polynom(D, "new", "meta", 3, in_field, a);
  int error = get_error(D);

  /* check */
  int n = get_nmfields(D, "new");

  dirfile_close(D);

  unlink(format);
  rmdir(filedir);

  if (n != 1)
    return 1;

  return (error != GD_E_OK);
}
