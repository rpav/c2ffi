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

#include <clang/AST/DeclObjC.h>

#include "c2ffi.h"
#include "c2ffi/ast.h"

using namespace c2ffi;

Decl::Decl(clang::NamedDecl *d) {
    _name = d->getDeclName().getAsString();
}

FieldsMixin::~FieldsMixin() {
    for(NameTypeVector::iterator i = _v.begin(); i != _v.end(); i++)
        delete (*i).second;
}

FunctionsMixin::~FunctionsMixin() {
    for(FunctionVector::iterator i = _v.begin(); i != _v.end(); i++)
        delete (*i);
}

void FieldsMixin::add_field(Name name, Type *t) {
    _v.push_back(NameTypePair(name, t));
}

void FieldsMixin::add_field(C2FFIASTConsumer *ast, clang::FieldDecl *f) {
    clang::ASTContext &ctx = ast->ci().getASTContext();
    Type *t = NULL;

    if(f->isBitField())
        t = new BitfieldType(ast->ci(), f->getTypeSourceInfo()->getType().getTypePtr(),
                             f->getBitWidthValue(ctx));
    else
        t = Type::make_type(ast, f->getTypeSourceInfo()->getType().getTypePtr());

    add_field(f->getDeclName().getAsString(), t);
}

void FieldsMixin::add_field(C2FFIASTConsumer *ast, clang::ParmVarDecl *p) {
        std::string name = p->getDeclName().getAsString();
        Type *t = Type::make_type(ast, p->getOriginalType().getTypePtr());
        add_field(name, t);
}

void FunctionsMixin::add_function(FunctionDecl *f) {
    _v.push_back(f);
}

void FunctionsMixin::add_functions(C2FFIASTConsumer *ast, const clang::ObjCContainerDecl *d) {
    for(clang::ObjCContainerDecl::method_iterator m = d->meth_begin();
        m != d->meth_end(); m++) {
        const clang::Type *return_type = m->getResultType().getTypePtr();
        FunctionDecl *fd = new FunctionDecl(m->getDeclName().getAsString(),
                                            Type::make_type(ast, return_type));

        fd->set_is_objc_method(true);
        fd->set_is_class_method(m->isClassMethod());

        for(clang::FunctionDecl::param_const_iterator i = m->param_begin();
            i != m->param_end(); i++) {
            fd->add_field(ast, *i);
        }

        add_function(fd);
    }
}

void EnumDecl::add_field(Name name, uint64_t v) {
    _v.push_back(NameNumPair(name, v));
}

void ObjCInterfaceDecl::add_protocol(Name name) {
    _protocols.push_back(name);
}
