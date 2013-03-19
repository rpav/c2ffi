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

#ifndef C2FFI_AST_H
#define C2FFI_AST_H

#include <set>
#include <map>
#include <clang/AST/ASTConsumer.h>
#include "c2ffi.h"

#define if_cast(v,T,e) if(T *v = llvm::dyn_cast<T>((e)))
#define if_const_cast(v,T,e) if(const T *v = llvm::dyn_cast<T>((e)))

namespace c2ffi {
    typedef std::set<const clang::Decl*> ClangDeclSet;
    typedef std::map<const clang::Decl*, int> ClangDeclIDMap;

    class C2FFIASTConsumer : public clang::ASTConsumer {
        clang::CompilerInstance &_ci;
        c2ffi::OutputDriver *_od;
        bool _mid;

        ClangDeclSet _cur_decls;
        ClangDeclIDMap _anon_decls;
        unsigned int _anon_id;

    public:
        C2FFIASTConsumer(clang::CompilerInstance &ci, c2ffi::OutputDriver *od)
            : _ci(ci), _od(od), _mid(false), _anon_id(1) { }

        clang::CompilerInstance& ci() { return _ci; }
        c2ffi::OutputDriver& od() { return *_od; }

        virtual bool HandleTopLevelDecl(clang::DeclGroupRef d);
        virtual void HandleTopLevelDeclInObjCContainer(clang::DeclGroupRef d);

        bool is_cur_decl(const clang::Decl *d) const;
        unsigned int decl_id(const clang::Decl *d) const;

        Decl* make_decl(const clang::Decl *d, bool is_toplevel = true);
        Decl* make_decl(const clang::NamedDecl *d, bool is_toplevel = true);
        Decl* make_decl(const clang::FunctionDecl *d, bool is_toplevel = true);
        Decl* make_decl(const clang::VarDecl *d, bool is_toplevel = true);
        Decl* make_decl(const clang::RecordDecl *d, bool is_toplevel = true);
        Decl* make_decl(const clang::TypedefDecl *d, bool is_toplevel = true);
        Decl* make_decl(const clang::EnumDecl *d, bool is_toplevel = true);
        Decl* make_decl(const clang::ObjCInterfaceDecl *d, bool is_toplevel = true);
        Decl* make_decl(const clang::ObjCCategoryDecl *d, bool is_toplevel = true);
        Decl* make_decl(const clang::ObjCProtocolDecl *d, bool is_toplevel = true);
    };
}

#endif /* C2FFI_AST_H */
