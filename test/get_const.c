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
/* Attempt to read constant */
#include "test.h"

int main(void)
{
  const char *filedir = "dirfile";
  const char *format = "dirfile/format";
  double c;
  int n, error, r = 0;
  DIRFILE *D;

  rmdirfile();
  mkdir(filedir, 0700);

  MAKEFORMATFILE(format, "const CONST FLOAT64 8.3\n");

  D = gd_open(filedir, GD_RDONLY | GD_VERBOSE);
  n = gd_get_constant(D, "const", GD_FLOAT64, &c);
  error = gd_error(D);

  gd_discard(D);

  unlink(format);
  rmdir(filedir);

  CHECKI(error, 0);
  CHECKI(n, 0);
  CHECKF(c, 8.3);

  return r;
}
