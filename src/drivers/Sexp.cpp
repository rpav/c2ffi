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
    class SexpOutputDriver : public OutputDriver {
        int _level;

        void endl() { if(_level <= 1) os() << std::endl; }

        void write_fields(const NameTypeVector &fields,
                          std::string pre = "",
                          std::string post = "") {
            std::string spaces(_level * 2, ' ');
            std::string spaces_pad(pre.size(), ' ');

            os() << std::endl << spaces << pre;

            for(NameTypeVector::const_iterator i = fields.begin();
                i != fields.end(); i++) {
                if(i != fields.begin())
                    os() << std::endl << spaces << spaces_pad;

                os()  << "(" << i->first << " ";
                write(*(i->second));
                os() << ")";
            }

            os() << post;
        }

        void write_functions(const FunctionVector &funcs) {
            std::string spaces(_level * 2, ' ');

            os() << std::endl << spaces << '(';

            for(FunctionVector::const_iterator i = funcs.begin();
                i != funcs.end(); i++) {
                if(i != funcs.begin())
                    os() << std::endl << spaces << " ";

                if((*i)->is_objc_method()) {
                    os() << "(";
                    if((*i)->is_class_method())
                        os() << "@+ ";
                    else
                        os() << "@- ";
                }

                write(*(*i));

                if((*i)->is_objc_method())
                    os() << ")";
            }

            os() << ')';
        }

        void maybe_write_location(const Decl &d) {
            if(d.location() != "") {
                endl();
                write_comment(d.location().c_str());
            }
        }

    public:
        SexpOutputDriver(std::ostream *os)
            : OutputDriver(os), _level(0) { }

        virtual void write_namespace(const std::string &ns) {
            os() << "(in-package :" << ns << ")" << std::endl;
        }

        virtual void write_comment(const char *str) {
            os() << ";; " << str << std::endl;
        }

        using OutputDriver::write;

        // Types -----------------------------------------------------------
        virtual void write(const SimpleType &t) {
            this->os() << t.name();
        }

        virtual void write(const BitfieldType &t) {
            os() << "(:bitfield " << t.width() << ")";
        }

        virtual void write(const PointerType& t) {
            _level++;
            this->os() << "(:pointer ";
            this->write(t.pointee());
            this->os() << ")";
            _level--;
        }

        virtual void write(const ArrayType &t) {
            _level++;
            os() << "(:array ";
            write(t.pointee());
            os() << " " << t.size() << ")";
            _level--;
        }

        virtual void write(const RecordType &t) {
            os() << "(";
            if(t.is_union())
                os() << ":union ";
            else
                os() << ":struct ";

            os() << t.name() << ")";
        }

        virtual void write(const EnumType &t) {
            os() << "(:enum " << t.name() << ")";
        }

        // Decls -----------------------------------------------------------
        virtual void write(const UnhandledDecl &d) {
            _level++;
            os() << ";; Unhandled: <" << d.kind() << "> " << d.name()
                 << " " << d.location();
            os() << std::endl;
            _level--;
        }

        virtual void write(const VarDecl &d) {
            _level++;
            maybe_write_location(d);
            if(d.is_extern())
                os() << "(extern ";
            else
                os() << "(const ";

            os() << d.name() << " ";
            write(d.type());

            if(d.value() != "")
                os() << " " << d.value();

            os() << ")";
            endl();
            _level--;
        }

        virtual void write(const FunctionDecl &d) {
            _level++;
            maybe_write_location(d);
            os() << "(function \"" << d.name() << "\" (";

            const NameTypeVector &params = d.fields();
            for(NameTypeVector::const_iterator i = params.begin();
                i != params.end(); i++) {
                if(i != params.begin())
                    os() << " ";

                os() << "(" << (*i).first;

                if((*i).first != "")
                    os() << " ";

                write(*(*i).second);
                os() << ")";
            }

            os() << ") ";
            write(d.return_type());
            os() << ")";
            endl();
            _level--;
        }

        virtual void write(const TypedefDecl &d) {
            _level++;
            maybe_write_location(d);
            os() << "(typedef " << d.name() << " ";
            write(d.type());
            os() << ")";
            endl();
            _level--;
        }


        virtual void write(const RecordDecl &d) {
            _level++;
            maybe_write_location(d);
            os() << "(";

            if(d.is_union())
                os() << "union ";
            else
                os() << "struct ";

            os() << d.name();
            write_fields(d.fields());
            os() << ")"; endl();
            _level--;
        }

        virtual void write(const EnumDecl &d) {
            _level++;
            maybe_write_location(d);
            os() << "(enum " << d.name();

            const NameNumVector &fields = d.fields();
            for(NameNumVector::const_iterator i = fields.begin();
                i != fields.end(); i++) {
                os() << std::endl
                     << "    (" << i->first << " " << i->second
                     << ")";
            }

            os() << ")"; endl();
            _level--;
        }

        virtual void write(const ObjCInterfaceDecl &d) {
            _level++;
            maybe_write_location(d);
            if(d.is_forward())
                os() << "(@class " << d.name();
            else
                os() << "(@interface " << d.name();

            os() << " (" << d.super() << ") ";

            os() << "(";
            const NameVector &protos = d.protocols();
            for(NameVector::const_iterator i = protos.begin();
                i != protos.end(); i++) {
                if(i != protos.begin())
                    os() << " ";
                os() << *i;
            }
            os() << ")";

            write_fields(d.fields(), "(", ")");
            write_functions(d.functions());

            os() << ")"; endl();
            _level--;
        }

        virtual void write(const ObjCCategoryDecl &d) {
            _level++;
            maybe_write_location(d);
            os() << "(@category " << d.name()
                 << " (" << d.category() << ")";
            write_functions(d.functions());
            os() << ")"; endl();
            _level--;
        }

        virtual void write(const ObjCProtocolDecl &d) {
            _level++;
            maybe_write_location(d);
            os() << "(@protocol " << d.name();
            write_functions(d.functions());
            os() << ")"; endl();
            _level--;
        }

    };

    OutputDriver* MakeSexpOutputDriver(std::ostream *os) {
        return new SexpOutputDriver(os);
    }
}
