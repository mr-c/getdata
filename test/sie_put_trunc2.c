/* Copyright (C) 2013 D. V. Wiebe
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

int main(void)
{
  const char *filedir = "dirfile";
  const char *format = "dirfile/format";
  const char *data = "dirfile/data.sie";
  const char *format_data = "data RAW UINT8 8\n/ENCODING sie\n/ENDIAN little\n";
  unsigned char c[0x32];
  const uint8_t data_data[] = {
    0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x12,
    0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x22,
    0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x32
  };
#define NREC 2
  const uint8_t data_out[] = {
    0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x12,
    0x33, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x29
  };
  uint8_t check[(NREC + 1) * 9];
  DIRFILE *D;
  int fd, i, n, error, r = 0;
  ssize_t l;

  memset(c, 0x29, 0x32);
  rmdirfile();
  mkdir(filedir, 0777); 

  fd = open(format, O_CREAT | O_EXCL | O_WRONLY, 0666);
  write(fd, format_data, strlen(format_data));
  close(fd);

  fd = open(data, O_CREAT | O_EXCL | O_WRONLY | O_BINARY, 0666);
  write(fd, data_data, 3 * 9 * sizeof(unsigned char));
  close(fd);

  D = gd_open(filedir, GD_RDWR | GD_VERBOSE);
  n = gd_putdata(D, "data", 0, 2, 0, 0x32, GD_UINT8, c);
  error = gd_error(D);
  CHECKI(error, 0);
  CHECKI(n, 0x32);

  gd_discard(D);

  fd = open(data, O_RDONLY | O_BINARY);
  l = read(fd, check, (NREC + 1) * 9);
  close(fd);
  CHECKI(l, NREC * 9);

  unlink(data);
  unlink(format);
  rmdir(filedir);

  for (i = 0; i < NREC * 9; ++i)
    CHECKXi(i, check[i], data_out[i]);

  return r;
}
