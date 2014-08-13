LLVM_SRC_PATH := $$HOME/Sources/llvm
LLVM_BUILD_PATH := $$HOME/Sources/build
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

CLANG_LIBS := \
	-lclangAST \
	-lclangAnalysis \
	-lclangBasic \
	-lclangDriver \
	-lclangEdit \
	-lclangFrontend \
	-lclangFrontendTool \
	-lclangLex \
	-lclangParse \
	-lclangSema \
	-lclangEdit \
	-lclangASTMatchers \
	-lclangRewrite \
	-lclangRewriteFrontend \
	-lclangStaticAnalyzerFrontend \
	-lclangStaticAnalyzerCheckers \
	-lclangStaticAnalyzerCore \
	-lclangSerialization \
	-lclangTooling

CXX := clang++
CXXFLAGS := -fno-rtti -O0 -std=c++11

all: replacer

replacer: replacer.o extractor.o
	$(CXX) $^ $(CLANG_LIBS) $(LLVM_LDFLAGS) -o $@

replacer.o: replacer.cpp extractor.hpp
	$(CXX) $(CXXFLAGS) -c $(LLVM_CXXFLAGS) $(CLANG_INCLUDES) $< -o $@

extractor.o: extractor.cpp extractor.hpp
	$(CXX) $(CXXFLAGS) -c $(LLVM_CXXFLAGS) $(CLANG_INCLUDES) $< -o $@

clean:
	rm replacer extractor replacer.o extractor.o
