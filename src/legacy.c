/* (C) 2002-2005 C. Barth Netterfield
 * (C) 2003-2005 Theodore Kisner
 * (C) 2005-2008 D. V. Wiebe
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
#endif

static struct {
  unsigned int n;
  DIRFILE** D;
} _GD_Dirfiles = {0, NULL};

/* Error-reporting kludge for deprecated API */
static char _GD_GlobalErrorString[MAX_LINE_LENGTH + 6];
static char _GD_GlobalErrorFile[MAX_LINE_LENGTH + 6];
static DIRFILE _GD_GlobalErrors = {
  .error = 0,
  .suberror = 0,
  .error_string = _GD_GlobalErrorString,
  .error_file = _GD_GlobalErrorFile,
  .flags = GD_INVALID
};

/* old error strings */
const char *GD_ERROR_CODES[GD_N_ERROR_CODES] = {
  "Success",
  "Error opening dirfile",
  "Error in Format file",
  "Error truncating dirfile",
  "Error creating dirfile",
  "Bad field code",
  "Unrecognized data type",
  "I/O error accessing field file",
  "Could not open included Format file",
  "Internal error",
  "Memory allocation failed",
  "No RAW fields defined",
  "Could not open interpolation file",
  "Too many levels of recursion",
  "Bad DIRFILE",
  "Cannot write to specified field",
  "Read-only dirfile",
  "Request out-of-range",
  "Operation not supported by current encoding scheme",
  "Unable to discover encoding",
  NULL, /* GD_E_BAD_ENTRY */
  NULL, /* GD_E_DUPLICATE */
  "Scalar field found where vector field expected",
  NULL, /* GD_E_BAD_INDEX */
};

static struct FormatType Format = {
  .rawEntries = NULL,
  .linterpEntries = NULL,
  .multiplyEntries = NULL,
  .mplexEntries = NULL,
  .bitEntries = NULL,
  .phaseEntries = NULL
};

/* _GD_CopyGlobalError: Copy the last error message to the global error buffer.
 */
static int _GD_CopyGlobalError(DIRFILE* D)
{
  _GD_GlobalErrors.suberror = D->suberror;
  _GD_GlobalErrors.error_line = D->error_line;
  strncpy(_GD_GlobalErrors.error_file, D->error_file, FILENAME_MAX);
  strncpy(_GD_GlobalErrors.error_string, D->error_string, FILENAME_MAX);

  return _GD_GlobalErrors.error = D->error;
}

/* legacy wrapper for get_error_string()
 */
char* GetDataErrorString(char* buffer, size_t buflen)
{
  return get_error_string(&_GD_GlobalErrors, buffer, buflen);
}

/* _GD_GetDirfile: Locate the legacy DIRFILE given the filespec.  This started
 * life as GetFormat...
 */
static DIRFILE* _GD_GetDirfile(const char *filename_in, int mode)
{
  unsigned int i_dirfile;

  char filedir[FILENAME_MAX];
  strncpy(filedir, filename_in, FILENAME_MAX);
  if (filedir[strlen(filedir) - 1] == '/')
    filedir[strlen(filedir) - 1] = '\0';

  /* first check to see if we have already read it */
  for (i_dirfile = 0; i_dirfile < _GD_Dirfiles.n; i_dirfile++) {
    if (strncmp(filedir, _GD_Dirfiles.D[i_dirfile]->name, FILENAME_MAX) == 0) {
      /* if the dirfile was previously opened read-only, close it so we can
       * re-open it read-write */
      if ((mode & GD_RDWR) && (_GD_Dirfiles.D[i_dirfile]->flags & GD_ACCMODE) ==
          GD_RDONLY) {
        /* close it */
        dirfile_close(_GD_Dirfiles.D[i_dirfile]);

        /* copy the last dirfile in the list over top of this one and decrement
         * the counter -- next realloc will do nothing */
        _GD_Dirfiles.D[i_dirfile] = _GD_Dirfiles.D[--_GD_Dirfiles.n];
      } else {
        _GD_ClearError(_GD_Dirfiles.D[i_dirfile]);
        return _GD_Dirfiles.D[i_dirfile];
      }
    }
  }

  /* if we get here, the file has not yet been read */
  /* Allocate the memory, then fill.  If we have an error, */
  /*  we will have to free the memory... */
  _GD_Dirfiles.n++;
  _GD_Dirfiles.D = realloc(_GD_Dirfiles.D, _GD_Dirfiles.n * sizeof(DIRFILE*));

  /* Open a dirfile */
  _GD_Dirfiles.D[_GD_Dirfiles.n - 1] = dirfile_open(filedir, mode);

  /* Error encountered -- the dirfile will shortly be deleted */
  if (_GD_Dirfiles.D[_GD_Dirfiles.n - 1]->error != GD_E_OK)
    return _GD_Dirfiles.D[--_GD_Dirfiles.n];

  return _GD_Dirfiles.D[_GD_Dirfiles.n - 1];
}

static void CopyRawEntry(struct RawEntryType* R, gd_entry_t* E)
{
  if (E == NULL)
    return;

  R->field = E->field;
  
  switch(E->data_type) {
    case GD_UINT8:
      R->type = 'c';
      break;
    case GD_UINT16:
      R->type = 'u';
      break;
    case GD_INT16:
      R->type = 's';
      break;
    case GD_UINT32:
      R->type = 'U';
      break;
    case GD_INT32:
      R->type = 'S';
      break;
    case GD_FLOAT32:
      R->type = 'f';
      break;
    case GD_FLOAT64:
      R->type = 'd';
      break;
    default: /* Well, this isn't right, but it's the best we can do. */
      R->type = 'n';
      break;
  }

  R->size = (int)E->size;
  R->samples_per_frame = (int)E->spf;
}

static void CopyLincomEntry(struct LincomEntryType* L, gd_entry_t* E)
{
  int i;

  if (E == NULL)
    return;

  L->field = E->field;
  L->n_fields = E->n_fields;
  for (i = 0; i < E->n_fields; ++i) {
    L->in_fields[i] = E->in_fields[i];
    L->m[i] = E->m[i];
    L->b[i] = E->b[i];
  }
}

static void CopyLinterpEntry(struct LinterpEntryType* L, gd_entry_t* E)
{
  if (E == NULL)
    return;

  L->field = E->field;
  L->raw_field = E->in_fields[0];
  L->linterp_file = E->table;
}

static void CopyBitEntry(struct BitEntryType* B, gd_entry_t* E)
{
  if (E == NULL)
    return;

  B->field = E->field;
  B->raw_field = E->in_fields[0];
  B->bitnum = E->bitnum;
  B->numbits = E->numbits;
}

static void CopyMultiplyEntry(struct MultiplyEntryType* M, gd_entry_t* E)
{
  if (E == NULL)
    return;

  M->field = E->field;
  M->in_fields[0] = E->in_fields[0];
  M->in_fields[1] = E->in_fields[1];
}

static void CopyPhaseEntry(struct PhaseEntryType* P, gd_entry_t* E)
{
  if (E == NULL)
    return;

  P->field = E->field;
  P->raw_field = E->in_fields[0];
  P->shift = E->shift;
}

/* Okay, reconstruct the old FormatType.  This is painful. */
const struct FormatType *GetFormat(const char *filedir, int *error_code) {
  DIRFILE *D = _GD_GetDirfile(filedir, GD_RDONLY);
  unsigned int i;

  int nraw = 0;
  int nlincom = 0;
  int nlinterp = 0;
  int nmultiply = 0;
  int nbit = 0;
  int nphase = 0;

  if (D->error) {
    *error_code = _GD_CopyGlobalError(D);
    return NULL;
  }
  
  /* fill the structure -- like everything about the legacy API, this is
   * not thread-safe */
  Format.FileDirName = filedir; 
  Format.frame_offset = (int)D->frame_offset;
  CopyRawEntry(&Format.first_field, D->first_field);

  /* Pass one: run through the entry list and count the number of different
   * types */
  Format.n_raw = 0; 
  Format.n_lincom = 0; 
  Format.n_linterp = 0; 
  Format.n_multiply = 0; 
  Format.n_mplex = 0; /* Erm... yeah... */
  Format.n_bit = 0; 
  Format.n_phase = 0; 
  for (i = 0; i < D->n_entries; ++i) 
    switch(D->entry[i]->field_type) {
      case GD_RAW_ENTRY:
        Format.n_raw++;
        break;
      case GD_LINCOM_ENTRY:
        Format.n_lincom++;
        break;
      case GD_LINTERP_ENTRY:
        Format.n_linterp++;
        break;
      case GD_BIT_ENTRY:
        Format.n_bit++;
        break;
      case GD_MULTIPLY_ENTRY:
        Format.n_multiply++;
        break;
      case GD_PHASE_ENTRY:
        Format.n_phase++;
        break;
      default:
        break;
    }

  /* Now reallocate the Entry arrays */
  Format.rawEntries = realloc(Format.rawEntries,
      Format.n_raw * sizeof(struct RawEntryType));
  Format.lincomEntries = realloc(Format.lincomEntries,
      Format.n_lincom * sizeof(struct LincomEntryType));
  Format.linterpEntries = realloc(Format.linterpEntries,
      Format.n_linterp * sizeof(struct LinterpEntryType));
  Format.multiplyEntries = realloc(Format.multiplyEntries,
      Format.n_multiply * sizeof(struct MultiplyEntryType));
  Format.bitEntries = realloc(Format.bitEntries,
      Format.n_bit * sizeof(struct BitEntryType));
  Format.phaseEntries = realloc(Format.phaseEntries,
      Format.n_phase * sizeof(struct PhaseEntryType));

  /* Pass 2: Fill the Entry structs */
  for (i = 0; i < D->n_entries; ++i)
    switch(D->entry[i]->field_type) {
      case GD_RAW_ENTRY:
        CopyRawEntry(&Format.rawEntries[nraw++], D->entry[i]);
        break;
      case GD_LINCOM_ENTRY:
        CopyLincomEntry(&Format.lincomEntries[nlincom++], D->entry[i]);
        break;
      case GD_LINTERP_ENTRY:
        CopyLinterpEntry(&Format.linterpEntries[nlinterp++], D->entry[i]);
        break;
      case GD_BIT_ENTRY:
        CopyBitEntry(&Format.bitEntries[nbit++], D->entry[i]);
        break;
      case GD_MULTIPLY_ENTRY:
        CopyMultiplyEntry(&Format.multiplyEntries[nmultiply++], D->entry[i]);
        break;
      case GD_PHASE_ENTRY:
        CopyPhaseEntry(&Format.phaseEntries[nphase++], D->entry[i]);
        break;
      default:
        break;
    }

  return &Format;
}

/* legacy interface to getdata() */
int GetData(const char *filename, const char *field_code,
    int first_frame, int first_samp, int num_frames, int num_samp,
    char return_type, void *data_out, int *error_code)
{
  DIRFILE* D;
  int nread;

  D = _GD_GetDirfile(filename, GD_RDONLY);

  if (D->error) {
    *error_code = _GD_CopyGlobalError(D);
    return 0;
  }

  nread = (int)getdata64(D, field_code, (off64_t)first_frame,
      (off64_t)first_samp, (size_t)num_frames, (size_t)num_samp,
      _GD_LegacyType(return_type), data_out);
  *error_code = _GD_CopyGlobalError(D);

  return nread;
}

/* legacy interface to get_nframes() --- the third argument to this function
 * has been ignored since at least 2005 (and why does it come after
 * error_code?)
 */
int GetNFrames(const char *filename, int *error_code,
    const void *unused __gd_unused)
{
  DIRFILE* D;
  int nf;

  D = _GD_GetDirfile(filename, GD_RDONLY);

  if (D->error) {
    *error_code = _GD_CopyGlobalError(D);
    return 0;
  }

  nf = (int)get_nframes(D);
  *error_code = _GD_CopyGlobalError(D);

  return nf;
}

/* legacy interface to get_spf()
*/
int GetSamplesPerFrame(const char *filename, const char *field_code,
    int *error_code)
{
  DIRFILE* D;

  D = _GD_GetDirfile(filename, GD_RDONLY);

  if (D->error) {
    *error_code = _GD_CopyGlobalError(D);
    return 0;
  }

  int spf = (int)get_spf(D, field_code);
  *error_code = _GD_CopyGlobalError(D);

  return spf;
}

/* legacy interface to putdata()
*/
int PutData(const char *filename, const char *field_code,
    int first_frame, int first_samp, int num_frames, int num_samp,
    char data_type, const void *data_in, int *error_code)
{
  DIRFILE* D;
  int n_write = 0;

  D = _GD_GetDirfile(filename, GD_RDWR | GD_UNENCODED);

  if (D->error) {
    *error_code = _GD_CopyGlobalError(D);
    return 0;
  }

  n_write = (int)putdata64(D, field_code, (off64_t)first_frame,
      (off64_t)first_samp, (size_t)num_frames, (size_t)num_samp,
      _GD_LegacyType(data_type), data_in);
  *error_code = _GD_CopyGlobalError(D);

  return n_write;
}
/* vim: ts=2 sw=2 et
*/
