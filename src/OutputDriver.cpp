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

#include <stdarg.h>
#include <stdio.h>
#include "c2ffi.h"

/*** Add new OutputDrivers here: ***************************************/

namespace c2ffi {
    OutputDriver* MakeSexpOutputDriver(std::ostream *os);
    OutputDriver* MakeJSONOutputDriver(std::ostream *os);

    OutputDriverField OutputDrivers[] = {
        { "json", &MakeJSONOutputDriver },
        { "sexp", &MakeSexpOutputDriver },
        { 0, 0 }
    };
}

/***********************************************************************/

namespace c2ffi {
    void OutputDriver::comment(char *fmt, ...) {
        va_list ap;
        char buf[1024];
        va_start(ap, fmt);

        vsnprintf(buf, sizeof(buf), fmt, ap);
        write_comment(buf);

        va_end(ap);
    }
}
