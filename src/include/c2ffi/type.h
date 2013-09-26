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

#ifndef C2FFI_TYPE_H
#define C2FFI_TYPE_H

#include <string>
#include <ostream>

#include <stdint.h>

#include <clang/AST/Type.h>
#include <clang/AST/Decl.h>
#include <clang/Frontend/CompilerInstance.h>

#include "c2ffi.h"

namespace c2ffi {
    class C2FFIASTConsumer;

    class Type : public Writable {
        unsigned int _id;
    protected:
        const clang::CompilerInstance &_ci;
        const clang::Type *_type;

        uint64_t _bit_offset;
        uint64_t _bit_size;
        unsigned _bit_alignment;

        friend class PointerType;
    public:
        Type(const clang::CompilerInstance &ci, const clang::Type *t);
        virtual ~Type() { }

        static Type* make_type(C2FFIASTConsumer*, const clang::Type*);

        unsigned int id() const { return _id; }
        void set_id(unsigned int id) { _id = id; }

        uint64_t bit_offset() const { return _bit_offset; }
        void set_bit_offset(uint64_t offset) { _bit_offset = offset; }

        uint64_t bit_size() const { return _bit_size; }
        void set_bit_size(uint64_t size) { _bit_size = size; }

        uint64_t bit_alignment() const { return _bit_alignment; }
        void set_bit_alignment(uint64_t alignment) { _bit_alignment = alignment; }

        std::string metatype() const;
    };

    // :int, :unsigned-char, :void, typedef names, etc
    class SimpleType : public Type {
        std::string _name;
    public:
        SimpleType(const clang::CompilerInstance &ci, const clang::Type *t,
                   std::string name)
            : Type(ci, t), _name(name) { }

        const std::string& name() const { return _name; }

        virtual void write(OutputDriver &od) const { od.write((const SimpleType&)*this); }
    };

    class BitfieldType : public Type {
        Type *_base;
        unsigned int _width;
    public:
        BitfieldType(const clang::CompilerInstance &ci, const clang::Type *t,
                     unsigned int width, Type *base)
            : Type(ci, t), _width(width), _base(base) { }

        virtual ~BitfieldType() { delete _base; }

        const Type* base() const { return _base; }
        unsigned int width() const { return _width; }

        virtual void write(OutputDriver &od) const { od.write((const BitfieldType&)*this); }
    };

    // This could be simple, but we want to be specific about what
    // we're pointing _to_
    class PointerType : public Type {
        Type *_pointee;
    public:
        PointerType(const clang::CompilerInstance &ci, const clang::Type *t,
                    Type *pointee)
            : Type(ci, t), _pointee(pointee) { }
        virtual ~PointerType() { delete _pointee; }

        const Type& pointee() const { return *_pointee; }
        bool is_string() const;

        virtual void write(OutputDriver &od) const { od.write((const PointerType&)*this); }
    };

    class ReferenceType : public PointerType {
    public:
        ReferenceType(const clang::CompilerInstance &ci, const clang::Type *t,
                    Type *pointee)
            : PointerType(ci, t, pointee) { }
        virtual void write(OutputDriver &od) const { od.write((const ReferenceType&)*this); }
    };

    class ArrayType : public PointerType {
        uint64_t _size;
    public:
        ArrayType(const clang::CompilerInstance &ci, const clang::Type *t,
                  Type *pointee, uint64_t size)
            : PointerType(ci, t, pointee), _size(size) { }

        uint64_t size() const { return _size; }
        virtual void write(OutputDriver &od) const { od.write((const ArrayType&)*this); }
    };

    class RecordType : public SimpleType {
        bool _is_union;
        bool _is_class;
    public:
        RecordType(const clang::CompilerInstance &ci, const clang::Type *t,
                   std::string name, bool is_union = false,
                   bool is_class = false)
            : SimpleType(ci, t, name), _is_union(is_union),
              _is_class(is_class) { }

        bool is_union() const { return _is_union; }
        bool is_class() const { return _is_class; }
        virtual void write(OutputDriver &od) const { od.write((const RecordType&)*this); }
    };

    class EnumType : public SimpleType {
    public:
        EnumType(const clang::CompilerInstance &ci, const clang::Type *t,
                 std::string name)
            : SimpleType(ci, t, name) { }
        virtual void write(OutputDriver &od) const { od.write((const EnumType&)*this); }
    };

    // This is a bit of a hack to contain inline declarations (e.g.,
    // anonymous typedef struct)
    class DeclType : public Type {
        Decl *_d;
    public:
        DeclType(clang::CompilerInstance &ci, const clang::Type *t,
                 Decl *d, const clang::Decl *cd);

        // Note, this cheats:
        virtual void write(OutputDriver &od) const;
    };
}

#endif /* C2FFI_TYPE_H */
