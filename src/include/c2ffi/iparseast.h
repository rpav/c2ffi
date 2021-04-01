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

  /// Parse the entire file specified, notifying the ASTConsumer as
  /// the file is parsed.
  ///
  /// This operation inserts the parsed decls into the translation
  /// unit held by Ctx.
  ///
  /// \param PrintStats Whether to print LLVM statistics related to parsing.
  /// \param TUKind The kind of translation unit being parsed.
  /// \param CompletionConsumer If given, an object to consume code completion
  /// results.
  /// \param SkipFunctionBodies Whether to skip parsing of function bodies.
  /// This option can be used, for example, to speed up searches for
  /// declarations/definitions when indexing.
  void IncrementalParseAST(Preprocessor &pp, ASTConsumer *C,
                ASTContext &Ctx, bool PrintStats = false,
                TranslationUnitKind TUKind = TU_Complete,
                CodeCompleteConsumer *CompletionConsumer = nullptr,
                bool SkipFunctionBodies = false);

  /// Parse the main file known to the preprocessor, producing an
  /// abstract syntax tree.
  void IncrementalParseAST(Sema &S, bool PrintStats = false,
                bool SkipFunctionBodies = false);

}  // end namespace clang

#endif
