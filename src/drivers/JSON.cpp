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

        void write_fields(const NameTypeVector &fields) {
            os() << '[';
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

            os() << ']';
        }

        void write_functions(const FunctionVector &funcs) {
            os() << '[';
            for(FunctionVector::const_iterator i = funcs.begin();
                i != funcs.end(); i++) {
                if(i != funcs.begin())
                    os() << ", ";
                write(*(*i));
            }
            os() << ']';
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

        virtual void write_comment(const char *str) {
            write_object("comment", 1, 1,
                         "text", qstr(str).c_str(),
                         NULL);
        }

        virtual void write_namespace(const std::string &ns) {
            write_object("namespace", 1, 1,
                         "name", qstr(ns).c_str(),
                         NULL);
            write_between();
        }

        // Types -----------------------------------------------------------
        virtual void write(const SimpleType &t) {
            write_object(t.name().c_str(), 1, 1, NULL);
        }

        virtual void write(const BitfieldType &t) {
            write_object(":bitfield", 1, 0,
                         "width", str(t.width()).c_str(),
                         "type", NULL);
            write(*t.base());
            write_object("", 0, 1, NULL);
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
                         "id", str(t.id()).c_str(),
                         NULL);
        }

        virtual void write(const EnumType &t) {
            write_object(":enum", 1, 1,
                         "name", qstr(t.name()).c_str(),
                         "id", str(t.id()).c_str(),
                         NULL);
        }

        // Decls -----------------------------------------------------------
        virtual void write(const UnhandledDecl &d) {
            write_object("unhandled", 1, 1,
                         "name", qstr(d.name()).c_str(),
                         "kind", qstr(d.kind()).c_str(),
                         "location", qstr(d.location()).c_str(),
                         NULL);
        }

        virtual void write(const VarDecl &d) {
            const char *type = d.is_extern() ? "extern" : "const";

            write_object(type, 1, 0,
                         "name", qstr(d.name()).c_str(),
                         "location", qstr(d.location()).c_str(),
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
                         "location", qstr(d.location()).c_str(),
                         "variadic", qstr(d.is_variadic()).c_str(),
                         NULL);

            if(d.is_objc_method())
                write_object("", 0, 0,
                             "scope", d.is_class_method() ? "\"class\"" : "\"instance\"",
                             NULL);

            write_object("", 0, 0, "parameters", NULL);
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
                         "location", qstr(d.location()).c_str(),
                         "type", NULL);

            write(d.type());
            write_object("", 0, 1, NULL);
        }

        virtual void write(const RecordDecl &d) {
            const char *type = d.is_union() ? "union" : "struct";

            write_object(type, 1, 0,
                         "name", qstr(d.name()).c_str(),
                         "id", str(d.id()).c_str(),
                         "location", qstr(d.location()).c_str(),
                         "fields", NULL);

            write_fields(d.fields());
            write_object("", 0, 1, NULL);
        }

        virtual void write(const EnumDecl &d) {
            write_object("enum", 1, 0,
                         "name", qstr(d.name()).c_str(),
                         "id", str(d.id()).c_str(),
                         "location", qstr(d.location()).c_str(),
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

        virtual void write(const ObjCInterfaceDecl &d) {
            write_object(d.is_forward() ? "@class" : "@interface", 1, 0,
                         "name", qstr(d.name()).c_str(),
                         "location", qstr(d.location()).c_str(),
                         "superclass", qstr(d.super()).c_str(),
                         "protocols", NULL);

            os() << "[";
            const NameVector &protos = d.protocols();
            for(NameVector::const_iterator i = protos.begin();
                i != protos.end(); i++) {
                if(i != protos.begin())
                    os() << ", ";
                os() << qstr(*i).c_str();
            }
            os() << "]";

            write_object("", 0, 0,
                         "ivars", NULL);
            write_fields(d.fields());

            write_object("", 0, 0,
                         "methods", NULL);
            write_functions(d.functions());

            write_object("", 0, 1, NULL);
        }

        virtual void write(const ObjCCategoryDecl &d) {
            write_object("@category", 1, 0,
                         "name", qstr(d.name()).c_str(),
                         "location", qstr(d.location()).c_str(),
                         "category", qstr(d.category()).c_str(),
                         "methods", NULL);
            write_functions(d.functions());
            write_object("", 0, 1, NULL);
        }

        virtual void write(const ObjCProtocolDecl &d) {
            write_object("@protocol", 1, 0,
                         "name", qstr(d.name()).c_str(),
                         "location", qstr(d.location()).c_str(),
                         "methods", NULL);
            write_functions(d.functions());
            write_object("", 0, 1, NULL);
        }
    };

    OutputDriver* MakeJSONOutputDriver(std::ostream *os) {
        return new JSONOutputDriver(os);
    }
}
