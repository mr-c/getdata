/* Copyright (C) 2015 D. V. Wiebe
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
#ifndef TEST_FLAC
  return 77;
#else
  const char *filedir = "dirfile";
  const char *format = "dirfile/format";
  const char *data_flac = "dirfile/data.flac";
  const char *data = "dirfile/data";
  const char *format_data = "data RAW UINT16 8\n";
  uint16_t c[8];
#ifdef USE_FLAC
  char command[4096];
  uint16_t d;
#endif
  struct stat buf;
  int fd, i, n, e1, e2, stat_data, unlink_data, unlink_flac, r = 0;
  DIRFILE *D;

  memset(c, 0, 8);
  rmdirfile();
  mkdir(filedir, 0777);

  for (i = 0; i < 8; ++i)
    c[i] = 0x0101 * i;

  fd = open(format, O_CREAT | O_EXCL | O_WRONLY, 0666);
  write(fd, format_data, strlen(format_data));
  close(fd);

#ifdef USE_FLAC
  D = gd_open(filedir, GD_RDWR | GD_FLAC_ENCODED | GD_BIG_ENDIAN | GD_VERBOSE);
#else
  D = gd_open(filedir, GD_RDWR | GD_FLAC_ENCODED | GD_BIG_ENDIAN);
#endif
  n = gd_putdata(D, "data", 5, 0, 1, 0, GD_UINT16, c);
  e1 = gd_error(D);

  e2 = gd_close(D);
  CHECKI(e2, 0);

  stat_data = stat(data_flac, &buf);

#ifdef USE_FLAC
  CHECKI(e1, GD_E_OK);
  CHECKI(n, 8);
  if (stat_data) {
    perror("stat");
  }
  CHECKI(stat_data, 0);
#else
  CHECKI(e1, GD_E_UNSUPPORTED);
  CHECKI(n, 0);
  CHECKI(stat_data, -1);
#endif

#ifdef USE_FLAC
  snprintf(command, 4096, "%s --silent --decode --delete-input-file "
      "--force-raw-format --sign=signed --endian=big %s --output-name=%s",
      FLAC, data_flac, data);
  if (gd_system(command)) {
    r = 1;
  } else {
    fd = open(data, O_RDONLY | O_BINARY);
    if (fd >= 0) {
      i = 0;
      while (read(fd, &d, sizeof d)) {
        if (i < 40 || i > 48) {
          CHECKXi(i, d, 0);
        } else
          CHECKXi(i, d, (i - 40) * 0x0101);
        i++;
      }
      CHECKI(i, 48);
      close(fd);
    }
  }
#endif

  unlink_flac = unlink(data_flac);
  CHECKI(unlink_flac, -1);

  unlink_data = unlink(data);
  unlink(format);
  rmdir(filedir);

#ifdef USE_FLAC
  CHECKI(unlink_data, 0);
#else
  CHECKI(unlink_data, -1);
#endif

  return r;
#endif
}