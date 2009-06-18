// (C) 2008 D. V. Wiebe
//
///////////////////////////////////////////////////////////////////////////
//
// This file is part of the GetData project.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// GetData is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// for more details.
//
// You should have received a copy of the GNU General Public License
// along with GetData; if not, write to the Free Software Foundation,
// Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
//

#ifndef GETDATA_LINTERPENTRY_H
#define GETDATA_LINTERPENTRY_H

#define NO_GETDATA_LEGACY_API

extern "C" {
#include <getdata.h>
}
#include <getdata/entry.h>

namespace GetData {

  class Dirfile;

  class LinterpEntry : public Entry {
    friend class Dirfile;

    public:
      LinterpEntry(const char* field_code, const char* in_field,
          const char* table, int fragment_index = 0);

      virtual const char *Input(int __gd_unused index = 0) {
        return E.in_fields[0];
      };

      virtual const char *Table() {
        return E.table;
      };

      int SetInput(const char* field);
      int SetTable(const char* table, int move_table);

    private:
      LinterpEntry(GetData::Dirfile *dirfile, const char* field_code) :
        Entry(dirfile, field_code) { };
  };
}

#endif