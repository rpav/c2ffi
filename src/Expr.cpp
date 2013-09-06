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

#include <clang/Lex/Token.h>
#include <clang/Lex/MacroInfo.h>
#include <clang/Lex/Preprocessor.h>
#include <clang/Lex/LiteralSupport.h>

#include <set>
#include <string>
#include <sstream>
#include <map>

#include "c2ffi.h"
#include "c2ffi/macros.h"

typedef std::set<std::string> StringSet;
typedef std::map<std::string, std::string> RedefMap;

enum best_guess {
    tok_invalid = 0,
    tok_ok = 1,
    tok_int,
    tok_unsigned,
    tok_long_long,
    tok_unsigned_long_long,
    tok_float,
    tok_string
};

static best_guess macro_type(clang::Preprocessor &pp,
                             const char *macro_name,
                             const clang::MacroInfo *mi,
                             StringSet *seen = NULL);

static best_guess num_type(clang::Preprocessor &pp,
                           const clang::Token &t) {
    llvm::StringRef sr(t.getLiteralData(), t.getLength());
    clang::NumericLiteralParser parser(sr, t.getLocation(), pp);

    if(parser.isIntegerLiteral()) {
        if(parser.isUnsigned)
            if(parser.isLongLong)
                return tok_unsigned_long_long;
            else
                return tok_unsigned;
        else
            if(parser.isLongLong)
                return tok_long_long;
            else
                return tok_int;
    }
    if(parser.isFloatingLiteral())
        return tok_float;

    return tok_invalid;
}

static best_guess tok_type(clang::Preprocessor &pp, const char *macro_name,
                           const clang::Token &t, StringSet *seen) {
    using namespace clang;
    tok::TokenKind k = t.getKind();

    if(k == tok::identifier) {
        return tok_ok;

        IdentifierInfo *ii = t.getIdentifierInfo();
        if(ii && !seen->count(ii->getNameStart()))
            return macro_type(pp, ii->getNameStart(),
                              pp.getMacroInfo(ii), seen);

        return tok_invalid;
    }

    if (k == tok::l_paren || k == tok::r_paren || k == tok::amp || k == tok::plus ||
        k == tok::star || k == tok::minus || k == tok::tilde || k == tok::slash ||
        k == tok::percent || k == tok::lessless || k == tok::greatergreater ||
        k == tok::caret || k == tok::pipe || k == tok::kw_int || k == tok::kw_float ||
        k == tok::kw_double || k == tok::kw_long || k == tok::kw_signed ||
        k == tok::kw_unsigned)
        return tok_ok;

    return tok_invalid;
}

static best_guess macro_type(clang::Preprocessor &pp, const char *macro_name,
                             const clang::MacroInfo *mi, StringSet *seen) {
    if(!mi) return tok_invalid;
    best_guess result = tok_invalid, guess = tok_invalid;

    bool owns_seen = (seen == NULL);
    if(owns_seen) seen = new StringSet;
    seen->insert(macro_name);

    for(clang::MacroInfo::tokens_iterator j = mi->tokens_begin();
        j != mi->tokens_end(); j++) {
        const clang::Token &t = (*j);

        if(t.isLiteral()) {
            if(t.getKind() == clang::tok::numeric_constant)
                guess = num_type(pp, t);
            else if(t.getKind() == clang::tok::string_literal)
                guess = tok_string;
            else {
                result = tok_invalid;
                goto end;
            }

            if(guess > result) result = guess;
        } else {
            guess = tok_type(pp, macro_name, t, seen);
            if(guess == tok_invalid) goto end;
            if(guess > result) result = guess;
        }
    }

 end:
    if(owns_seen) delete seen;

    // Not much good if we don't know a type
    if(result <= tok_ok)
        return tok_invalid;

    return result;
}

static std::string macro_to_string(const clang::Preprocessor &pp,
                                   const clang::MacroInfo *mi) {
    std::stringstream ss;

    for(clang::MacroInfo::tokens_iterator j = mi->tokens_begin();
        j != mi->tokens_end(); j++) {
        const clang::Token &t = (*j);

        if(t.getFlags() & clang::Token::LeadingSpace)
            ss << " ";

        ss << pp.getSpelling(t);
    }

    return ss.str();
}

static void
output_redef(clang::Preprocessor &pp, const char *name, const clang::MacroInfo *mi,
             const best_guess type, std::ostream &os) {
    using namespace c2ffi;

    os << "const ";

    switch(type) {
        case tok_unsigned_long_long: os << "unsigned long long"; break;
        case tok_long_long: os << "long long"; break;
        case tok_unsigned: os << "unsigned long"; break;
        case tok_int: os << "long"; break;
        case tok_float: os << "double"; break;
        case tok_string:
        default:
            os << "char*"; break;
    }

    os << " __c2ffi_" << name << " = " << name << ";" << std::endl;
}

void c2ffi::process_macros(clang::CompilerInstance &ci, std::ostream &os) {
    using namespace c2ffi;

    clang::SourceManager &sm = ci.getSourceManager();
    clang::Preprocessor &pp = ci.getPreprocessor();

    for(clang::Preprocessor::macro_iterator i = pp.macro_begin();
        i != pp.macro_end(); i++) {
        const clang::MacroInfo *mi = (*i).second->getMacroInfo();
        const clang::SourceLocation sl = mi->getDefinitionLoc();
        std::string loc = sl.printToString(sm);
        const char *name = (*i).first->getNameStart();

        if(mi->isBuiltinMacro() || loc.substr(0,10) == "<built-in>") {
        } else if(mi->isFunctionLike()) {
        } else if(best_guess type = macro_type(pp, name, mi)) {
            os << "/* " << loc << " */" << std::endl;
            os << "#define " << name << " "
                      << macro_to_string(pp, mi)
                      << std::endl << std::endl;
        }
    }


    for(clang::Preprocessor::macro_iterator i = pp.macro_begin();
        i != pp.macro_end(); i++) {
        clang::MacroInfo *mi = (*i).second->getMacroInfo();
        clang::SourceLocation sl = mi->getDefinitionLoc();
        std::string loc = sl.printToString(sm);
        const char *name = (*i).first->getNameStart();

        if(mi->isBuiltinMacro() || loc.substr(0,10) == "<built-in>") {
        } else if(mi->isFunctionLike()) {
        } else if(best_guess type = macro_type(pp, name, mi)) {
            output_redef(pp, name, mi, type, os);
        }
    }
}
