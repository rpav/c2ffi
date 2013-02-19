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

#include <vector>
#include <string>
#include <iostream>
#include <fstream>

#include "c2ffi.h"

namespace c2ffi {
    typedef std::vector<std::string> IncludeVector;

    struct config {
        config() : od(0), macro_output(NULL) { }

        IncludeVector includes;
        IncludeVector sys_includes;
        OutputDriver *od;

        std::ofstream *macro_output;

        std::string filename;
    };

    void process_args(config &config, int argc, char *argv[]);
}

#endif /* C2FFI_OPT_H */
