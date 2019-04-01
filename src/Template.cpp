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

#include <sstream>

#include "c2ffi.h"
#include "c2ffi/ast.h"

using namespace c2ffi;
using namespace std;

TemplateArg::TemplateArg(C2FFIASTConsumer* ast, const clang::TemplateArgument& arg)
    : _type(NULL), _has_val(false), _val("")
{

    if(arg.getKind() == clang::TemplateArgument::Type)
        _type = Type::make_type(ast, arg.getAsType().getTypePtrOrNull());
    else if(arg.getKind() == clang::TemplateArgument::Integral) {
        _has_val = true;
        _val     = arg.getAsIntegral().toString(10);
        _type    = Type::make_type(ast, arg.getIntegralType().getTypePtrOrNull());
    } else if(arg.getKind() == clang::TemplateArgument::Declaration) {
        _has_val = true;
        _val     = arg.getAsDecl()->getNameAsString();
        _type    = Type::make_type(ast, arg.getAsDecl()->getType().getTypePtrOrNull());
    } else if(arg.getKind() == clang::TemplateArgument::Expression) {
        const clang::ASTContext& ctx  = ast->ci().getASTContext();
        const clang::Expr*       expr = arg.getAsExpr();

        _type = Type::make_type(ast, expr->getType().getTypePtrOrNull());

        if(expr->isEvaluatable(ctx)) {
            clang::Expr::EvalResult r;
            expr->EvaluateAsInt(r, ctx);

            if(r.Val.isInt()) {
                _has_val = true;
                _val     = r.Val.getInt().toString(10);
            }
        }
    } else {
        std::stringstream ss;
        ss << "<unknown:" << arg.getKind() << ">";
        _type = new SimpleType(ast->ci(), NULL, ss.str());
    }
}

TemplateMixin::TemplateMixin(C2FFIASTConsumer* ast, const clang::TemplateArgumentList* arglist)
    : _is_template(false)
{

    if(arglist == NULL) return;

    _is_template = true;

    for(int i = 0; i < arglist->size(); i++) _args.push_back(new TemplateArg(ast, (*arglist)[i]));
}

void C2FFIASTConsumer::write_template(
    const clang::ClassTemplateSpecializationDecl* d,
    std::ofstream&                                out)
{
    using namespace std;

    out << "template ";

    if(d->isUnion())
        out << "union ";
    else if(d->isClass())
        out << "class ";
    else
        out << "struct ";

    out << d->getNameAsString() << "<";

    const clang::TemplateArgumentList& arglist = d->getTemplateInstantiationArgs();

    for(int i = 0; i < arglist.size(); i++) {
        if(i > 0) out << ", ";

        const clang::TemplateArgument& arg = arglist[i];

        if(arg.getKind() == clang::TemplateArgument::Type)
            out << arg.getAsType().getAsString();
        else if(arg.getKind() == clang::TemplateArgument::Integral) {
            out << arg.getAsIntegral().toString(10);
        } else if(arg.getKind() == clang::TemplateArgument::Declaration) {
            out << arg.getAsDecl()->getNameAsString();
        } else if(arg.getKind() == clang::TemplateArgument::Expression) {
            const clang::ASTContext& ctx  = _ci.getASTContext();
            const clang::Expr*       expr = arg.getAsExpr();

            if(expr->isEvaluatable(ctx)) {
                clang::Expr::EvalResult r;
                expr->EvaluateAsInt(r, ctx);
                if(r.Val.isInt())
                    out << r.Val.getInt().toString(10);
            }
        } else {
            out << "?" << arg.getKind() << "?";
        }
    }

    out << ">;" << endl;
}
