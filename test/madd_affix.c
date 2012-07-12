/* Copyright (C) 2011 D. V. Wiebe
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

int main(void)
{
  const char *filedir = "dirfile";
  const char *format = "dirfile/format";
  const char *format1 = "dirfile/format1";
  const char *data = "dirfile/data";
  const char *format_data = "/INCLUDE format1 A Z\n";
  int fd, e1, e2, e3, r = 0;
  DIRFILE *D;
  gd_entry_t E, e;

  rmdirfile();
  mkdir(filedir, 0777);

  memset(&E, 0, sizeof(E));
  E.field = "data";
  E.field_type = GD_RAW_ENTRY;
  E.fragment_index = 1;
  E.EN(raw,spf) = 2;
  E.EN(raw,data_type) = GD_UINT8;
  E.scalar[0] = NULL;

  fd = open(format, O_CREAT | O_EXCL | O_WRONLY, 0666);
  write(fd, format_data, strlen(format_data));
  close(fd);

  close(open(format1, O_CREAT | O_EXCL | O_WRONLY, 0666));

  D = gd_open(filedir, GD_RDWR | GD_UNENCODED | GD_VERBOSE);
  gd_add(D, &E);
  e1 = gd_error(D);

  E.field_type = GD_CONST_ENTRY;
  E.EN(scalar,const_type) = GD_UINT8;
  E.fragment_index = 99; /* ignored */

  gd_madd(D, &E, "AdataZ");
  e2 = gd_error(D);

  /* check */
  gd_entry(D, "AdataZ/data", &e);
  e3 = gd_error(D);
  gd_close(D);

  unlink(data);
  unlink(format);
  unlink(format1);
  rmdir(filedir);

  CHECKI(e1, GD_E_OK);
  CHECKI(e2, GD_E_OK);
  CHECKI(e3, GD_E_OK);
  if (e3 == 0) {
    CHECKI(e.field_type, GD_CONST_ENTRY);
    CHECKI(e.fragment_index, 1);
    CHECKI(e.EN(scalar,const_type), GD_UINT8);
    gd_free_entry_strings(&e);
  }

  return r;
}
