/*  -*- c++ -*-

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

    This code mostly originates from clang, with some small modifications.
    https://clang.llvm.org/
*/

#ifndef C2FFI_PARSE_PARSEAST_H
#define C2FFI_PARSE_PARSEAST_H

#include "clang/Basic/LangOptions.h"
#include <clang/Sema/Sema.h>

namespace c2ffi {
  using namespace clang;

  /// Parse the extra file known to the preprocessor, amending an
  /// abstract syntax tree.
  void IncrementalParseAST(Sema &S, FileID &fid, bool PrintStats = false,
                bool SkipFunctionBodies = false);

}  // end namespace clang

#endif
