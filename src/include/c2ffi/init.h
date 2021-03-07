/*  -*- c++ -*-

    c2ffi
    Copyright (C) 2013  Ryan Pavlik

    This file is part of c2ffi.

    c2ffi is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 2 of the License, or
    (at your option) any later version.

    c2ffi is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with c2ffi.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef C2FFI_INIT_H
#define C2FFI_INIT_H

#include "c2ffi.h"
#include "c2ffi/opt.h"

namespace c2ffi {
    void add_include(clang::CompilerInstance &ci, const char *path,
                     bool isAngled = false, bool show_error = false,
                     bool is_framework = false);
    void add_includes(clang::CompilerInstance &ci,
                      c2ffi::IncludeVector &v, bool is_angled = false,
                      bool show_error = false, bool is_framework = false);

    void init_ci(config &c, clang::CompilerInstance &ci);
}

#endif /* C2FFI_INIT_H */
