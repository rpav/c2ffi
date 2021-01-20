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

#include <limits.h>

#include <getopt.h>
#include <sys/stat.h>

#include <llvm/Support/Host.h>

#include "c2ffi.h"
#include "c2ffi/opt.h"

static char short_opt[] = "I:i:D:M:o:hN:x:A:T:E";

enum {
    WITH_MACRO_DEFS = CHAR_MAX+1,
    DECLSPEC        = CHAR_MAX+2
};

static struct option options[] = {
    { "include",     required_argument, 0, 'I' },
    { "sys-include", required_argument, 0, 'i' },
    { "driver",      required_argument, 0, 'D' },
    { "help",        no_argument,       0, 'h' },
    { "macro-file",  required_argument, 0, 'M' },
    { "output",      required_argument, 0, 'o' },
    { "namespace",   required_argument, 0, 'N' },
    { "lang",        required_argument, 0, 'x' },
    { "arch",        required_argument, 0, 'A' },
    { "templates",   required_argument, 0, 'T' },
    { "std",         required_argument, 0, 'S' },
    { "with-macro-defs", no_argument,   0, WITH_MACRO_DEFS },
    { "declspec",    no_argument,       0, DECLSPEC        },
    { 0, 0, 0, 0 }
};

static void usage(void);
static c2ffi::OutputDriver* select_driver(std::string name, std::ostream *os);

clang::InputKind parseLang(std::string str) {
    using namespace clang;

    if(str == "c")      return InputKind(Language::C);
    if(str == "c++")    return InputKind(Language::CXX);
    if(str == "objc")   return InputKind(Language::ObjC);
    if(str == "objc++") return InputKind(Language::ObjCXX);

    exit(1);
}

clang::LangStandard::Kind parseStd(std::string std) {
#define LANGSTANDARD(ident, name, lang, desc, features) if(std == name) return clang::LangStandard::lang_##ident;
#include "clang/Basic/LangStandards.def"
    return clang::LangStandard::lang_unspecified;
}

clang::InputKind parseExtension(std::string file) {
    using namespace clang;

    std::string ext = file.substr(file.find_last_of('.')+1, std::string::npos);

    if(ext == "c")      return InputKind(Language::C);
    if(ext == "cpp" ||
       ext == "cxx" ||
       ext == "c++" ||
       ext == "hpp" ||
       ext == "hxx")    return InputKind(Language::CXX);
    if(ext == "m")      return InputKind(Language::ObjC);
    if(ext == "mm")     return InputKind(Language::ObjCXX);

    return InputKind(Language::C);
}

void c2ffi::process_args(config &config, int argc, char *argv[]) {
    int o, index;
    bool output_specified = false;
    std::ostream *os = &std::cout;

    for(;;) {
        o = getopt_long(argc, argv, short_opt, options, &index);

        if(o == -1)
            break;

        switch(o) {
            case 'M': {
                if(config.macro_output) {
                    std::cerr << "Error: You may only specify one macro file"
                              << std::endl;
                    exit(1);
                }

                std::ofstream *of = new std::ofstream;
                of->open(optarg);
                config.macro_output = of;
                break;
            }

            case 'o': {
                if(output_specified) {
                    std::cerr << "Error: You may only specify one output file"
                              << std::endl;
                    exit(1);
                }

                std::ofstream *of = new std::ofstream;
                of->open(optarg);
                os = of;
                output_specified = true;
                break;
            }

            case 'I':
                config.includes.push_back(optarg);
                break;

            case 'i':
                config.sys_includes.push_back(optarg);
                break;

            case 'D':
                if(config.od) {
                    std::cerr << "Error: you may only specify one output driver"
                              << std::endl;
                    exit(1);
                }
                config.od = select_driver(optarg, os);
                break;

            case 'N':
                config.to_namespace = optarg;
                break;

            case 'x':
                config.kind = parseLang(optarg);
                break;

            case 'A':
                config.arch = optarg;
                break;

            case 'T':
                if(config.template_output) {
                    std::cerr << "Error: you may only specify one template output file"
                              << std::endl;
                    exit(1);
                }

                config.template_output = new std::ofstream;
                config.template_output->open(optarg);
                break;

            case 'E':
                config.preprocess_only = true;
                break;

            case 'S':
                config.std = parseStd(optarg);
                if(config.std == clang::LangStandard::lang_unspecified) {
                    std::cerr << "Error: unknown standard specified, --std="
                              << optarg << std::endl;
                    exit(1);
                }
                break;

            case WITH_MACRO_DEFS:
                config.with_macro_defs = true;
                break;

            case DECLSPEC:
                config.declspec = true;
                break;

            case 'h':
                usage();
                exit(0);

            case '?':
            default:
                usage();
                exit(1);
        }
    }

    if(optind >= argc) {
        std::cerr << "Error: No file specified." << std::endl;
        usage();
        exit(1);
    } else {
        config.filename = std::string(argv[optind++]);
        if(config.kind.getLanguage() == clang::Language::Unknown)
            config.kind = parseExtension(config.filename);
    }

    struct stat buf;
    if(stat(config.filename.c_str(), &buf) < 0) {
        std::cerr << "Error: No such file: " << config.filename
                  << std::endl;
        exit(1);
    } else if(!S_ISREG(buf.st_mode)) {
        std::cerr << "Error: Not a regular file: " << config.filename
                  << std::endl;
        exit(1);
    }

    config.output = os;

    if(!config.od)
        config.od = OutputDrivers[0].fn(os);
    else
        config.od->set_os(os);
}

void usage(void) {
    using namespace c2ffi;
    using namespace std;

    cout <<
        "Usage: c2ffi [options ...] FILE\n"
        "\n"
        "Options:\n"
        "      -I, --include        Add a \"LOCAL\" include path\n"
        "      -i, --sys-include    Add a <system> include path\n"
        "      -D, --driver         Specify an output driver (default: "
         << OutputDrivers[0].name << ")\n"
        "\n"
        "      -o, --output         Specify an output file (default: stdout)\n"
        "      -M, --macro-file     Specify a file for macro definition output\n"
        "      --with-macro-defs    Also include #defines for macro definitions\n"
        "\n"
        "      -N, --namespace      Specify target namespace/package/etc\n"
        "\n"
        "      -A, --arch           Specify the target triple for LLVM\n"
        "                           (default: "
         << llvm::sys::getDefaultTargetTriple() << ")\n"
        "      -x, --lang           Specify language (c, c++, objc, objc++)\n"
        "      --std                Specify the standard (c99, c++0x, c++11, ...)\n"
        "\n"
        "      -E                   Preprocessed output only, a la clang -E\n"
        "\n"
        "      --declspec           Enable support for Microsoft __declspec extension\n"
        "\n"
        "Drivers: ";

    for(int i = 0;; i++) {
        if(!OutputDrivers[i].name) break;
        cout << OutputDrivers[i].name;
        if(OutputDrivers[i+1].name)
            cout << ", ";
    }

    cout << endl;
}

c2ffi::OutputDriver* select_driver(std::string name, std::ostream *os) {
    using namespace c2ffi;
    using namespace std;

    for(int i = 0;; i++) {
        if(!OutputDrivers[i].name) break;

        if(name == OutputDrivers[i].name)
            return OutputDrivers[i].fn(os);
    }

    cerr << "Error: Invalid output driver: " << name << endl;
    usage();
    exit(1);
}
