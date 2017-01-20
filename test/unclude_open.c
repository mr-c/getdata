/* Copyright (C) 2013, 2017 D.V. Wiebe
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
  const char *data = "dirfile/data";
  const char *format = "dirfile/format";
  const char *format1 = "dirfile/format1";
  const char *format2 = "dirfile/format2";
  int ret, e1, e2, unlink_format1, unlink_format2, r = 0;
  unsigned int nfields, nfragments;
  uint8_t c = 3;
  DIRFILE *D;

  rmdirfile();
  mkdir(filedir, 0700);

  MAKEFORMATFILE(format, "/INCLUDE format1\na CONST UINT8 1\n");
  MAKEFORMATFILE(format1, "data RAW UINT8 11\n/INCLUDE format2\n");
  MAKEFORMATFILE(format2, "c CONST UINT8 11\n");
  MAKEDATAFILE(data, unsigned char, i, 256);

  D = gd_open(filedir, GD_RDWR | GD_VERBOSE);
  gd_putdata(D, "data", 0, 0, 0, 1, GD_UINT8, &c);
  ret = gd_uninclude(D, 1, 0);
  e1 = gd_error(D);
  CHECKI(ret,0);
  CHECKI(e1, 0);

  nfields = gd_nfields(D);
  CHECKI(nfields,2);

  nfragments = gd_nfragments(D);
  CHECKI(nfragments,1);

  e2 = gd_close(D);
  CHECKI(e2, 0);

  unlink_format2 = unlink(format2);
  unlink_format1 = unlink(format1);
  unlink(data);
  unlink(format);
  rmdir(filedir);

  CHECKI(unlink_format2,0);
  CHECKI(unlink_format1,0);

  return r;
}
