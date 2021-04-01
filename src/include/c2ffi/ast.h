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
#include "c2ffi/opt.h"

#define if_cast(v,T,e) if(T *v = llvm::dyn_cast<T>((e)))
#define if_const_cast(v,T,e) if(const T *v = llvm::dyn_cast<T>((e)))

namespace c2ffi {
    typedef std::set<const clang::Decl*> ClangDeclSet;
    typedef std::map<const clang::Decl*, int> ClangDeclIDMap;

    class C2FFIASTConsumer : public clang::ASTConsumer {
        config &_config;

        clang::CompilerInstance &_ci;
        c2ffi::OutputDriver *_od;
        bool _mid;

        ClangDeclSet _cur_decls;
        ClangDeclIDMap _decl_map;
        unsigned int _decl_id;

        ClangDeclSet _cxx_decls;

        const clang::NamedDecl *_ns;

    public:
        C2FFIASTConsumer(clang::CompilerInstance &ci, config &config)
            : _config(config), _ci(ci), _od(config.od), _mid(false), _decl_id(0), _ns() { }

        clang::CompilerInstance& ci() { return _ci; }
        c2ffi::OutputDriver& od() { return *_od; }

        virtual bool HandleTopLevelDecl(clang::DeclGroupRef d);
        virtual void HandleTopLevelDeclInObjCContainer(clang::DeclGroupRef d);

        void HandleDecl(clang::Decl *d, const clang::NamedDecl *ns = NULL);
        void HandleDeclContext(const clang::DeclContext *dc,
                               const clang::NamedDecl *ns);
        void HandleNS(const clang::NamespaceDecl *ns);
        void PostProcess(std::ostream& out);

        Decl* proc(const clang::Decl*, Decl*);

        bool is_cur_decl(const clang::Decl *d) const;
        unsigned int decl_id(const clang::Decl *d) const;
        unsigned int add_decl(const clang::Decl *d) {
            if(!d) {
                return 0;
            } else if(!_decl_map.count(d)) {
                _decl_map[d] = ++_decl_id;
                return _decl_id;
            } else {
                return _decl_map[d];
            }
        }
        unsigned int add_cxx_decl(const clang::Decl *d) {
            if(d) {
                _cxx_decls.insert(d);
                return add_decl(d);
            }

            return 0;
        }

        const clang::NamedDecl* ns() const { return _ns; }

        Decl* make_decl(const clang::Decl *d, bool is_toplevel = true);
        Decl* make_decl(const clang::NamedDecl *d, bool is_toplevel = true);
        Decl* make_decl(const clang::FunctionDecl *d, bool is_toplevel = true);
        Decl* make_decl(const clang::VarDecl *d, bool is_toplevel = true);
        Decl* make_decl(const clang::RecordDecl *d, bool is_toplevel = true);
        Decl* make_decl(const clang::TypedefDecl *d, bool is_toplevel = true);
        Decl* make_decl(const clang::EnumDecl *d, bool is_toplevel = true);
        Decl* make_decl(const clang::CXXRecordDecl *d, bool is_toplevel = true);
        Decl* make_decl(const clang::NamespaceDecl *d, bool is_toplevel = true);
        Decl* make_decl(const clang::ObjCInterfaceDecl *d, bool is_toplevel = true);
        Decl* make_decl(const clang::ObjCCategoryDecl *d, bool is_toplevel = true);
        Decl* make_decl(const clang::ObjCProtocolDecl *d, bool is_toplevel = true);

        void write_template(const clang::ClassTemplateSpecializationDecl *d,
                            std::ostream &out);
    };
}

#endif /* C2FFI_AST_H */
