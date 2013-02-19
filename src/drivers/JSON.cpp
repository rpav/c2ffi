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

#include <sstream>
#include <stdarg.h>

using namespace c2ffi;

namespace c2ffi {
    class JSONOutputDriver : public OutputDriver {
        int _level;

        void write_object(const char *type, bool open, bool close, ...) {
            va_list ap;
            char *ptr = NULL;

            va_start(ap, close);

            if(open) os() << "{ \"tag\": \"" << type << "\"";
            while((ptr = va_arg(ap, char*))) {
                os() << ", \"" << ptr << "\": ";

                if((ptr = va_arg(ap, char*)))
                    os() << ptr;
                else
                    break;
            }
            if(close) os() << " }";

            va_end(ap);
        }

        template<typename T> std::string str(T v) {
            std::stringstream ss;
            ss << v;
            return ss.str();
        }

        template<typename T> std::string qstr(T v) {
            std::stringstream ss;
            ss << '"' << v << '"';
            return ss.str();
        }

    public:
        JSONOutputDriver(std::ostream *os)
            : OutputDriver(os), _level(0) { }

        using OutputDriver::write;

        virtual void write_header() {
            os() << "[" << std::endl;
        }

        virtual void write_between() {
            os() << "," << std::endl;
        }

        virtual void write_footer() {
            os() << "\n]" << std::endl;
        }

        // Types -----------------------------------------------------------
        virtual void write(const SimpleType &t) {
            write_object(t.name().c_str(), 1, 1, 0);
        }

        virtual void write(const BitfieldType &t) {
            write_object(":bitfield", 1, 1,
                         "width", str(t.width()).c_str(),
                         NULL);
        }

        virtual void write(const PointerType& t) {
            write_object(":pointer", 1, 0,
                         "type", NULL);
            write(t.pointee());
            write_object("", 0, 1, NULL);
        }

        virtual void write(const ArrayType &t) {
            write_object(":array", 1, 0,
                         "type", NULL);
            write(t.pointee());
            write_object("", 0, 1,
                         "size", str(t.size()).c_str(),
                         NULL);
        }

        virtual void write(const RecordType &t) {
            const char *type = t.is_union() ? ":union" : ":struct";

            write_object(type, 1, 1,
                         "name", qstr(t.name()).c_str(),
                         NULL);
        }

        // Decls -----------------------------------------------------------
        virtual void write(const UnhandledDecl &d) {
            // json.org apparently doesn't spec comments. stupid.
        }

        virtual void write(const VarDecl &d) {
            write_object("constant", 1, 0,
                         "name", qstr(d.name()).c_str(),
                         "type", NULL);

            write(d.type());

            if(d.value() != "")
                write_object("", 0, 0,
                             "value", d.value().c_str(),
                             NULL);

            write_object("", 0, 1, NULL);
        }
        virtual void write(const FunctionDecl &d) {
            write_object("function", 1, 0,
                         "name", qstr(d.name()).c_str(),
                         "parameters", NULL);

            os() << "[";
            const NameTypeVector &params = d.fields();
            for(NameTypeVector::const_iterator i = params.begin();
                i != params.end(); i++) {
                if(i != params.begin())
                    os() << ", ";

                write_object("parameter", 1, 0,
                             "name", qstr((*i).first).c_str(),
                             "type", NULL);
                write(*(*i).second);
                write_object("", 0, 1, NULL);
            }

            os() << "]";

            write_object("", 0, 0,
                         "return-type", NULL);
            write(d.return_type());
            write_object("", 0, 1, NULL);
        }

        virtual void write(const TypedefDecl &d) {
            write_object("typedef", 1, 0,
                         "name", qstr(d.name()).c_str(),
                         "type", NULL);

            write(d.type());
            write_object("", 0, 1, NULL);
        }

        virtual void write(const RecordDecl &d) {
            const char *type = d.is_union() ? "union" : "struct";

            write_object(type, 1, 0,
                         "name", qstr(d.name()).c_str(),
                         "fields", NULL);

            os() << "[";
            const NameTypeVector &fields = d.fields();
            for(NameTypeVector::const_iterator i = fields.begin();
                i != fields.end(); i++) {
                if(i != fields.begin())
                    os() << ", ";

                write_object("field", 1, 0,
                             "name", qstr(i->first).c_str(),
                             "type", NULL);
                write(*(i->second));
                write_object("", 0, 1, NULL);
            }

            os() << "]";
            write_object("", 0, 1, NULL);
        }

        virtual void write(const EnumDecl &d) {
            write_object("enum", 1, 0,
                         "name", qstr(d.name()).c_str(),
                         "fields", NULL);

            os() << "[";
            const NameNumVector &fields = d.fields();
            for(NameNumVector::const_iterator i = fields.begin();
                i != fields.end(); i++) {
                if(i != fields.begin())
                    os() << ", ";

                write_object("field", 1, 1,
                             "name", qstr(i->first).c_str(),
                             "value", str(i->second).c_str(),
                             NULL);
            }

            os() << "]";
            write_object("", 0, 1, NULL);
        }
    };

    OutputDriver* MakeJSONOutputDriver(std::ostream *os) {
        return new JSONOutputDriver(os);
    }
}
