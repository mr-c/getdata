/* Copyright (C) 2008-2014 D. V. Wiebe
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
#include "internal.h"

void _GD_FreeE(DIRFILE *restrict D, gd_entry_t *restrict entry, int priv)
{
  int i;
  size_t n;

  dtrace("%p, %p, %i", D, entry, priv);

  if (!entry || entry->field_type == GD_NO_ENTRY) {
    dreturnvoid();
    return;
  }

  free(entry->field);

  switch(entry->field_type) {
    case GD_LINCOM_ENTRY:
      for (i = 0; i < entry->EN(lincom,n_fields); ++i) {
        free(entry->in_fields[i]);
        free(entry->scalar[i]);
        free(entry->scalar[i + GD_MAX_LINCOM]);
      }
      break;
    case GD_LINTERP_ENTRY:
      free(entry->in_fields[0]);
      free(entry->EN(linterp,table));
      if (priv) {
        if (entry->e->u.linterp.table_dirfd > 0)
          _GD_ReleaseDir(D, entry->e->u.linterp.table_dirfd);
        free(entry->e->u.linterp.table_file);
        free(entry->e->u.linterp.lut);
      } else
        entry->EN(linterp,table) = NULL;
      break;
    case GD_DIVIDE_ENTRY:
    case GD_MULTIPLY_ENTRY:
      free(entry->in_fields[1]);
      free(entry->in_fields[0]);
      break;
    case GD_BIT_ENTRY:
    case GD_SBIT_ENTRY:
      free(entry->scalar[0]);
      free(entry->scalar[1]);
      free(entry->in_fields[0]);
      break;
    case GD_PHASE_ENTRY:
    case GD_RECIP_ENTRY:
      free(entry->scalar[0]);
      free(entry->in_fields[0]);
      break;
    case GD_POLYNOM_ENTRY:
      free(entry->in_fields[0]);
      for (i = 0; i <= entry->EN(polynom,poly_ord); ++i)
        free(entry->scalar[i]);
      break;
    case GD_STRING_ENTRY:
      if (priv)
        free(entry->e->u.string);
      break;
    case GD_SARRAY_ENTRY:
      if (priv) {
        for (n = 0; n < entry->EN(scalar,array_len); ++n)
          free(((char **)entry->e->u.scalar.d)[n]);
        free(entry->e->u.scalar.d);
      }
      break;
    case GD_CONST_ENTRY:
    case GD_CARRAY_ENTRY:
      if (priv) {
        free(entry->e->u.scalar.client);
        free(entry->e->u.scalar.d);
      }
      break;
    case GD_RAW_ENTRY:
      free(entry->scalar[0]);
      if (priv) {
        free(entry->e->u.raw.filebase);
        free(entry->e->u.raw.file[0].name);
        free(entry->e->u.raw.file[1].name);
      }
      break;
    case GD_WINDOW_ENTRY:
      free(entry->scalar[0]);
      free(entry->in_fields[0]);
      free(entry->in_fields[1]);
      break;
    case GD_MPLEX_ENTRY:
      free(entry->scalar[0]);
      free(entry->scalar[1]);
      free(entry->in_fields[0]);
      free(entry->in_fields[1]);
      break;
    case GD_ALIAS_ENTRY:
      free(entry->in_fields[0]);
      break;
    case GD_INDEX_ENTRY:
    case GD_NO_ENTRY:
      break;
  }

  if (priv) {
    free(entry->e->alias_list);
    for (i = 0; i < GD_N_ENTRY_LISTS; ++i)
      free(entry->e->entry_list[i]);
    free(entry->e->string_value_list);
    free(entry->e->const_value_list);
    if (entry->e->carray_value_list)
      for (i = 0; entry->e->carray_value_list[i].n != 0; ++i)
        free(entry->e->carray_value_list[i].d);
    free(entry->e->carray_value_list);
    if (entry->e->n_meta > -1)
      free(entry->e->p.meta_entry);
    free(entry->e);
    free(entry);
  } else {
    entry->field = NULL;
    memset(entry->in_fields, 0, sizeof(char*) * GD_MAX_LINCOM);
    memset(entry->scalar, 0, sizeof(char*) * GD_MAX_LINCOM * 2);
  }

  dreturnvoid();
  return;
}

gd_entry_t* gd_free_entry_strings(gd_entry_t* entry) gd_nothrow
{
  dtrace("%p", entry);

  _GD_FreeE(NULL, entry, 0);

  dreturn("%p", entry);
  return entry;
}

static int _GD_GetScalar(DIRFILE *restrict D, gd_entry_t *restrict E, int i,
    gd_type_t type, void *restrict data, int err)
{
  void *ptr = NULL;
  gd_entry_t* C = NULL;
  int repr, e = 0;
  char* field_code;
  const char* scalar = E->scalar[i];
  int index = E->scalar_ind[i];

  dtrace("%p, %p, %i, %i, %p, %i", D, E, i, type, data, err);

  if (scalar != NULL) {
    C = _GD_FindFieldAndRepr(D, scalar, &field_code, &repr, NULL, 0, 1);

    if (D->error) {
      dreturn("%i", 1);
      return 1;
    }

    if (C == NULL) {
      if (err)
        _GD_SetError(D, GD_E_BAD_SCALAR, GD_E_SCALAR_CODE, E->field, 0,
            field_code);
      e = 1;
    } else if (C->field_type != GD_CONST_ENTRY &&
        C->field_type != GD_CARRAY_ENTRY)
    {
      if (err)
        _GD_SetError(D, GD_E_BAD_SCALAR, GD_E_SCALAR_TYPE, E->field, 0,
            field_code);
      e = 1;
    } else {
      if (C->field_type == GD_CONST_ENTRY) {
        index = 0;
        E->scalar_ind[i] = -1;
      } else if (index < 0) {
        /* a CARRAY masquerading as a CONST; default to the first element */
        index = E->scalar_ind[i] = 0;
      }

      if ((D->flags & GD_ACCMODE) == GD_RDWR)
        ptr = _GD_Realloc(D, C->e->u.scalar.client,
            (C->e->u.scalar.n_client + 1) * sizeof(gd_entry_t*));

      _GD_DoField(D, C, repr, index, 1, type, data);

      if (ptr) {
        C->e->u.scalar.client = (gd_entry_t **)ptr;
        C->e->u.scalar.client[C->e->u.scalar.n_client++] = E;
      }
    }

    if (field_code != scalar)
      free(field_code);
  }

  dreturn("%i", e);
  return e;
}

/* resolve non-literal scalars */
int _GD_CalculateEntry(DIRFILE *restrict D, gd_entry_t *restrict E, int err)
{
  int i, e = 0, cs = 0;

  dtrace("%p, %p, %i", D, E, err);

  switch(E->field_type) {
    case GD_RAW_ENTRY:
      e = _GD_GetScalar(D, E, 0, GD_UINT_TYPE, &E->EN(raw,spf), err);
      break;
    case GD_POLYNOM_ENTRY:
      for (i = 0; i <= E->EN(polynom,poly_ord); ++i) {
        if (_GD_GetScalar(D, E, i, GD_COMPLEX128, &E->EN(polynom,ca)[i], err))
          e = 1;
        else {
          E->EN(polynom,a)[i] = creal(E->EN(polynom,ca)[i]);

          if (!cs && cimag(E->EN(polynom,ca)[i]))
            cs = 1;
        }

        if (D->error)
          break;
      }
      break;
    case GD_LINCOM_ENTRY:
      for (i = 0; i < E->EN(lincom,n_fields); ++i) {
        if (_GD_GetScalar(D, E, i, GD_COMPLEX128, &E->EN(lincom,cm)[i], err))
          e = 1;
        else {
          E->EN(lincom,m)[i] = creal(E->EN(lincom,cm)[i]);

          if (!cs && cimag(E->EN(lincom,cm)[i]))
            cs = 1;
        }

        if (_GD_GetScalar(D, E, i + GD_MAX_LINCOM, GD_COMPLEX128,
            &E->EN(lincom,cb)[i], err))
        {
          e = 1;
        } else {
          E->EN(lincom,b)[i] = creal(E->EN(lincom,cb)[i]);

          if (!cs && cimag(E->EN(lincom,cb)[i]))
            cs = 1;
        }

        if (D->error)
          break;
      }
      break;
    case GD_RECIP_ENTRY:
      if (_GD_GetScalar(D, E, 0, GD_COMPLEX128, &E->EN(recip,cdividend), err))
        e = 1;
      else {
        E->EN(recip,dividend) = creal(E->EN(recip,cdividend));
        if (cimag(E->EN(recip,cdividend)))
          cs = 1;
      }
      break;
    case GD_BIT_ENTRY:
    case GD_SBIT_ENTRY:
      e = _GD_GetScalar(D, E, 0, GD_INT_TYPE, &E->EN(bit,bitnum), err);
      e |= _GD_GetScalar(D, E, 1, GD_INT_TYPE, &E->EN(bit,numbits), err);
      break;
    case GD_PHASE_ENTRY:
      e = _GD_GetScalar(D, E, 0, GD_INT64, &E->EN(phase,shift), err);
      break;
    case GD_WINDOW_ENTRY:
      switch (E->EN(window,windop)) {
        case GD_WINDOP_EQ:
        case GD_WINDOP_NE:
          e = _GD_GetScalar(D, E, 0, GD_INT64, &E->EN(window,threshold.i), err);
          break;
        case GD_WINDOP_SET:
        case GD_WINDOP_CLR:
          e = _GD_GetScalar(D, E, 0, GD_UINT64, &E->EN(window,threshold.u),
              err);
          break;
        default:
          e = _GD_GetScalar(D, E, 0, GD_FLOAT64, &E->EN(window,threshold.r),
              err);
          break;
      }
      break;
    case GD_MPLEX_ENTRY:
      e = _GD_GetScalar(D, E, 0, GD_INT_TYPE, &E->EN(mplex,count_val), err);
      e |= _GD_GetScalar(D, E, 1, GD_INT_TYPE, &E->EN(mplex,period), err);
      break;
    case GD_NO_ENTRY:
    case GD_LINTERP_ENTRY:
    case GD_MULTIPLY_ENTRY:
    case GD_DIVIDE_ENTRY:
    case GD_STRING_ENTRY:
    case GD_CONST_ENTRY:
    case GD_CARRAY_ENTRY:
    case GD_SARRAY_ENTRY:
    case GD_INDEX_ENTRY:
    case GD_ALIAS_ENTRY:
      break;
  }

  if (!e)
    E->flags |= GD_EN_CALC;

  if (cs)
    E->flags |= GD_EN_COMPSCAL;
  else
    E->flags &= ~GD_EN_COMPSCAL;

  dreturn("%i", !e);
  return !e;
}

char* gd_raw_filename(DIRFILE* D, const char* field_code_in) gd_nothrow
{
  int repr;
  char *field_code, *filename;
  gd_entry_t *E;

  dtrace("%p, \"%s\"", D, field_code_in);

  _GD_ClearError(D);

  if (D->flags & GD_INVALID) {/* don't crash */
    _GD_SetError(D, GD_E_BAD_DIRFILE, 0, NULL, 0, NULL);
    dreturn("%p", NULL);
    return NULL;
  }

  /* Check field */
  E = _GD_FindFieldAndRepr(D, field_code_in, &field_code, &repr, NULL, 1, 1);

  if (D->error) {
    dreturn("%p", NULL);
    return NULL;
  }

  if (E->field_type != GD_RAW_ENTRY) {
    _GD_SetError(D, GD_E_BAD_FIELD_TYPE, GD_E_FIELD_BAD, NULL, 0, field_code);
    dreturn("%p", NULL);
    return NULL;
  }

  if (field_code != field_code_in)
    free(field_code);

  if (E->e->u.raw.file[0].name == NULL) {
    /* ensure encoding sybtype is known */
    if (!_GD_Supports(D, E, GD_EF_NAME)) {
      dreturn("%p", NULL);
      return NULL;
    }

    if (E->e->u.raw.file[0].subenc == GD_ENC_UNKNOWN) {
      _GD_SetError(D, GD_E_UNKNOWN_ENCODING, GD_E_UNENC_UNDET, NULL, 0, NULL);
      dreturn("%p", NULL);
      return NULL;
    } else if ((*gd_ef_[E->e->u.raw.file[0].subenc].name)(D,
          (const char*)D->fragment[E->fragment_index].enc_data,
          E->e->u.raw.file, E->e->u.raw.filebase, 0, 0))
    {
      dreturn("%p", NULL);
      return NULL;
    }
  }

  filename = _GD_MakeFullPath(D, D->fragment[E->fragment_index].dirfd,
      E->e->u.raw.file->name, 1);

  dreturn("%p", filename);
  return filename;
}

int gd_entry(DIRFILE* D, const char* field_code_in, gd_entry_t* entry)
  gd_nothrow
{
  int i, repr;
  gd_entry_t *E;
  char* field_code;

  dtrace("%p, \"%s\", %p", D, field_code_in, entry);

  if (D->flags & GD_INVALID) {
    _GD_SetError(D, GD_E_BAD_DIRFILE, 0, NULL, 0, NULL);
    dreturn("%i", 1);
    return -1;
  }

  _GD_ClearError(D);

  /* get rid of the represenation, if any */
  E = _GD_FindFieldAndRepr(D, field_code_in, &field_code, &repr, NULL, 1, 1);

  if (D->error) {
    dreturn("%i", -1);
    return -1;
  }

  if (field_code != field_code_in)
    free(field_code);

  /* Calculate the entry, if necessary */
  if (!(E->flags & GD_EN_CALC))
    _GD_CalculateEntry(D, E, 0);

  /* now copy to the user supplied buffer */
  memcpy(entry, E, sizeof(gd_entry_t));
  entry->e = NULL;

  /* duplicate strings */
  entry->field = strdup(E->field);

  switch(E->field_type) {
    case GD_LINCOM_ENTRY:
      for (i = 0; i < E->EN(lincom,n_fields); ++i) {
        entry->in_fields[i] = strdup(E->in_fields[i]);
        if (E->scalar[i])
          entry->scalar[i] = strdup(E->scalar[i]);
        if (E->scalar[i + GD_MAX_LINCOM])
          entry->scalar[i + GD_MAX_LINCOM] = strdup(E->scalar[i +
              GD_MAX_LINCOM]);
      }
      break;
    case GD_LINTERP_ENTRY:
      entry->in_fields[0] = strdup(E->in_fields[0]);
      entry->EN(linterp,table) = strdup(E->EN(linterp,table));
      break;
    case GD_POLYNOM_ENTRY:
      entry->in_fields[0] = strdup(E->in_fields[0]);
      for (i = 0; i <= E->EN(polynom,poly_ord); ++i)
        if (E->scalar[i])
          entry->scalar[i] = strdup(E->scalar[i]);
      break;
    case GD_MULTIPLY_ENTRY:
    case GD_DIVIDE_ENTRY:
      entry->in_fields[0] = strdup(E->in_fields[0]);
      entry->in_fields[1] = strdup(E->in_fields[1]);
      break;
    case GD_RECIP_ENTRY:
    case GD_PHASE_ENTRY:
      entry->in_fields[0] = strdup(E->in_fields[0]);
      if (E->scalar[0])
        entry->scalar[0] = strdup(E->scalar[0]);
      break;
    case GD_BIT_ENTRY:
    case GD_SBIT_ENTRY:
      entry->in_fields[0] = strdup(E->in_fields[0]);
      if (E->scalar[0])
        entry->scalar[0] = strdup(E->scalar[0]);
      if (E->scalar[1])
        entry->scalar[1] = strdup(E->scalar[1]);
      break;
    case GD_RAW_ENTRY:
      if (E->scalar[0])
        entry->scalar[0] = strdup(E->scalar[0]);
      break;
    case GD_WINDOW_ENTRY:
      entry->in_fields[0] = strdup(E->in_fields[0]);
      entry->in_fields[1] = strdup(E->in_fields[1]);
      if (E->scalar[0])
        entry->scalar[0] = strdup(E->scalar[0]);
      break;
    case GD_MPLEX_ENTRY:
      entry->in_fields[0] = strdup(E->in_fields[0]);
      entry->in_fields[1] = strdup(E->in_fields[1]);
      if (E->scalar[0])
        entry->scalar[0] = strdup(E->scalar[0]);
      if (E->scalar[1])
        entry->scalar[1] = strdup(E->scalar[1]);
      break;
    case GD_INDEX_ENTRY:
    case GD_CONST_ENTRY:
    case GD_CARRAY_ENTRY:
    case GD_SARRAY_ENTRY:
    case GD_STRING_ENTRY:
    case GD_NO_ENTRY:
    case GD_ALIAS_ENTRY:
      break;
  }

  dreturn("%i", 0);
  return 0;
}

const char *gd_alias_target(DIRFILE *D, const char *field_code) gd_nothrow
{
  gd_entry_t *E;

  dtrace("%p, \"%s\"", D, field_code);

  if (D->flags & GD_INVALID) {
    _GD_SetError(D, GD_E_BAD_DIRFILE, 0, NULL, 0, NULL);
    dreturn("%p", NULL);
    return NULL;
  }

  E = _GD_FindField(D, field_code, D->entry, D->n_entries, 0, NULL);

  if (E == NULL) {
    _GD_SetError(D, GD_E_BAD_CODE, GD_E_CODE_MISSING, NULL, 0, field_code);
    dreturn("%p", NULL);
    return NULL;
  }

  if (E->field_type != GD_ALIAS_ENTRY) {
    _GD_SetError(D, GD_E_BAD_FIELD_TYPE, GD_E_FIELD_BAD, NULL, 0, field_code);
    dreturn("%p", NULL);
    return NULL;
  }

  dreturn("\"%s\"", E->in_fields[0]);
  return E->in_fields[0];
}

const char **gd_aliases(DIRFILE *D, const char *field_code) gd_nothrow
{
  gd_entry_t *E;
  int j = 1;
  unsigned u, n;

  dtrace("%p, \"%s\"", D, field_code);

  if (D->flags & GD_INVALID) {
    _GD_SetError(D, GD_E_BAD_DIRFILE, 0, NULL, 0, NULL);
    dreturn("%p", NULL);
    return NULL;
  }

  n = gd_naliases(D, field_code) + 1;

  if (D->error) {
    dreturn("%p", NULL);
    return NULL;
  }

  E = _GD_FindField(D, field_code, D->entry, D->n_entries, 1, NULL);

  if (E == NULL) {
    _GD_SetError(D, GD_E_BAD_CODE, GD_E_CODE_MISSING, NULL, 0, field_code);
    dreturn("%p", NULL);
    return NULL;
  }

  E->e->alias_list = (const char**)_GD_Realloc(D, E->e->alias_list,
      sizeof(const char *) * n);

  if (D->error) {
    dreturn("%p", NULL);
    return NULL;
  }

  E->e->alias_list[0] = E->field;

  for (u = 0; u < D->n_entries; ++u)
    if (D->entry[u]->field_type == GD_ALIAS_ENTRY &&
        D->entry[u]->e->entry[0] == E)
    {
      E->e->alias_list[j++] = D->entry[u]->field;
    }

  /* terminate */
  E->e->alias_list[j] = NULL;

  dreturn("%p", E->e->alias_list);
  return E->e->alias_list;
}

unsigned int gd_naliases(DIRFILE *D, const char *field_code) gd_nothrow
{
  gd_entry_t *E;
  unsigned u, n = 1;

  dtrace("%p, \"%s\"", D, field_code);

  if (D->flags & GD_INVALID) {
    _GD_SetError(D, GD_E_BAD_DIRFILE, 0, NULL, 0, NULL);
    dreturn("%u", 0);
    return 0;
  }

  E = _GD_FindField(D, field_code, D->entry, D->n_entries, 1, NULL);

  if (E == NULL) {
    _GD_SetError(D, GD_E_BAD_CODE, GD_E_CODE_MISSING, NULL, 0, field_code);
    dreturn("%u", 0);
    return 0;
  }

  for (u = 0; u < D->n_entries; ++u)
    if (D->entry[u]->field_type == GD_ALIAS_ENTRY &&
        D->entry[u]->e->entry[0] == E)
    {
      n++;
    }

  dreturn("%u", n);
  return n;
}

gd_entype_t gd_entry_type(DIRFILE* D, const char* field_code_in) gd_nothrow
{
  gd_entry_t* E;
  char* field_code;
  int repr;

  dtrace("%p, \"%s\"", D, field_code_in);

  if (D->flags & GD_INVALID) {
    _GD_SetError(D, GD_E_BAD_DIRFILE, 0, NULL, 0, NULL);
    dreturn("%i", GD_NO_ENTRY);
    return GD_NO_ENTRY;
  }

  _GD_ClearError(D);

  /* get rid of the represenation, if any */
  E = _GD_FindFieldAndRepr(D, field_code_in, &field_code, &repr, NULL, 1, 1);

  if (D->error) {
    dreturn("%i", GD_NO_ENTRY);
    return GD_NO_ENTRY;
  }

  if (field_code != field_code_in)
    free(field_code);

  dreturn("%i", E->field_type);
  return E->field_type;
}

int gd_fragment_index(DIRFILE *D, const char *field_code) gd_nothrow
{
  gd_entry_t *E;

  dtrace("%p, \"%s\"", D, field_code);

  if (D->flags & GD_INVALID) {
    _GD_SetError(D, GD_E_BAD_DIRFILE, 0, NULL, 0, NULL);
    dreturn("%i", -1);
    return -1;
  }

  _GD_ClearError(D);

  E = _GD_FindField(D, field_code, D->entry, D->n_entries, 0, NULL);

  if (E == NULL) {
    _GD_SetError(D, GD_E_BAD_CODE, GD_E_CODE_MISSING, NULL, 0, field_code);
    dreturn("%i", -1);
    return -1;
  }

  dreturn("%i", E->fragment_index);
  return E->fragment_index;
}

int gd_hide(DIRFILE *D, const char *field_code) gd_nothrow
{
  gd_entry_t *E;

  dtrace("%p, \"%s\"", D, field_code);

  if (D->flags & GD_INVALID) {
    _GD_SetError(D, GD_E_BAD_DIRFILE, 0, NULL, 0, NULL);
    dreturn("%i", -1);
    return -1;
  } else if ((D->flags & GD_ACCMODE) != GD_RDWR) {
    _GD_SetError(D, GD_E_ACCMODE, 0, NULL, 0, NULL);
    dreturn("%i", -1);
    return -1;
  }

  _GD_ClearError(D);

  E = _GD_FindField(D, field_code, D->entry, D->n_entries, 0, NULL);

  if (D->error) {
    _GD_SetError(D, GD_E_BAD_CODE, GD_E_CODE_MISSING, NULL, 0, field_code);
    dreturn("%i", -1);
    return -1;
  }

  if (D->fragment[E->fragment_index].protection & GD_PROTECT_FORMAT) {
    _GD_SetError(D, GD_E_PROTECTED, GD_E_PROTECTED_FORMAT, NULL, 0,
        D->fragment[E->fragment_index].cname);
    dreturn("%i", -1);
    return -1;
  }

  if (!(E->flags & GD_EN_HIDDEN)) {
    E->flags |= GD_EN_HIDDEN;
    D->fragment[E->fragment_index].modified = 1;
  }

  dreturn("%i", 0);
  return 0;
}

int gd_hidden(DIRFILE *D, const char *field_code) gd_nothrow
{
  gd_entry_t *E;

  dtrace("%p, \"%s\"", D, field_code);

  if (D->flags & GD_INVALID) {
    _GD_SetError(D, GD_E_BAD_DIRFILE, 0, NULL, 0, NULL);
    dreturn("%i", -1);
    return -1;
  }

  _GD_ClearError(D);

  E = _GD_FindField(D, field_code, D->entry, D->n_entries, 0, NULL);

  if (D->error) {
    _GD_SetError(D, GD_E_BAD_CODE, GD_E_CODE_MISSING, NULL, 0, field_code);
    dreturn("%i", -1);
    return -1;
  }

  dreturn("%i", (E->flags & GD_EN_HIDDEN) ? 1 : 0);
  return (E->flags & GD_EN_HIDDEN) ? 1 : 0;
}

int gd_unhide(DIRFILE *D, const char *field_code) gd_nothrow
{
  gd_entry_t *E;

  dtrace("%p, \"%s\"", D, field_code);

  if (D->flags & GD_INVALID) {
    _GD_SetError(D, GD_E_BAD_DIRFILE, 0, NULL, 0, NULL);
    dreturn("%i", -1);
    return -1;
  } else if ((D->flags & GD_ACCMODE) != GD_RDWR) {
    _GD_SetError(D, GD_E_ACCMODE, 0, NULL, 0, NULL);
    dreturn("%i", -1);
    return -1;
  }

  _GD_ClearError(D);

  E = _GD_FindField(D, field_code, D->entry, D->n_entries, 0, NULL);

  if (D->error) {
    _GD_SetError(D, GD_E_BAD_CODE, GD_E_CODE_MISSING, NULL, 0, field_code);
    dreturn("%i", -1);
    return -1;
  }

  if (D->fragment[E->fragment_index].protection & GD_PROTECT_FORMAT) {
    _GD_SetError(D, GD_E_PROTECTED, GD_E_PROTECTED_FORMAT, NULL, 0,
        D->fragment[E->fragment_index].cname);
    dreturn("%i", -1);
    return -1;
  }

  if (E->flags & GD_EN_HIDDEN) {
    E->flags &= ~GD_EN_HIDDEN;
    D->fragment[E->fragment_index].modified = 1;
  }

  dreturn("%i", 0);
  return 0;
}

int gd_validate(DIRFILE *D, const char *field_code_in) gd_nothrow
{
  int i, repr;
  gd_entry_t* E;
  char *field_code;

  dtrace("%p, \"%s\"", D, field_code_in);

  if (D->flags & GD_INVALID) {
    _GD_SetError(D, GD_E_BAD_DIRFILE, 0, NULL, 0, NULL);
    dreturn("%i", -1);
    return -1;
  }

  _GD_ClearError(D);

  /* get rid of the representation, if any */
  E = _GD_FindFieldAndRepr(D, field_code_in, &field_code, &repr, NULL, 1, 1);

  if (D->error) {
    dreturn("%i", -1);
    return -1;
  }

  if (field_code != field_code_in)
    free(field_code);

  /* calculate scalars */
  if (!(E->flags & GD_EN_CALC))
    _GD_CalculateEntry(D, E, 1);

  /* check input fields */
  switch (E->field_type) {
    case GD_LINCOM_ENTRY:
      for (i = 0; i < E->EN(lincom,n_fields); ++i)
        _GD_BadInput(D, E, i, 1);
      break;
    case GD_DIVIDE_ENTRY:
    case GD_MULTIPLY_ENTRY:
    case GD_WINDOW_ENTRY:
    case GD_MPLEX_ENTRY:
      _GD_BadInput(D, E, 1, 1);
      /* fallthrough */
    case GD_LINTERP_ENTRY:
    case GD_BIT_ENTRY:
    case GD_PHASE_ENTRY:
    case GD_POLYNOM_ENTRY:
    case GD_SBIT_ENTRY:
    case GD_RECIP_ENTRY:
      _GD_BadInput(D, E, 0, 1);
      /* Fallthrough */
    case GD_RAW_ENTRY:
    case GD_CONST_ENTRY:
    case GD_CARRAY_ENTRY:
    case GD_SARRAY_ENTRY:
    case GD_STRING_ENTRY:
    case GD_INDEX_ENTRY:
    case GD_NO_ENTRY:
    case GD_ALIAS_ENTRY:
      break;
  }

  if (D->error) {
    dreturn("%i", -1);
    return -1;
  }

  dreturn("%i", 0);
  return 0;
}

char *gd_linterp_tablename(DIRFILE *D, const char *field_code_in) gd_nothrow
{
  int repr;
  gd_entry_t *E;
  char *field_code, *table;

  dtrace("%p, \"%s\"", D, field_code_in);

  if (D->flags & GD_INVALID) {
    _GD_SetError(D, GD_E_BAD_DIRFILE, 0, NULL, 0, NULL);
    dreturn("%p", NULL);
    return NULL;
  }

  _GD_ClearError(D);

  E = _GD_FindFieldAndRepr(D, field_code_in, &field_code, &repr, NULL, 1, 1);

  if (D->error) {
    dreturn("%p", NULL);
    return NULL;
  }

  if (field_code != field_code_in)
    free(field_code);

  if (E->field_type != GD_LINTERP_ENTRY) {
    _GD_SetError(D, GD_E_BAD_FIELD_TYPE, GD_E_FIELD_BAD, NULL, 0,
        field_code_in);
    dreturn("%p", NULL);
    return NULL;
  }

  /* initialise */
  if (E->e->u.linterp.table_file == NULL)
    if (_GD_SetTablePath(D, E, E->e)) {
      dreturn("%p", NULL);
      return NULL;
    }

  table = _GD_MakeFullPath(D, E->e->u.linterp.table_dirfd,
      E->e->u.linterp.table_file, 1);

  dreturn("%s", table);
  return table;
}
