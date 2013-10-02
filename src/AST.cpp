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
#include <map>

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

void C2FFIASTConsumer::HandleTopLevelDeclInObjCContainer(clang::DeclGroupRef d) {
    _od->write_comment("HandleTopLevelDeclInObjCContainer");
}

bool C2FFIASTConsumer::HandleTopLevelDecl(clang::DeclGroupRef d) {
    clang::DeclGroupRef::iterator it;

    for(it = d.begin(); it != d.end(); it++) {
        Decl *decl = NULL;

        if((*it)->isInvalidDecl()) {
            std::cerr << "Skipping invalid Decl:" << std::endl;
            (*it)->dump();
            continue;
        }

        if_cast(x, clang::VarDecl, *it) { decl = make_decl(x); }
        else if_cast(x, clang::FunctionDecl, *it) { decl = make_decl(x); }
        else if_cast(x, clang::RecordDecl, *it) { decl = make_decl(x); }
        else if_cast(x, clang::EnumDecl, *it) { decl = make_decl(x); }
        else if_cast(x, clang::TypedefDecl, *it) { decl = make_decl(x); }

        /* ObjC */
        else if_cast(x, clang::ObjCInterfaceDecl, *it) { decl = make_decl(x); }
        else if_cast(x, clang::ObjCCategoryDecl, *it) { decl = make_decl(x); }
        else if_cast(x, clang::ObjCProtocolDecl, *it) { decl = make_decl(x); }
        else if_cast(x, clang::ObjCImplementationDecl, *it) continue;
        else if_cast(x, clang::ObjCMethodDecl, *it) continue;

        /* Always should be last */
        else if_cast(x, clang::NamedDecl, *it) { decl = make_decl(x); }
        else decl = make_decl(*it);

        if(decl) {
            decl->set_location(_ci, (*it));

            if(_mid) _od->write_between();
            else _mid = true;

            _od->write(*decl);
            delete decl;
        }
    }

    return true;
}

bool C2FFIASTConsumer::is_cur_decl(const clang::Decl *d) const {
    return _cur_decls.count(d);
}

Decl* C2FFIASTConsumer::make_decl(const clang::Decl *d, bool is_toplevel) {
    return new UnhandledDecl("", d->getDeclKindName());
}

Decl* C2FFIASTConsumer::make_decl(const clang::NamedDecl *d, bool is_toplevel) {
    return new UnhandledDecl(d->getDeclName().getAsString(),
                             d->getDeclKindName());
}

Decl* C2FFIASTConsumer::make_decl(const clang::FunctionDecl *d, bool is_toplevel) {
    _cur_decls.insert(d);

    const clang::Type *return_type = d->getResultType().getTypePtr();
    FunctionDecl *fd = new FunctionDecl(d->getDeclName().getAsString(),
                                        Type::make_type(this, return_type),
                                        d->isVariadic());

    for(clang::FunctionDecl::param_const_iterator i = d->param_begin();
        i != d->param_end(); i++) {
        fd->add_field(this, *i);
    }

    return fd;
}

Decl* C2FFIASTConsumer::make_decl(const clang::VarDecl *d, bool is_toplevel) {
    clang::ASTContext &ctx = _ci.getASTContext();
    clang::APValue *v = NULL;
    std::string name = d->getDeclName().getAsString(),
                value = "";
    bool is_string = false;

    if(name.substr(0, 8) == "__c2ffi_")
        name = name.substr(8, std::string::npos);

    if(d->hasInit() && ((v = d->evaluateValue()) ||
                        (v = d->getEvaluatedValue()))) {
        if(v->isLValue()) {
            clang::APValue::LValueBase base = v->getLValueBase();
            const clang::Expr *e = base.get<const clang::Expr*>();

            if_const_cast(s, clang::StringLiteral, e) {
                value = s->getString();
                is_string = true;
            }
        } else {
            value = value_to_string(v);
        }
    }

    Type *t = Type::make_type(this, d->getTypeSourceInfo()->getType().getTypePtr());
    return new VarDecl(name, t, value, d->hasExternalStorage(), is_string);
}

Decl* C2FFIASTConsumer::make_decl(const clang::RecordDecl *d, bool is_toplevel) {
    std::string name = d->getDeclName().getAsString();
    clang::ASTContext &ctx = _ci.getASTContext();
    const clang::Type *t = d->getTypeForDecl();

    if(is_toplevel && name == "") return NULL;

    _cur_decls.insert(d);
    RecordDecl *rd = new RecordDecl(name, d->isUnion());

    if(!t->isIncompleteType()) {
        rd->set_bit_size(ctx.getTypeSize(t));
        rd->set_bit_alignment(ctx.getTypeAlign(t));
    } else {
        rd->set_bit_size(0);
        rd->set_bit_alignment(0);
    }

    if(name == "") {
        _anon_decls[d] = _anon_id;
        rd->set_id(_anon_id);
        _anon_id++;
    }

    for(clang::RecordDecl::field_iterator i = d->field_begin();
        i != d->field_end(); i++)
        rd->add_field(this, *i);

    return rd;
}

Decl* C2FFIASTConsumer::make_decl(const clang::TypedefDecl *d, bool is_toplevel) {
    return new TypedefDecl(d->getDeclName().getAsString(),
                           Type::make_type(this, d->getTypeSourceInfo()->getType().getTypePtr()));
}

Decl* C2FFIASTConsumer::make_decl(const clang::EnumDecl *d, bool is_toplevel) {
    std::string name = d->getDeclName().getAsString();

    _cur_decls.insert(d);
    EnumDecl *decl = new EnumDecl(name);

    if(name == "") {
        _anon_decls[d] = _anon_id;
        decl->set_id(_anon_id);
        _anon_id++;
    }

    for(clang::EnumDecl::enumerator_iterator i = d->enumerator_begin();
        i != d->enumerator_end(); i++) {
        const clang::EnumConstantDecl *ecd = (*i);
        decl->add_field(ecd->getDeclName().getAsString(),
                        ecd->getInitVal().getLimitedValue());
    }

    return decl;
}

Decl* C2FFIASTConsumer::make_decl(const clang::ObjCInterfaceDecl *d, bool is_toplevel) {
    const clang::ObjCInterfaceDecl *super = d->getSuperClass();

    _cur_decls.insert(d);
    ObjCInterfaceDecl *r = new ObjCInterfaceDecl(d->getDeclName().getAsString(),
                                                 super ? super->getDeclName().getAsString() : "",
                                                 !d->hasDefinition());

    for(clang::ObjCInterfaceDecl::protocol_iterator i = d->protocol_begin();
        i != d->protocol_end(); i++)
        r->add_protocol((*i)->getDeclName().getAsString());

    for(clang::ObjCInterfaceDecl::ivar_iterator i = d->ivar_begin();
        i != d->ivar_end(); i++) {
        r->add_field(this, *i);
    }

    r->add_functions(this, d);
    return r;
}

Decl* C2FFIASTConsumer::make_decl(const clang::ObjCCategoryDecl *d, bool is_toplevel) {
    ObjCCategoryDecl *r = new ObjCCategoryDecl(d->getClassInterface()->getDeclName().getAsString(),
                                               d->getDeclName().getAsString());
    _cur_decls.insert(d);
    r->add_functions(this, d);
    return r;
}

Decl* C2FFIASTConsumer::make_decl(const clang::ObjCProtocolDecl *d, bool is_toplevel) {
    ObjCProtocolDecl *r = new ObjCProtocolDecl(d->getDeclName().getAsString());
    _cur_decls.insert(d);
    r->add_functions(this, d);
    return r;
}

unsigned int C2FFIASTConsumer::decl_id(const clang::Decl *d) const {
    ClangDeclIDMap::const_iterator it = _anon_decls.find(d);

    if(it != _anon_decls.end())
        return it->second;
    else
        return 0;
}
