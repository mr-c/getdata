/* Copyright (C) 2008-2011, 2013, 2016 D. V. Wiebe
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
#ifdef ENC_SKIP_TEST
  return 77;
#else
  const char *filedir = "dirfile";
  const char *format = "dirfile/format";
  const char *data = "dirfile/data";
  const char *encdata = "dirfile/data" ENC_SUFFIX;
  char command[4096];
  int error, r = 0;
  off_t n;
  DIRFILE *D;

  rmdirfile();
  mkdir(filedir, 0777);

  MAKEFORMATFILE(format, "data RAW UINT16 1\n");
  MAKEDATAFILE(data, uint16_t, i, 256);

  /* compress */
  ENC_COMPRESS;
  if (gd_system(command)) {
    perror(BZIP2);
    r = 1;
  }

#ifdef USE_ENC
  D = gd_open(filedir, GD_RDONLY | GD_VERBOSE);
#else
  D = gd_open(filedir, GD_RDONLY);
#endif
  n = gd_nframes(D);
  error = gd_error(D);

#ifdef USE_ENC
  CHECKI(error, 0);
  CHECKI(n, 256);
#else
  CHECKI(error, GD_E_UNSUPPORTED);
  CHECKI(n, GD_E_UNSUPPORTED);
#endif

  gd_discard(D);

  unlink(encdata);
  unlink(format);
  rmdir(filedir);

  return r;
#endif
}
