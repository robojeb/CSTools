LLVM_SRC_PATH := $$HOME/clang-svn/llvm
LLVM_BUILD_PATH := $$HOME/clang-svn/build
LLVM_BIN_PATH := $(LLVM_BUILD_PATH)/Debug+Asserts/bin

$(info -----------------------------------------------)
$(info Using LLVM_SRC_PATH = $(LLVM_SRC_PATH))
$(info Using LLVM_BUILD_PATH = $(LLVM_BUILD_PATH))
$(info Using LLVM_BIN_PATH = $(LLVM_BIN_PATH))
$(info -----------------------------------------------)

CLANG_INCLUDES := \
  -I$(LLVM_SRC_PATH)/tools/clang/include \
  -I$(LLVM_BUILD_PATH)/tools/clang/include
#	-I$(LLVM_SRC_PATH)/include \
#	-I$(LLVM_BUILD_PATH)/include

LLVM_CXXFLAGS := `$(LLVM_BIN_PATH)/llvm-config --cxxflags`
LLVM_LDFLAGS := `$(LLVM_BIN_PATH)/llvm-config --ldflags --libs --system-libs`
LLVM_LDFLAGS_NOLIBS := `$(LLVM_BIN_PATH)/llvm-config --ldflags`
BOOST_LIBS := -lboost_regex

CLANG_LIBS := \
  -lclangFrontendTool \
  -lclangFrontend \
  -lclangDriver \
  -lclangSerialization \
  -lclangCodeGen \
  -lclangParse \
  -lclangSema \
  -lclangRewriteFrontend \
  -lclangRewrite \
  -lclangStaticAnalyzerFrontend \
  -lclangStaticAnalyzerCheckers \
  -lclangStaticAnalyzerCore \
  -lclangARCMigrate \
  -lclangAnalysis \
  -lclangEdit \
  -lclangAST \
  -lclangLex \
  -lclangBasic

CXX := clang++
CXXFLAGS := -fno-rtti -O0 -std=c++11

all: stylecheck

stylecheck: stylecheck.o varcheck.o issue.o
	$(CXX) $^ $(CLANG_LIBS) $(BOOST_LIBS) $(LLVM_LDFLAGS) -o $@

issue.o: issue.cpp issue.hpp
	$(CXX) $(CXXFLAGS) -c $(LLVM_CXXFLAGS) $(CLANG_INCLUDES) $< -o $@

stylecheck.o: stylecheck.cpp issue.hpp varcheck.hpp
	$(CXX) $(CXXFLAGS) -c $(LLVM_CXXFLAGS) $(CLANG_INCLUDES) $< -o $@

varcheck.o: varcheck.cpp varcheck.hpp issue.hpp
	$(CXX) $(CXXFLAGS) -c $(LLVM_CXXFLAGS) $(CLANG_INCLUDES) $< -o $@

clean:
	rm stylecheck *.o
