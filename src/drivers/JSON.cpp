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
#include <iomanip>

using namespace c2ffi;

namespace c2ffi {
    class JSONOutputDriver : public OutputDriver {
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

        std::string hex_str(unsigned char c) {
            std::ostringstream ss;
            ss.setf(std::ios::hex, std::ios::basefield);
            ss << "\\u00" << std::setw(2) << std::setfill('0') << (int)c;

            return ss.str();
        }

        std::string escape_string(std::string s) {
            for(size_t i = s.find('\\'); i != std::string::npos;
                i = s.find('\\', i+2))
                s.replace(i, 1, "\\\\");

            for(size_t i = s.find('\"'); i != std::string::npos;
                i = s.find('\"', i+2))
                s.replace(i, 1, "\\\"");

            for(size_t i = 0; i < s.size(); i++)
                if((unsigned char)(s[i]) > 127 ||
                   (unsigned char)(s[i]) < 32) {
                    s.replace(i, 1, hex_str(s[i]));
                    i += 5;
                }

            return s;
        }

        template<typename T> std::string str(T v) {
            std::stringstream ss;
            ss << v;
            return ss.str();
        }

        template<typename T> std::string qstr(T v) {
            std::stringstream ss;
            ss << '"' << escape_string(v) << '"';
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
                             "bit-offset", str(i->second->bit_offset()).c_str(),
                             "bit-size", str(i->second->bit_size()).c_str(),
                             "bit-alignment", str(i->second->bit_alignment()).c_str(),
                             "type", NULL);
                write(*(i->second));
                write_object("", 0, 1, NULL);
            }

            os() << ']';
        }

        void write_template(const TemplateMixin &d) {
            if(d.is_template()) {
                write_object("", 0, 0,
                             "template", NULL);
                os() << "[";
                for(TemplateArgVector::const_iterator i =
                        d.args().begin();
                    i != d.args().end(); ++i) {
                    if(i != d.args().begin())
                        os() << ", ";

                    write_object("parameter", 1, 0,
                                 "type", NULL);
                    write(*((*i)->type()));

                    if((*i)->has_val())
                        write_object("", 0, 0,
                                     "value", qstr((*i)->val()).c_str(),
                                     NULL);

                    write_object("", 0, 1, NULL);
                }
                os() << "]";
            }
        }

        void write_functions(const FunctionVector &funcs) {
            os() << '[';
            for(FunctionVector::const_iterator i = funcs.begin();
                i != funcs.end(); i++) {
                if(i != funcs.begin())
                    os() << ", ";
                write((const Writable&)*(*i));
            }
            os() << ']';
        }

        void write_function_header(const FunctionDecl &d) {
            const char *variadic = d.is_variadic() ? "true" : "false";
            const char *inline_ = d.is_inline() ? "true" : "false";

            write_object("function", 1, 0,
                         "name", qstr(d.name()).c_str(),
                         "ns", str(d.ns()).c_str(),
                         "location", qstr(d.location()).c_str(),
                         "variadic", variadic,
                         "inline", inline_,
                         "storage-class", qstr(d.storage_class()).c_str(),
                         NULL);
            write_template(d);
        }

        void write_function_params(const FunctionDecl &d) {
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
        }

        void write_function_return(const FunctionDecl &d) {
            write_object("", 0, 0,
                         "return-type", NULL);
            write(d.return_type());
            write_object("", 0, 1, NULL);
        }


    public:
        JSONOutputDriver(std::ostream *os)
            : OutputDriver(os) { }

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

        virtual void write(const BasicType &t) {
            write_object(t.name().c_str(), 1, 1,
                         "bit-size", str(t.bit_size()).c_str(),
                         "bit-alignment", str(t.bit_alignment()).c_str(),
                         NULL);
        }

        virtual void write(const BitfieldType &t) {
            write_object(":bitfield", 1, 0,
                         "width", str(t.width()).c_str(),
                         "type", NULL);
            write(*t.base());
            write_object("", 0, 1, NULL);
        }

        virtual void write(const PointerType &t) {
            write_object(":pointer", 1, 0,
                         "type", NULL);
            write(t.pointee());
            write_object("", 0, 1, NULL);
        }

        virtual void write(const ReferenceType &t) {
            write_object(":reference", 1, 0,
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
            const char *type = NULL;

            if(t.is_union())
                type = ":union";
            else if(t.is_class())
                type = ":class";
            else
                type = ":struct";

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
                         "ns", str(d.ns()).c_str(),
                         "location", qstr(d.location()).c_str(),
                         "type", NULL);

            write(d.type());

            if(d.value() != "") {
                if(d.is_string()
                    || d.value() == "inf"
                    || d.value() == "INF"
                    || d.value() == "nan")
                    write_object("", 0, 0,
                                 "value", qstr(d.value()).c_str(),
                                 NULL);
                else
                    write_object("", 0, 0,
                                 "value", str(d.value()).c_str(),
                                 NULL);
            }

            write_object("", 0, 1, NULL);
        }

        virtual void write(const FunctionDecl &d) {
            write_function_header(d);

            if(d.is_objc_method())
                write_object("", 0, 0,
                             "scope", d.is_class_method() ? "\"class\"" : "\"instance\"",
                             NULL);

            write_function_params(d);
            write_function_return(d);
        }

        virtual void write(const CXXFunctionDecl &d) {
            write_function_header(d);

            write_object("", 0, 0,
                         "scope", d.is_static() ? "\"class\"" : "\"instance\"",
                         "virtual", d.is_virtual() ? "true" : "false",
                         "pure", d.is_pure() ? "true" : "false",
                         "const", d.is_const() ? "true" : "false",
                         NULL);

            write_function_params(d);
            write_function_return(d);
        }

        virtual void write(const TypedefDecl &d) {
            write_object("typedef", 1, 0,
                         "ns", str(d.ns()).c_str(),
                         "name", qstr(d.name()).c_str(),
                         "location", qstr(d.location()).c_str(),
                         "type", NULL);

            write(d.type());
            write_object("", 0, 1, NULL);
        }

        virtual void write(const RecordDecl &d) {
            const char *type = d.is_union() ? "union" : "struct";

            write_object(type, 1, 0,
                         "ns", str(d.ns()).c_str(),
                         "name", qstr(d.name()).c_str(),
                         "id", str(d.id()).c_str(),
                         "location", qstr(d.location()).c_str(),
                         "bit-size", str(d.bit_size()).c_str(),
                         "bit-alignment", str(d.bit_alignment()).c_str(),
                         "fields", NULL);

            write_fields(d.fields());
            write_object("", 0, 1, NULL);
        }

        virtual void write(const CXXRecordDecl &d) {
            const char *type = d.is_union() ? "union" :
                (d.is_class() ? "class" : "struct");

            write_object(type, 1, 0,
                         "ns", str(d.ns()).c_str(),
                         "name", qstr(d.name()).c_str(),
                         "id", str(d.id()).c_str(),
                         "location", qstr(d.location()).c_str(),
                         "bit-size", str(d.bit_size()).c_str(),
                         "bit-alignment", str(d.bit_alignment()).c_str(),
                         NULL);

            write_template(d);

            write_object("", 0, 0,
                         "parents", NULL);

            os() << "[";

            const CXXRecordDecl::ParentRecordVector &parents = d.parents();
            for(CXXRecordDecl::ParentRecordVector::const_iterator i
                    = parents.begin();
                i != parents.end(); ++i) {
                if(i != parents.begin())
                    os() << ", ";

                write_object("class", 1, 0,
                             "name", qstr((*i).name).c_str(),
                             "offset", str((*i).parent_offset).c_str(),
                             "is_virtual", ((*i).is_virtual ? "true" : "false"),
                             "access", NULL);

                switch((*i).access) {
                    case CXXRecordDecl::access_private:
                        os() << "\"private\""; break;
                    case CXXRecordDecl::access_protected:
                        os() << "\"protected\""; break;
                    case CXXRecordDecl::access_public:
                        os() << "\"public\""; break;
                    default:
                        os() << "\"unknown\"";
                }

                write_object("", 0, 1, NULL);
            }

            os() << "]";

            write_object("", 0, 0,
                         "fields", NULL);

            write_fields(d.fields());
            write_object("", 0, 0,
                         "methods", NULL);
            write_functions(d.functions());
            write_object("", 0, 1, NULL);
        }

        virtual void write(const CXXNamespaceDecl &d) {
            write_object("namespace", 1, 1,
                         "ns", str(d.ns()).c_str(),
                         "name", qstr(d.name()).c_str(),
                         "id", str(d.id()).c_str(),
                         NULL);
        }

        virtual void write(const EnumDecl &d) {
            write_object("enum", 1, 0,
                         "ns", str(d.ns()).c_str(),
                         "name", qstr(d.name()).c_str(),
                         "id", str(d.id()).c_str(),
                         "location", qstr(d.location()).c_str(),
                         "fields", NULL);

            os() << "[";
            const NameNumVector &fields = d.fields();
            for(NameNumVector::const_iterator i = fields.begin();
                i != fields.end(); ++i) {
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
