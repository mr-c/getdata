/* Copyright (C) 2010-2011, 2013, 2017 D.V. Wiebe
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
/* Attempt to write LINTERP */
#include "test.h"

int main(void)
{
  const char *filedir = "dirfile";
  const char *format = "dirfile/format";
  const char *data = "dirfile/data";
  const char *table = "dirfile/table";
  int8_t c[8];
  int i, n, error, r = 0;
  DIRFILE *D;
  FILE *t;

  memset(c, 0, 8);
  rmdirfile();
  mkdir(filedir, 0700);

  for (i = 0; i < 8; ++i)
    c[i] = (int8_t)(40 + i);

  MAKEFORMATFILE(format, "linterp LINTERP data ./table\ndata RAW INT8 8\n");

  t = fopen(table, "wt");
  for (i = 0; i < 10; ++i)
    fprintf(t, "%i %i\n", i * 6, (i - 5) * (i - 5));
  fclose(t);

  D = gd_open(filedir, GD_RDWR | GD_UNENCODED);
  n = gd_putdata(D, "linterp", 5, 0, 1, 0, GD_INT8, c);
  error = gd_error(D);

  gd_discard(D);

  unlink(table);
  unlink(data);
  unlink(format);
  rmdir(filedir);

  CHECKI(n,0);
  CHECKI(error, GD_E_DOMAIN);

  return r;
}
