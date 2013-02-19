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

#include <iostream>

#include <llvm/Support/raw_ostream.h>
#include <llvm/Support/Host.h>
#include <llvm/ADT/IntrusiveRefCntPtr.h>

#include <clang/Basic/DiagnosticOptions.h>
#include <clang/Frontend/TextDiagnosticPrinter.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Basic/TargetOptions.h>
#include <clang/Basic/TargetInfo.h>
#include <clang/Basic/FileManager.h>
#include <clang/Basic/SourceManager.h>
#include <clang/Lex/HeaderSearch.h>
#include <clang/Lex/Preprocessor.h>
#include <clang/Basic/Diagnostic.h>
#include <clang/AST/ASTContext.h>
#include <clang/AST/ASTConsumer.h>
#include <clang/Parse/Parser.h>
#include <clang/Parse/ParseAST.h>

#include "c2ffi.h"
#include "c2ffi/ast.h"

using namespace c2ffi;

static std::string value_to_string(clang::APValue *v) {
    std::string s;
    llvm::raw_string_ostream ss(s);

    if(v->isInt() && v->getInt().isSigned())
        v->getInt().print(ss, true);
    else if(v->isInt())
        v->getInt().print(ss, false);
    else if(v->isFloat())
        ss << v->getFloat().convertToDouble();

    ss.flush();
    return s;
}

bool C2FFIASTConsumer::HandleTopLevelDecl(clang::DeclGroupRef d) {
    clang::DeclGroupRef::iterator it;

    for(it = d.begin(); it != d.end(); it++) {
        Decl *decl = NULL;

        if_cast(x, clang::VarDecl, *it) { decl = make_decl(x); }
        else if_cast(x, clang::FunctionDecl, *it) { decl = make_decl(x); }
        else if_cast(x, clang::RecordDecl, *it) { decl = make_decl(x); }
        else if_cast(x, clang::EnumDecl, *it) { decl = make_decl(x); }
        else if_cast(x, clang::TypedefDecl, *it) { decl = make_decl(x); }
        else if_cast(x, clang::NamedDecl, *it) { decl = make_decl(x); }
        else decl = make_decl(*it);

        if(decl) {
            if(_mid) _od->write_between();
            else _mid = true;

            _od->write(*decl);
            delete decl;
        }
    }

    return true;
}

Decl* C2FFIASTConsumer::make_decl(const clang::Decl *d, bool is_toplevel) {
    return new UnhandledDecl("", d->getDeclKindName());
}

Decl* C2FFIASTConsumer::make_decl(const clang::NamedDecl *d, bool is_toplevel) {
    return new UnhandledDecl(d->getDeclName().getAsString(),
                             d->getDeclKindName());
}

Decl* C2FFIASTConsumer::make_decl(const clang::FunctionDecl *d, bool is_toplevel) {
    const clang::Type *return_type = d->getResultType().getTypePtr();
    FunctionDecl *fd = new FunctionDecl(d->getDeclName().getAsString(),
                                        Type::make_type(this, return_type));

    for(clang::FunctionDecl::param_const_iterator i = d->param_begin();
        i != d->param_end(); i++) {
        clang::ParmVarDecl *p = (*i);
        std::string name = p->getDeclName().getAsString();
        Type *t = Type::make_type(this, p->getOriginalType().getTypePtr());

        fd->add_field(name, t);
    }

    return fd;
}

Decl* C2FFIASTConsumer::make_decl(const clang::VarDecl *d, bool is_toplevel) {
    clang::ASTContext &ctx = _ci.getASTContext();
    clang::APValue *v = NULL;
    std::string name = d->getDeclName().getAsString(),
                value = "";

    if(name.substr(0, 8) == "__c2ffi_")
        name = name.substr(8, std::string::npos);

    if(d->hasInit() && ((v = d->evaluateValue()) ||
                        (v = d->getEvaluatedValue()))) {
        /* FIXME: massive hack.  Should probably implement our own
           printing for strings, but this requires further delving
           into the StmtPrinter source. */
        if(v->isLValue()) {
            clang::APValue::LValueBase base = v->getLValueBase();
            const clang::Expr *e = base.get<const clang::Expr*>();

            if(e) {
                llvm::raw_string_ostream ss(value);
                e->printPretty(ss, 0, ctx.getPrintingPolicy());
                ss.flush();
            }
        } else {
            value = value_to_string(v);
        }
    }

    Type *t = Type::make_type(this, d->getTypeSourceInfo()->getType().getTypePtr());
    return new VarDecl(name, t, value);
}

Decl* C2FFIASTConsumer::make_decl(const clang::RecordDecl *d, bool is_toplevel) {
    std::string name = d->getDeclName().getAsString();
    clang::ASTContext &ctx = _ci.getASTContext();

    if(is_toplevel && name == "") return NULL;

    RecordDecl *rd = new RecordDecl(name, d->isUnion());

    for(clang::RecordDecl::field_iterator i = d->field_begin();
        i != d->field_end(); i++) {
        const clang::FieldDecl *f = (*i);

        Type *t = NULL;

        if(f->isBitField())
            t = new BitfieldType(_ci, f->getTypeSourceInfo()->getType().getTypePtr(),
                                 f->getBitWidthValue(ctx));
        else
            t = Type::make_type(this, f->getTypeSourceInfo()->getType().getTypePtr());

        rd->add_field(f->getDeclName().getAsString(), t);
    }

    return rd;
}

Decl* C2FFIASTConsumer::make_decl(const clang::TypedefDecl *d, bool is_toplevel) {
    return new TypedefDecl(d->getDeclName().getAsString(),
                           Type::make_type(this, d->getTypeSourceInfo()->getType().getTypePtr()));
}

Decl* C2FFIASTConsumer::make_decl(const clang::EnumDecl *d, bool is_toplevel) {
    std::string name = d->getDeclName().getAsString();

    if(is_toplevel && name == "") return NULL;

    EnumDecl *decl = new EnumDecl(name);

    for(clang::EnumDecl::enumerator_iterator i = d->enumerator_begin();
        i != d->enumerator_end(); i++) {
        const clang::EnumConstantDecl *ecd = (*i);
        decl->add_field(ecd->getDeclName().getAsString(),
                        ecd->getInitVal().getLimitedValue());
    }

    return decl;
}
