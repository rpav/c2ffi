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

#ifndef C2FFI_DECL_H
#define C2FFI_DECL_H

#include <string>
#include <utility>
#include <vector>
#include <clang/AST/Decl.h>
#include "c2ffi.h"
#include "c2ffi/type.h"

namespace c2ffi {
    typedef std::pair<std::string, Type*> NameTypePair;
    typedef std::vector<NameTypePair> NameTypeVector;

    typedef std::pair<std::string, uint64_t> NameNumPair;
    typedef std::vector<NameNumPair> NameNumVector;

    class Decl : public Writable {
        std::string _name;
    public:
        Decl(std::string name)
            : _name(name) { }
        Decl(clang::NamedDecl *d);
        virtual ~Decl() { }

        virtual const std::string& name() const { return _name; }
    };

    class UnhandledDecl : public Decl {
        std::string _kind;
    public:
        UnhandledDecl(std::string name, std::string kind)
            : Decl(name), _kind(kind) { }

        virtual void write(OutputDriver &od) const { od.write((const UnhandledDecl&)*this); }
        const std::string& kind() const { return _kind; }
    };

    class TypeDecl : public Decl {
        Type *_type;
    public:
        TypeDecl(std::string name, Type *type)
            : Decl(name), _type(type) { }
        virtual ~TypeDecl() { delete _type; }

        virtual void write(OutputDriver &od) const { od.write((const TypeDecl&)*this); }
        virtual const Type& type() const { return *_type; }
    };

    class VarDecl : public TypeDecl {
        std::string _value;
    public:
        VarDecl(std::string name, Type *type, std::string value = "")
            : TypeDecl(name, type), _value(value) { }

        virtual void write(OutputDriver &od) const { od.write((const VarDecl&)*this); }

        const std::string& value() const { return _value; }
    };

    class FieldsDecl : public Decl {
        NameTypeVector _v;
    public:
        FieldsDecl(std::string name) : Decl(name) { }
        virtual ~FieldsDecl();

        void add_field(std::string, Type*);
        const NameTypeVector& fields() const { return _v; }
    };

    class FunctionDecl : public FieldsDecl {
        Type *_return;
    public:
        FunctionDecl(std::string name, Type *type)
            : FieldsDecl(name), _return(type) { }

        virtual void write(OutputDriver &od) const { od.write((const FunctionDecl&)*this); }

        virtual const Type& return_type() const { return *_return; }
    };

    class TypedefDecl : public TypeDecl {
    public:
        TypedefDecl(std::string name, Type *type)
            : TypeDecl(name, type) { }

        virtual void write(OutputDriver &od) const { od.write((const TypedefDecl&)*this); }
    };

    class RecordDecl : public FieldsDecl {
        bool _is_union;
        NameTypeVector _v;

    public:
        RecordDecl(std::string name, bool is_union = false)
            : FieldsDecl(name), _is_union(is_union) { }

        virtual void write(OutputDriver &od) const { od.write((const RecordDecl&)*this); }
        bool is_union() const { return _is_union; }
    };

    class EnumDecl : public Decl {
        NameNumVector _v;
    public:
        EnumDecl(std::string name) : Decl(name) { }

        virtual void write(OutputDriver &od) const { od.write((const EnumDecl&)*this); }

        void add_field(std::string, uint64_t);
        const NameNumVector& fields() const { return _v; }
    };
}

#endif /* C2FFI_DECL_H */
