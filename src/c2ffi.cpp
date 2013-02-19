/*
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

#include <iostream>

#include <llvm/Support/Host.h>
#include <llvm/ADT/IntrusiveRefCntPtr.h>

#include <clang/Basic/DiagnosticOptions.h>
#include <clang/Frontend/TextDiagnosticPrinter.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Basic/TargetOptions.h>
#include <clang/Basic/TargetInfo.h>
#include <clang/Basic/FileManager.h>
#include <clang/Basic/SourceManager.h>
#include <clang/Lex/HeaderSearch.h>
#include <clang/Lex/Preprocessor.h>
#include <clang/Basic/Diagnostic.h>
#include <clang/AST/ASTContext.h>
#include <clang/AST/ASTConsumer.h>
#include <clang/Parse/Parser.h>
#include <clang/Parse/ParseAST.h>

#include "c2ffi.h"
#include "c2ffi/init.h"
#include "c2ffi/opt.h"
#include "c2ffi/ast.h"
#include "c2ffi/macros.h"

using namespace c2ffi;

int main(int argc, char *argv[]) {
    clang::CompilerInstance ci;
    c2ffi::config sys;

    init_ci(ci);
    process_args(sys, argc, argv);

    add_include(ci, "/usr/local/include", true);
    add_include(ci, "/usr/local/lib/clang/3.3/include", true);
    add_include(ci, "/usr/include", true);

    add_includes(ci, sys.includes, false);
    add_includes(ci, sys.sys_includes, true);

    C2FFIASTConsumer *astc = new C2FFIASTConsumer(ci, sys.od);
    ci.setASTConsumer(astc);
    ci.createASTContext();

    const clang::FileEntry *file = ci.getFileManager().getFile(sys.filename);
    ci.getSourceManager().createMainFileID(file);
    ci.getDiagnosticClient().BeginSourceFile(ci.getLangOpts(),
                                             &ci.getPreprocessor());

    sys.od->write_header();

    if(sys.to_namespace != "")
        sys.od->write_namespace(sys.to_namespace);

    clang::ParseAST(ci.getPreprocessor(), astc, ci.getASTContext());
    sys.od->write_footer();

    if(sys.macro_output) {
        process_macros(ci, *sys.macro_output);
        sys.macro_output->close();
    }

    ci.getDiagnosticClient().EndSourceFile();

    return 0;
}
