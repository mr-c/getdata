/* Copyright (C) 2010-2011, 2013 D. V. Wiebe
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

#include <inttypes.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

int main(void)
{
  const char *filedir = "dirfile";
  const char *format = "dirfile/format";
  const char *format_data =
    "parent CARRAY UINT8 1\n"
    "parent/data1 CARRAY UINT8 1 2 3 4 5\n"
    "parent/data2 CARRAY UINT8 2 4 6 8 10 12\n"
    "parent/data3 CARRAY UINT8 3 6 9 12 15 18 21\n"
    "META parent data4 LINTERP UINT8 1\n";
  int fd, r = 0;
  size_t i, error;
  struct uint8_carrays {
    size_t n;
    uint8_t *d;
  } *field_list;
  DIRFILE *D;

  rmdirfile();
  mkdir(filedir, 0777);

  fd = open(format, O_CREAT | O_EXCL | O_WRONLY, 0666);
  write(fd, format_data, strlen(format_data));
  close(fd);

  D = gd_open(filedir, GD_RDONLY | GD_VERBOSE);
  field_list = (struct uint8_carrays *)gd_mcarrays(D, "parent", GD_UINT8);

  error = gd_error(D);
  CHECKI(error, 0);

  if (!r)
    for (fd = 0; fd < 3; ++fd) {
      CHECKUi(fd,field_list[fd].n, (size_t)(5 + fd));
      for (i = 0; i < field_list[fd].n; ++i)
        CHECKUi(fd * 1000 + i,field_list[fd].d[i], (fd + 1) * (i + 1));
    }


  gd_discard(D);
  unlink(format);
  rmdir(filedir);

  return r;
}
