/* Copyright (C) 2012 D. V. Wiebe
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
#include <errno.h>
#include <stdio.h>

int main(void)
{
  const char *filedir = "dirfile";
  const char *format = "dirfile/format";
  const char *format1 = "dirfile/format1";
  const char *data = "dirfile/data";
  const char *format_data = "/INCLUDE format1 A Z\n";
  const char *format1_data = "data RAW UINT8 8\nconst CONST INT64 11\n";
  unsigned char data_data[256];
  int fd, e1, e2, n, r = 0;
  DIRFILE *D;
  gd_entry_t E;

  rmdirfile();
  mkdir(filedir, 0777);

  for (fd = 0; fd < 256; ++fd)
    data_data[fd] = (unsigned char)fd;

  fd = open(format, O_CREAT | O_EXCL | O_WRONLY, 0666);
  write(fd, format_data, strlen(format_data));
  close(fd);

  fd = open(format1, O_CREAT | O_EXCL | O_WRONLY, 0666);
  write(fd, format1_data, strlen(format1_data));
  close(fd);

  fd = open(data, O_CREAT | O_EXCL | O_WRONLY | O_BINARY, 0666);
  write(fd, data_data, 256);
  close(fd);

  D = gd_open(filedir, GD_RDWR);
  gd_entry(D, "AdataZ", &E);
  E.scalar[0] = "const";
  gd_alter_entry(D, "AdataZ", &E, 0);
  e1 = gd_error(D);

  E.scalar[0] = "AconstZ";
  gd_alter_entry(D, "AdataZ", &E, 0);
  e2 = gd_error(D);

  E.scalar[0] = NULL;
  gd_free_entry_strings(&E);

  n = gd_entry(D, "AdataZ", &E);

  gd_discard(D);

  unlink(data);
  unlink(format1);
  unlink(format);
  rmdir(filedir);

  CHECKI(e1, GD_E_BAD_CODE);
  CHECKI(e2, 0);
  CHECKI(n, 0);
  CHECKI(E.EN(raw,spf), 11);
  gd_free_entry_strings(&E);

  return r;
}