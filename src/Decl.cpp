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

#include "c2ffi.h"

using namespace c2ffi;

Decl::Decl(clang::NamedDecl *d) {
    _name = d->getDeclName().getAsString();
}

FieldsDecl::~FieldsDecl() {
    for(NameTypeVector::iterator i = _v.begin(); i != _v.end(); i++)
        delete (*i).second;
}

void FieldsDecl::add_field(std::string name, Type *t) {
    _v.push_back(NameTypePair(name, t));
}

void EnumDecl::add_field(std::string name, uint64_t v) {
    _v.push_back(NameNumPair(name, v));
}
