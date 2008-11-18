/* (C) 2008 D. V. Wiebe
 *
 ***************************************************************************
 *
 * This file is part of the GetData project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * GetData is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with GetData; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */
#include "internal.h"

#ifdef STDC_HEADERS
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#endif

#define BUFFER_SIZE 9000000

static void _GD_ShiftFragment(DIRFILE* D, off64_t offset, int fragment,
    int move)
{
  unsigned int i, n_raw = 0;

  dtrace("%p, %lli, %i, %i\n", D, (long long)offset, fragment, move);

  /* check protection */
  if (D->fragment[fragment].protection & GD_PROTECT_FORMAT) {
    _GD_SetError(D, GD_E_PROTECTED, GD_E_PROTECTED_FORMAT, NULL, 0,
        D->fragment[fragment].cname);
    dreturnvoid();
    return;
  }

  if (move && offset != D->fragment[fragment].frame_offset) {
    gd_entry_t **raw_entry = malloc(sizeof(gd_entry_t*) * D->n_entries);
    const struct encoding_t* enc;
    void *buffer = malloc(BUFFER_SIZE);
    size_t ns;
    ssize_t nread, nwrote;

    if (raw_entry == NULL || buffer == NULL) {
      _GD_SetError(D, GD_E_ALLOC, 0, NULL, 0, NULL);
      dreturnvoid();
      return;
    }

    offset -= D->fragment[fragment].frame_offset;

    /* Because it may fail, the move must occur out-of-place and then be copied
     * back over the affected files once success is assured */
    for (i = 0; i < D->n_entries; ++i)
      if (D->entry[i]->fragment_index == fragment &&
          D->entry[i]->field_type == GD_RAW_ENTRY)
      {
        /* check data protection */
        if (D->fragment[fragment].protection & GD_PROTECT_DATA) {
          _GD_SetError(D, GD_E_PROTECTED, GD_E_PROTECTED_FORMAT, NULL, 0,
              D->fragment[fragment].cname);
          break;
        }

        if (!_GD_Supports(D, D->entry[i], GD_EF_OPEN | GD_EF_CLOSE |
              GD_EF_READ | GD_EF_WRITE | GD_EF_SYNC | GD_EF_UNLINK |
              GD_EF_TEMP))
          break;

        enc = encode + raw_entry[i]->e->file[0].encoding;
        ns = BUFFER_SIZE / raw_entry[i]->e->size;

        /* add this raw field to the list */
        raw_entry[n_raw++] = D->entry[i];

        /* Create a temporary file and open it */
        if ((*enc->temp)(raw_entry[i]->e->file, GD_TEMP_OPEN)) {
          _GD_SetError(D, GD_E_RAW_IO, 0, raw_entry[i]->e->file[1].name, errno,
              NULL);
          break;
        }

        /* Open the input file */
        if ((*enc->open)(raw_entry[i]->e->file, raw_entry[i]->e->filebase,
              0, 0))
        {
          _GD_SetError(D, GD_E_RAW_IO, 0, raw_entry[i]->e->file[0].name, errno,
              NULL);
          break;
        }

        /* Adjust for the change in offset */
        if (offset < 0) { /* new offset is less, pad new file */
          if ((*enc->seek)(raw_entry[i]->e->file + 1,
                -offset * raw_entry[i]->spf, raw_entry[i]->data_type, 1)) {
            _GD_SetError(D, GD_E_RAW_IO, 0, raw_entry[i]->e->file[1].name,
                errno, NULL);
            break;
          }
        } else { /* new offset is more, truncate new file */
          if ((*enc->seek)(raw_entry[i]->e->file, offset * raw_entry[i]->spf,
              raw_entry[i]->data_type, 0))
            _GD_SetError(D, GD_E_RAW_IO, 0, raw_entry[i]->e->file[1].name,
                errno, NULL);
            break;
        }

        /* Now copy the old file to the new file */
        for (;;) {
          nread = (*enc->read)(raw_entry[i]->e->file, buffer,
              raw_entry[i]->data_type, ns);

          if (nread < 0) {
            _GD_SetError(D, GD_E_RAW_IO, 0, raw_entry[i]->e->file[1].name,
                errno, NULL);
            break;
          }

          if (nread == 0)
            break;

          nwrote = (*enc->write)(raw_entry[i]->e->file, buffer,
              raw_entry[i]->data_type, nread);

          if (nwrote < nread) {
            _GD_SetError(D, GD_E_RAW_IO, 0, raw_entry[i]->e->file[1].name,
                errno, NULL);
            break;
          }
        }

        /* Well, I suppose the copy worked.  Close both files */
        if ((*enc->close)(raw_entry[i]->e->file) ||
            (*enc->sync)(raw_entry[i]->e->file + 1) ||
            (*enc->close)(raw_entry[i]->e->file + 1))
        {
            _GD_SetError(D, GD_E_RAW_IO, 0, raw_entry[i]->e->file[1].name,
                errno, NULL);
            break;
        }
      }

    /* If successful, move the temporary file over the old file, otherwise
     * remove the temporary files */
    for (i = 0; i < n_raw; ++i)
      if ((*encode[raw_entry[i]->e->file[0].encoding].temp)
          (raw_entry[i]->e->file, (D->error) ? GD_TEMP_DESTROY : GD_TEMP_MOVE))
        _GD_SetError(D, GD_E_RAW_IO, 0, raw_entry[i]->e->file[0].name,
            errno, NULL);

    free(raw_entry);
    free(buffer);

    if (D->error) {
      dreturnvoid();
      return;
    }
  }

  D->fragment[fragment].frame_offset = offset;
  D->fragment[fragment].modified = 1;

  dreturnvoid();
}

int put_frameoffset64(DIRFILE* D, off64_t offset, int fragment, int move)
{
  int i;

  dtrace("%p, %lli, %i, %i\n", D, (long long)offset, fragment, move);

  if (D->flags & GD_INVALID) {/* don't crash */
    _GD_SetError(D, GD_E_BAD_DIRFILE, 0, NULL, 0, NULL);
    dreturn("%i", -1);
    return -1;
  }

  if ((D->flags & GD_ACCMODE) != GD_RDWR) {
    _GD_SetError(D, GD_E_ACCMODE, 0, NULL, 0, NULL);
    dreturn("%zi", -1);
    return -1;
  }

  if (fragment < GD_ALL_FRAGMENTS || fragment >= D->n_fragment) {
    _GD_SetError(D, GD_E_BAD_INDEX, 0, NULL, 0, NULL);
    dreturn("%i", -1);
    return -1;
  }

  if (offset < 0) {
    _GD_SetError(D, GD_E_RANGE, 0, NULL, 0, NULL);
    dreturn("%i", -1);
    return -1;
  }

  _GD_ClearError(D);

  if (fragment == GD_ALL_FRAGMENTS) {
    for (i = 0; i < D->n_fragment; ++i) {
      _GD_ShiftFragment(D, offset, i, move);

      if (D->error)
        break;
    }
  } else
    _GD_ShiftFragment(D, offset, fragment, move);

  dreturn("%i", (D->error) ? -1 : 0);
  return (D->error) ? -1 : 0;
}

off64_t get_frameoffset64(DIRFILE* D, int fragment)
{
  dtrace("%p, %i\n", D, fragment);

  if (D->flags & GD_INVALID) {/* don't crash */
    _GD_SetError(D, GD_E_BAD_DIRFILE, 0, NULL, 0, NULL);
    dreturn("%i", -1);
    return -1;
  }

  if (fragment < 0 || fragment >= D->n_fragment) {
    _GD_SetError(D, GD_E_BAD_INDEX, 0, NULL, 0, NULL);
    dreturn("%i", -1);
    return -1;
  }

  _GD_ClearError(D);

  dreturn("%lli", (long long)D->fragment[fragment].frame_offset);
  return D->fragment[fragment].frame_offset;
}

/* 32(ish)-bit wrappers for the 64-bit versions, when needed */
int put_frameoffset(DIRFILE* D, off_t offset, int fragment, int move)
{
  return put_frameoffset64(D, offset, fragment, move);
}

off_t get_frameoffset(DIRFILE* D, int fragment)
{
  return get_frameoffset64(D, fragment);
}