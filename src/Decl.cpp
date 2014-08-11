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
#include <clang/AST/DeclCXX.h>
#include <clang/AST/ASTContext.h>

#include "c2ffi.h"
#include "c2ffi/ast.h"

using namespace c2ffi;

Decl::Decl(clang::NamedDecl *d) {
    _name = d->getDeclName().getAsString();
}

void Decl::set_location(clang::CompilerInstance &ci, const clang::Decl *d) {
    clang::SourceLocation sloc = d->getLocation();

    if(sloc.isValid()) {
        std::string loc = sloc.printToString(ci.getSourceManager());
        set_location(loc);
    }
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
    std::pair<uint64_t, unsigned> type_info =
        ctx.getTypeInfo(f->getTypeSourceInfo()->getType().getTypePtr());
    Type *t = Type::make_type(ast, f->getTypeSourceInfo()->getType().getTypePtr());;

    if(f->isBitField())
        t = new BitfieldType(ast->ci(), f->getTypeSourceInfo()->getType().getTypePtr(),
                             f->getBitWidthValue(ctx), t);

    t->set_bit_offset(ctx.getFieldOffset(f));
    t->set_bit_size(type_info.first);
    t->set_bit_alignment(type_info.second);

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
        FunctionDecl *f = new FunctionDecl(m->getDeclName().getAsString(),
                                            Type::make_type(ast, return_type),
                                            m->isVariadic(), false,
                                            clang::SC_None);

        f->set_is_objc_method(true);
        f->set_is_class_method(m->isClassMethod());
        f->set_location(ast->ci(), (*m));

        for(clang::FunctionDecl::param_const_iterator i = m->param_begin();
            i != m->param_end(); i++) {
            f->add_field(ast, *i);
        }

        add_function(f);
    }
}

void FunctionsMixin::add_functions(C2FFIASTConsumer *ast, const clang::CXXRecordDecl *d) {
    for(clang::CXXRecordDecl::method_iterator i = d->method_begin();
        i != d->method_end(); ++i) {
        const clang::CXXMethodDecl *m = (*i);
        const clang::Type *return_type = m->getResultType().getTypePtr();

        CXXFunctionDecl *f = new CXXFunctionDecl(m->getDeclName().getAsString(),
                                                 Type::make_type(ast, return_type),
                                                 m->isVariadic(),
                                                 m->isInlineSpecified(),
                                                 m->getStorageClass());

        f->set_is_static(m->isStatic());
        f->set_is_virtual(m->isVirtual());
        f->set_is_const(m->isConst());
        f->set_is_pure(m->isPure());
        f->set_location(ast->ci(), m);

        for(clang::FunctionDecl::param_const_iterator i = m->param_begin();
            i != m->param_end(); i++) {
            f->add_field(ast, *i);
        }

        add_function(f);
    }
}

static const char *sc2str[] = {
    "none", "extern", "static", "private_extern"
};

FunctionDecl::FunctionDecl(std::string name, Type *type, bool is_variadic,
                           bool is_inline, clang::StorageClass storage_class)
    : Decl(name), _return(type), _is_variadic(is_variadic), _is_inline(is_inline),
      _storage_class("unknown"),
      _is_class_method(false), _is_objc_method(false) {

    if(storage_class < sizeof(sc2str) / sizeof(*sc2str))
        _storage_class = sc2str[storage_class];
}

void RecordDecl::fill_record_decl(C2FFIASTConsumer *ast, const clang::RecordDecl *d) {
    clang::ASTContext &ctx = ast->ci().getASTContext();
    std::string name = d->getDeclName().getAsString();
    const clang::Type *t = d->getTypeForDecl();

    if(!t->isIncompleteType()) {
        set_bit_size(ctx.getTypeSize(t));
        set_bit_alignment(ctx.getTypeAlign(t));
    } else {
        set_bit_size(0);
        set_bit_alignment(0);
    }

    if(name == "")
        set_id(ast->add_decl(d));

    for(clang::RecordDecl::field_iterator i = d->field_begin();
        i != d->field_end(); i++)
        add_field(ast, *i);
}

void EnumDecl::add_field(Name name, uint64_t v) {
    _v.push_back(NameNumPair(name, v));
}

void ObjCInterfaceDecl::add_protocol(Name name) {
    _protocols.push_back(name);
}
