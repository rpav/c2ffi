# $Id: Makefile.am,v 1.1.1.1 2002/04/29 03:19:28 rpav Exp $

include $(top_srcdir)/make-extras

SUBDIRS = include test

# This list taken from the clang-tutorial stuff.  There should be a better
# way to get this.
CLANG_LIBS = -lclangTooling\
             -lclangFrontendTool\
             -lclangFrontend\
             -lclangDriver\
             -lclangSerialization\
             -lclangCodeGen\
             -lclangParse\
             -lclangSema\
             -lclangStaticAnalyzerFrontend\
             -lclangStaticAnalyzerCheckers\
             -lclangStaticAnalyzerCore\
             -lclangAnalysis\
             -lclangARCMigrate\
             -lclangRewriteFrontend\
             -lclangEdit\
             -lclangAST\
             -lclangLex\
             -lclangBasic

bin_PROGRAMS = c2ffi
c2ffi_CXXFLAGS = -fno-rtti $(PTHREAD_CFLAGS) $(LLVM_CXXFLAGS) -Wno-reorder
c2ffi_LDADD = $(PTHREAD_LIBS) $(LLVM_LDFLAGS) $(CLANG_LIBS) $(LLVM_LIBS) \
              $(LIBDL) $(LIBCURSES) $(LIBZ)

c2ffi_SOURCES = c2ffi.cpp init.cpp options.cpp \
		OutputDriver.cpp Decl.cpp Type.cpp AST.cpp Expr.cpp \
		Template.cpp \
		drivers/Sexp.cpp drivers/JSON.cpp drivers/Null.cpp
