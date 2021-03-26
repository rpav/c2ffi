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
#include <memory>

#include <llvm/Support/Host.h>
#include <llvm/ADT/IntrusiveRefCntPtr.h>
#include <llvm/Option/Option.h>

#include <clang/Driver/Driver.h>
#include <clang/Driver/Compilation.h>
#include <clang/Driver/Tool.h>
#include <clang/Frontend/FrontendDiagnostic.h>
#include <clang/Basic/DiagnosticOptions.h>
#include <clang/Frontend/TextDiagnosticPrinter.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/FrontendTool/Utils.h>
#include <clang/Basic/TargetOptions.h>
#include <clang/Basic/TargetInfo.h>
#include <clang/Basic/FileManager.h>
#include <clang/Basic/SourceManager.h>
#include <clang/Lex/HeaderSearch.h>
#include <clang/Lex/Preprocessor.h>
#include <clang/Lex/PreprocessorOptions.h>
#include <clang/Basic/Diagnostic.h>
#include <clang/AST/ASTContext.h>
#include <clang/AST/ASTConsumer.h>
#include <clang/Parse/Parser.h>
#include <clang/Parse/ParseAST.h>

#include <sys/stat.h>

#include "c2ffi/init.h"
#include "c2ffi/opt.h"

using namespace c2ffi;

void c2ffi::add_include(clang::CompilerInstance &ci, const char *path, bool is_angled,
                        bool show_error) {
    struct stat buf{};
    if(stat(path, &buf) < 0 || !S_ISDIR(buf.st_mode)) {
        if(show_error) {
            std::cerr << "Error: Not a directory: ";
            if(is_angled)
                std::cerr << "-i ";
            else
                std::cerr << "-I ";

            std::cerr << path << std::endl;
            exit(1);
        }

        return;
    }

    auto &fm = ci.getFileManager();
    if (auto dirent = fm.getDirectoryRef(path); dirent) {
        clang::DirectoryLookup lookup(*dirent, clang::SrcMgr::C_System, false);

        ci.getPreprocessor().getHeaderSearchInfo()
            .AddSearchPath(lookup, is_angled);
    }
}

void c2ffi::add_includes(clang::CompilerInstance &ci,
                         c2ffi::IncludeVector &includeVector, bool is_angled,
                         bool show_error) {
    for(auto &&include : includeVector)
        add_include(ci, include.c_str(), is_angled, show_error);
}

void c2ffi::init_ci(config &c, clang::CompilerInstance &ci) {
    using clang::DiagnosticOptions;
    using clang::TextDiagnosticPrinter;
    using clang::TargetOptions;
    using clang::TargetInfo;
    using clang::IntrusiveRefCntPtr;
    using clang::CompilerInvocation;

    std::vector<const char *> cargs;
    cargs.push_back(c.c2ffi_binpath.c_str());
    cargs.push_back("-fsyntax-only");
    cargs.push_back("-resource-dir");
    cargs.push_back(CLANG_RESOURCE_DIRECTORY);
    if (c.nostdinc) {
        cargs.push_back("-nostdinc");
    }
    if (!c.lang.empty()) {
        cargs.push_back("-x");
        cargs.push_back(c.lang.c_str());
    }
    cargs.push_back(c.filename.c_str());

    IntrusiveRefCntPtr<DiagnosticOptions> DiagOpts = new DiagnosticOptions();
    TextDiagnosticPrinter *tpd =
        new TextDiagnosticPrinter(llvm::errs(), &*DiagOpts, false);
    IntrusiveRefCntPtr<clang::DiagnosticIDs> DiagID(
        new clang::DiagnosticIDs());
    clang::DiagnosticsEngine Diags(DiagID, &*DiagOpts, tpd);

    clang::driver::Driver Driver(
        c.c2ffi_binpath,
        c.arch.empty() ? llvm::sys::getDefaultTargetTriple() : c.arch, Diags);
    Driver.setCheckInputsExist(false);

    std::unique_ptr<clang::driver::Compilation> C(Driver.BuildCompilation(cargs));
    const clang::driver::JobList &Jobs = C->getJobs();
    if (Jobs.size() != 1) {
        Diags.Report(clang::diag::err_fe_expected_compiler_job);
        exit(1);
    }

    const clang::driver::Command &Cmd = clang::cast<clang::driver::Command>(*Jobs.begin());
    if (llvm::StringRef(Cmd.getCreator().getName()) != "clang") {
        Diags.Report(clang::diag::err_fe_expected_clang_command);
        exit(1);
    }

    static std::unique_ptr<clang::CompilerInvocation> cinv;
    cinv = std::make_unique<CompilerInvocation>();
    CompilerInvocation::CreateFromArgs(*cinv, Cmd.getArguments(), Diags);
    if (c.nostdinc) {
        // setting -nostdinc isn't sufficient for some reason, this erases all
        // the search paths that were added previously.
        cinv->getHeaderSearchOpts().UserEntries.clear();
    }

    ci.setInvocation(std::move(cinv));

    // Extract the language that was inferred or specified for the input file.
    auto &fInputs = ci.getInvocation().getFrontendOpts().Inputs;
    if (fInputs.size() != 1) {
        std::cout << "Error: No input files from frontend" << std::endl;
        exit(1);
    } else {
        c.kind = fInputs[0].getKind();
        switch (c.kind.getLanguage()) {
        case clang::Language::C:
        case clang::Language::CXX:
        case clang::Language::ObjC:
        case clang::Language::ObjCXX:
            break;
        default:
            std::cerr << "Error: Language " << (c.lang.empty() ? "of file " + c.filename : c.lang)
                      << " not supported." << std::endl;
            exit(1);
        }
    }

    // Create the compilers actual diagnostics engine.
    ci.createDiagnostics();
    ci.getDiagnostics().setWarningsAsErrors(c.warn_as_error);
    ci.getDiagnostics().setErrorLimit(c.error_limit);

    TargetInfo *pti = TargetInfo::CreateTargetInfo(
        ci.getDiagnostics(), ci.getInvocation().TargetOpts);
    ci.setTarget(pti);

    clang::LangOptions &lo = ci.getLangOpts();
    switch(pti->getTriple().getEnvironment()) {
        case llvm::Triple::EnvironmentType::GNU:
            lo.GNUMode = 1;
            break;
        case llvm::Triple::EnvironmentType::MSVC:
            lo.MSVCCompat = 1;
            lo.MicrosoftExt = 1;
            break;
        default:
            std::cerr << "c2ffi warning: Unhandled environment: '"
                      << pti->getTriple().getEnvironmentName().str()
                      << "' for triple '" << c.arch
                      << "'" << std::endl;
    }

    if(c.declspec)
        lo.DeclSpecKeyword = 1;

    if(c.wchar_size != 0)
        lo.WCharSize = c.wchar_size;

    clang::PreprocessorOptions preopts;
    ci.getInvocation().setLangDefaults(lo, c.kind, pti->getTriple(), preopts, c.std);
    ci.createFileManager();
    ci.createSourceManager(ci.getFileManager());

    // examples/clang-interpreter/main.cpp
    // Infer the builtin include path if unspecified.
    clang::HeaderSearchOptions &hso = ci.getHeaderSearchOpts();
    if (!c.nostdinc && hso.ResourceDir.empty())
        hso.ResourceDir = CLANG_RESOURCE_DIRECTORY;
    ci.createPreprocessor(clang::TU_Complete);
    ci.getPreprocessorOpts().UsePredefines = false;
    ci.getPreprocessorOutputOpts().ShowCPP = c.preprocess_only;
    ci.getPreprocessor().setPreprocessedOutput(c.preprocess_only);
}
