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

#ifndef C2FFI_TEMPLATE_H
#define C2FFI_TEMPLATE_H

#include <string>
#include <vector>

#include <clang/AST/TemplateBase.h>
#include <clang/AST/DeclTemplate.h>
#include <clang/Frontend/CompilerInstance.h>

namespace c2ffi {
    class C2FFIASTConsumer;

    class TemplateArg {
        Type *_type;
        bool _has_val;
        std::string _val;

    public:
        TemplateArg(C2FFIASTConsumer *ast,
                    const clang::TemplateArgument &arg);
        bool has_val() const { return _has_val; }
        const Type* type() const { return _type; }
        const std::string& val() const { return _val; }
    };

    typedef std::vector<TemplateArg*> TemplateArgVector;

    class TemplateMixin {
        bool _is_template;
        TemplateArgVector _args;

    public:
        TemplateMixin(C2FFIASTConsumer *ast,
                      const clang::TemplateArgumentList *arglist);

        const TemplateArgVector& args() const { return _args; }
        bool is_template() const { return _is_template; }
    };
}

#endif // C2FFI_TEMPLATE_H
