#include <cstdio>
#include <memory>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>

#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Basic/Diagnostic.h"
#include "clang/Basic/FileManager.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Basic/TargetOptions.h"
#include "clang/Basic/TargetInfo.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/CompilerInvocation.h"
#include "clang/Lex/Preprocessor.h"
#include "clang/Parse/ParseAST.h"
#include "clang/Rewrite/Core/Rewriter.h"
#include "clang/Rewrite/Frontend/Rewriters.h"
#include "llvm/Support/Host.h"
#include "llvm/Support/raw_ostream.h"

#include "extractor.hpp"

using namespace clang;


// By implementing RecursiveASTVisitor, we can specify which AST nodes
// we're interested in by overriding relevant methods.
class MyASTVisitor : public RecursiveASTVisitor<MyASTVisitor> {
public:
  MyASTVisitor(Rewriter &R, std::vector<std::string> des, std::string repFile) :
    TheRewriter(R),
    DesiredFunctions(des),
    ReplacementFile(repFile) {}

  bool VisitFunctionDecl(FunctionDecl *f) {
    // Only function definitions (with bodies), not declarations.
    if (f->hasBody()) {

      std::string FuncName = f->getNameInfo().getName().getAsString();
      for (std::string desired : DesiredFunctions){
        if (FuncName == desired) {
            SourceExtractor se{ReplacementFile, desired};
            SourceLocation ST = f->getSourceRange().getBegin();
            TheRewriter.ReplaceText(f->getSourceRange(), se.getExtractedSource());
        }
      }
    }

    return true;
  }

private:
  Rewriter &TheRewriter;
  std::vector<std::string> DesiredFunctions;
  std::string ReplacementFile;
};

// Implementation of the ASTConsumer interface for reading an AST produced
// by the Clang parser.
class MyASTConsumer : public ASTConsumer {
public:
  MyASTConsumer(Rewriter &R, std::vector<std::string> des, std::string repFile) :
    Visitor(R, des, repFile) {}

  // Override the method that gets called for each parsed top-level
  // declaration.
  virtual bool HandleTopLevelDecl(DeclGroupRef DR) {
    for (DeclGroupRef::iterator b = DR.begin(), e = DR.end(); b != e; ++b)
      // Traverse the declaration using our AST visitor.
      Visitor.TraverseDecl(*b);
    return true;
  }

private:
  MyASTVisitor Visitor;
};

int main(int argc, char *argv[]) {
  if (argc < 4) {
    llvm::errs() << "Usage: replacer <initial file> <replacement file> <functions...>\n";
    return 1;
  }

  std::vector<std::string> desiredFunctions;

  for (int i = 3; i < argc; i++){
    desiredFunctions.push_back(argv[i]);
  }

  std::string ReplacementFile(argv[2]);

  // CompilerInstance will hold the instance of the Clang compiler for us,
  // managing the various objects needed to run the compiler.
  CompilerInstance TheCompInst;
  TheCompInst.createDiagnostics();

  CompilerInvocation TheCompInv;

  LangOptions &lo = TheCompInst.getLangOpts();
  lo.CPlusPlus = 1;
  lo.Bool = 1;
  lo.WChar = 1;
  lo.NoBuiltin = 0;

  // Initialize target info with the default triple for our platform.
  auto TO = std::make_shared<TargetOptions>();
  TO->Triple = llvm::sys::getDefaultTargetTriple();
  TargetInfo *TI =
      TargetInfo::CreateTargetInfo(TheCompInst.getDiagnostics(), TO);
  TheCompInst.setTarget(TI);

  TheCompInst.createFileManager();
  FileManager &FileMgr = TheCompInst.getFileManager();
  TheCompInst.createSourceManager(FileMgr);
  SourceManager &SourceMgr = TheCompInst.getSourceManager();
  TheCompInst.createPreprocessor(TU_Module);

  Preprocessor& pp = TheCompInst.getPreprocessor();

  pp.getBuiltinInfo().InitializeBuiltins(pp.getIdentifierTable(), lo);
                                          //pp.getLangOpts().NoBuiltin);

  TheCompInst.createASTContext();

  // A Rewriter helps us manage the code rewriting task.
  Rewriter TheRewriter;
  TheRewriter.setSourceMgr(SourceMgr, TheCompInst.getLangOpts());

  // Set the main file handled by the source manager to the input file.
  const FileEntry *FileIn = FileMgr.getFile(argv[1]);
  SourceMgr.setMainFileID(
      SourceMgr.createFileID(FileIn, SourceLocation(), SrcMgr::C_User));
  TheCompInst.getDiagnosticClient().BeginSourceFile(
      TheCompInst.getLangOpts(), &TheCompInst.getPreprocessor());

  // Create an AST consumer instance which is going to get called by
  // ParseAST.
  MyASTConsumer TheConsumer(TheRewriter, desiredFunctions, ReplacementFile);

  // Parse the file to AST, registering our consumer as the AST consumer.
  ParseAST(TheCompInst.getPreprocessor(), &TheConsumer,
           TheCompInst.getASTContext());

  // At this point the rewriter's buffer should be full with the rewritten
  // file contents.
  const RewriteBuffer *RewriteBuf =
      TheRewriter.getRewriteBufferFor(SourceMgr.getMainFileID());
  llvm::outs() << std::string(RewriteBuf->begin(), RewriteBuf->end());

  return 0;
}
