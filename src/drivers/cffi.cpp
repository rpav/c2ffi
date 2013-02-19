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

#include <sstream>
#include "c2ffi.h"

using namespace c2ffi;

namespace c2ffi {
    class CFFIOutputDriver : public OutputDriver {
        int _level;

        void endl() { if(_level <= 1) os() << std::endl; }

        std::string ncvt(const std::string &s, const std::string &type) {
            std::stringstream ss;

            ss << "#.(c2ffi-rename \"" << s << "\" :" << type << ")";
            return ss.str();
        }

        void export_sym(const std::string &s, const std::string &type) {
            os() << "(export '" << ncvt(s, type) << ")" << std::endl;
        }

    public:
        CFFIOutputDriver(std::ostream *os)
            : OutputDriver(os), _level(0) { }

        virtual void write_namespace(const std::string &ns) {
            os() << "(cl:in-package :" << ns << ")" << std::endl << std::endl;
            os() <<
                "(cl:eval-when (:compile-toplevel :load-toplevel :execute)\n"
                "  (cl:unless (cl:fboundp 'c2ffi-rename)\n"
                "    (cl:defun c2ffi-rename (c-name c-type)\n"
                "      (cl:let ((hyphenated (cl:if (eq #\\_ (aref c-name 0))\n"
                "                               (cl:string-upcase c-name)\n"
                "                               (cl:nstring-upcase\n"
                "                                (cl:substitute #\\- #\\_ c-name)))))\n"
                "        (cl:cond\n"
                "          ((cl:eq c-type :cconst)\n"
                "           (cl:intern (cl:format cl:nil \"+~A+\" hyphenated)))\n"
                "          ((cl:eq c-type :cenumfield)\n"
                "           (cl:intern hyphenated :keyword))\n"
                "          (cl:t (cl:intern hyphenated)))))))\n"
                 << std::endl;
        }

        virtual void write_comment(const char *str) {
            os() << ";; " << str << std::endl;
        }

        using OutputDriver::write;

        // Types -----------------------------------------------------------
        virtual void write(const SimpleType &t) {
            if(t.name()[0] != ':')
                this->os() << ncvt(t.name(), "ctype");
            else
                this->os() << t.name();
        }

        virtual void write(const BitfieldType &t) {
            os() << ":bitfield " << t.width();
        }

        virtual void write(const PointerType& t) {
            _level++;
            if(t.is_string())
                os() << ":string";
            else {
                os() << "(:pointer ";
                write(t.pointee());
                os() << ")";
            }
            _level--;
        }

        virtual void write(const ArrayType &t) {
            _level++;
            write(t.pointee());
            os() << " :count " << t.size();
            _level--;
        }

        virtual void write(const RecordType &t) {
            os() << "(";
            if(t.is_union())
                os() << ":union ";
            else
                os() << ":struct ";

            os() << ncvt(t.name(), "cstruct") << ")";
        }

        // Decls -----------------------------------------------------------
        virtual void write(const UnhandledDecl &d) {
            _level++;
            os() << ";; Unhandled: <" << d.kind() << "> " << d.name();
            endl();
            _level--;
        }

        virtual void write(const VarDecl &d) {
            _level++;
            if(d.is_extern()) {
                os() << "(cffi:defcvar " << ncvt(d.name(), "cvar") << " ";
                write(d.type());
                os() << ")";
                endl();
                export_sym(d.name(), "cvar");
            } else if(d.value() != "") {
                // Don't use defconstant here because it's problematic
                // with strings
                os() << "(cl:defvar " << ncvt(d.name(), "cconst") << " "
                     << d.value() << ")";
                endl();
                export_sym(d.name(), "cconst");
            }
            _level--;
        }

        virtual void write(const FunctionDecl &d) {
            _level++;
            os() << "(cffi:defcfun (\"" << d.name() << "\" "
                 << ncvt(d.name(), "cfun") << ") ";
            write(d.return_type());
            endl();

            const NameTypeVector &params = d.fields();
            for(NameTypeVector::const_iterator i = params.begin();
                i != params.end(); i++) {
                if(i != params.begin())
                    os() << std::endl;

                os() << "    (" << ncvt((*i).first, "cparam");

                if((*i).first != "")
                    os() << " ";

                write(*(*i).second);
                os() << ")";
            }

            os() << ")";
            endl();

            export_sym(d.name(), "cfun");
            _level--;
        }

        virtual void write(const TypedefDecl &d) {
            _level++;
            os() << "(cffi:defctype " << ncvt(d.name(), "ctype") << " ";
            write(d.type());
            os() << ")";
            endl();
            export_sym(d.name(), "ctype");
            _level--;
        }

        virtual void write(const RecordDecl &d) {
            _level++;
            os() << "(cffi:";

            if(d.is_union())
                os() << "defcunion " << ncvt(d.name(), "cunion");
            else
                os() << "defcstruct " << ncvt(d.name(), "cstruct");

            const NameTypeVector &fields = d.fields();
            for(NameTypeVector::const_iterator i = fields.begin();
                i != fields.end(); i++) {
                os() << std::endl
                     << "    (" << ncvt(i->first, "cfield") << " ";
                write(*(i->second));
                os() << ")";
            }

            os() << ")"; endl();

            if(d.is_union())
                export_sym(d.name(), "cunion");
            else
                export_sym(d.name(), "cstruct");

            _level--;
        }

        virtual void write(const EnumDecl &d) {
            _level++;
            os() << "(cffi:defcenum " << ncvt(d.name(), "cenum");

            const NameNumVector &fields = d.fields();
            for(NameNumVector::const_iterator i = fields.begin();
                i != fields.end(); i++) {
                os() << std::endl
                     << "    (" << ncvt(i->first, "cenumfield")
                     << " " << i->second
                     << ")";
            }

            os() << ")"; endl();
            export_sym(d.name(), "cenum");
            _level--;
        }
    };

    OutputDriver* MakeCFFIOutputDriver(std::ostream *os) {
        return new CFFIOutputDriver(os);
    }
}
