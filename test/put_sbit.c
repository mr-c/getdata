/* Copyright (C) 2009-2011, 2013, 2017 D.V. Wiebe
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
/* Attempt to write BIT */
#include "test.h"

int main(void)
{
  const char *filedir = "dirfile";
  const char *format = "dirfile/format";
  const char *data = "dirfile/data";
  int8_t c[8];
  int8_t d = 0xA5;
  int fd, i, n, error, r = 0;
  DIRFILE *D;

  rmdirfile();
  mkdir(filedir, 0700);

  for (i = 0; i < 8; ++i)
    c[i] = i;

  MAKEFORMATFILE(format, "bit SBIT data 2 3\ndata RAW INT8 8\n");

  fd = open(data, O_CREAT | O_EXCL | O_WRONLY | O_BINARY, 0666);
  for (i = 0; i < 50; ++i)
    write(fd, &d, sizeof(int8_t));
  close(fd);

  D = gd_open(filedir, GD_RDWR | GD_UNENCODED | GD_VERBOSE);
  n = gd_putdata(D, "bit", 5, 0, 1, 0, GD_INT8, c);
  error = gd_error(D);

  gd_discard(D);

  fd = open(data, O_RDONLY | O_BINARY);
  i = 0;
  while (read(fd, &d, sizeof(int8_t))) {
    if (i < 40 || i >= 48) {
      CHECKIi(i,d,-91);
    } else
      CHECKIi(i,d,(-95 | (i - 40) << 2));
    i++;
  }
  close(fd);

  unlink(data);
  unlink(format);
  rmdir(filedir);

  CHECKI(n,8);
  CHECKI(error, 0);

  return r;
}
