/* Copyright (C) 2014 D. V. Wiebe
 *
 ***************************************************************************
 *
 * This file is part of the GetData project.
 *
 * GetData is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation; either version 2.1 of the License, or (at your
 * option) any later version.
 *
 * GetData is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
 * License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with GetData; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */
#include "test.h"

#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <inttypes.h>
#include <errno.h>
#include <stdio.h>

int main(void)
{
  const char *filedir = "dirfile";
  const char *format = "dirfile/format";
  const char *format_data = "sarray SARRAY a b c d e f g h i j k l m n\n";
  const char *data[] = {"a", "b", "c", "d", "e"};
  int fd, i, ret, error, n, r = 0;
  size_t z;
  DIRFILE *D;
  const char *d[5];

  rmdirfile();
  mkdir(filedir, 0777);

  fd = open(format, O_CREAT | O_EXCL | O_WRONLY | O_BINARY, 0666);
  write(fd, format_data, strlen(format_data));
  close(fd);

  D = gd_open(filedir, GD_RDWR | GD_VERBOSE);
  ret = gd_alter_sarray(D, "sarray", 5);
  error = gd_error(D);
  CHECKI(error, 0);
  CHECKI(ret, 0);

  z = gd_sarray_len(D, "sarray");
  CHECKU(z, 5);

  n = gd_get_sarray(D, "sarray", d);
  CHECKI(n, 0);
  for (i = 0; i < 5; ++i)
    CHECKSi(i, d[i], data[i]);

  gd_discard(D);

  unlink(format);
  rmdir(filedir);

  return r;
}
