/* -*- c++ -*-

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

#include "c2ffi.h"

using namespace c2ffi;

namespace c2ffi {
    class NullOutputDriver : public OutputDriver {
    public:
        NullOutputDriver(std::ostream *os)
            : OutputDriver(os) { }

        virtual void write_namespace(const std::string &ns) { }

        virtual void write_comment(const char *str) { }

        using OutputDriver::write;

        // Types -----------------------------------------------------------
        virtual void write(const SimpleType &t) { }
        virtual void write(const TypedefType &t) { }
        virtual void write(const BasicType &t) { }
        virtual void write(const BitfieldType &t) { }
        virtual void write(const PointerType& t) { }
        virtual void write(const ArrayType &t) { }
        virtual void write(const RecordType &t) { }
        virtual void write(const EnumType &t) { }
        virtual void write(const ComplexType& t) { }

        // Decls -----------------------------------------------------------
        virtual void write(const UnhandledDecl &d) { }
        virtual void write(const VarDecl &d) { }
        virtual void write(const FunctionDecl &d) { }
        virtual void write(const TypedefDecl &d) { }
        virtual void write(const RecordDecl &d) { }
        virtual void write(const EnumDecl &d) { }
        virtual void write(const ObjCInterfaceDecl &d) { }
        virtual void write(const ObjCCategoryDecl &d) { }
        virtual void write(const ObjCProtocolDecl &d) { }
    };

    OutputDriver* MakeNullOutputDriver(std::ostream *os) {
        return new NullOutputDriver(os);
    }
}
