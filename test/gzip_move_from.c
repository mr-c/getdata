/* Copyright (C) 2016 D. V. Wiebe
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

#ifndef TEST_GZIP
#define ENC_SKIP_TEST 1
#endif

#ifdef USE_GZIP
#define USE_ENC 1
#endif

#define ENC_SUFFIX ".gz"
#define ENC_NAME "gzip"
#define ENC_COMPRESS \
  snprintf(command, 4096, "%s -f %s > /dev/null", GZIP, data)

#include "enc_move_from.c"
