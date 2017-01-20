/* Copyright (C) 2008-2011, 2013, 2017 D.V. Wiebe
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
/* Closing a dirfile should succeed cleanly */
#include "test.h"

int main(void)
{
  const char *filedir = "dirfile";
  const char *format = "dirfile/format";
  const char *data = "dirfile/data";
  int ret, n, r = 0;
  DIRFILE *D;

  rmdirfile();
  mkdir(filedir, 0700);
  MAKEEMPTYFILE(format, 0600);

  D = gd_open(filedir, GD_RDWR | GD_UNENCODED | GD_VERBOSE);
  gd_add_spec(D, "data RAW UINT8 1", 0);
  ret = gd_discard(D);
  D = gd_open(filedir, GD_RDONLY | GD_VERBOSE);
  n = gd_nfields(D);
  gd_discard(D);

  unlink(data);
  unlink(format);
  rmdir(filedir);

  CHECKI(ret, 0);
  CHECKI(n, 1);

  return r;
}
