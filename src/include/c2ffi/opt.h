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

#ifndef C2FFI_OPT_H
#define C2FFI_OPT_H

#include <clang/Frontend/FrontendOptions.h>

#include <vector>
#include <string>
#include <iostream>
#include <fstream>

#include "c2ffi.h"

namespace c2ffi {
    typedef std::vector<std::string> IncludeVector;

    struct config {
        config() : od(NULL), macro_output(false),
                   template_output(NULL),
                   std(clang::LangStandard::lang_unspecified),
                   preprocess_only(false),
                   with_macro_defs(false),
                   nostdinc(false),
                   wchar_size(0),
                   error_limit(0)
        { }

        IncludeVector includes;
        IncludeVector sys_includes;
        OutputDriver *od;

        std::ostream  *output;
        bool macro_output;
        std::ofstream *template_output;

        std::string c2ffi_binpath;
        std::string filename;
        std::string to_namespace;

        clang::InputKind kind;
        std::string lang;
        clang::LangStandard::Kind std;
        std::string arch;

        bool preprocess_only;
        bool with_macro_defs;
        bool declspec;
        bool fail_on_error;
        bool warn_as_error;
        bool nostdinc;

        int wchar_size;

        int error_limit;
    };

    void process_args(config &config, int argc, char *argv[]);
}

#endif /* C2FFI_OPT_H */
