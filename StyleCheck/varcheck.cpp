#include "varcheck.hpp"
#include "severity.hpp"
#include "issue.hpp"
#include <iostream>

using namespace std;
using namespace clang;

// Implementation of the ASTConsumer interface for reading an AST produced
// by the Clang parser.
class MyASTConsumer : public ASTConsumer {
public:
  MyASTConsumer(vector<Issue>& lineIssues) :
    Visitor(lineIssues) {}

  // Override the method that gets called for each parsed top-level
  // declaration.
  virtual bool HandleTopLevelDecl(DeclGroupRef DR) {
    for (DeclGroupRef::iterator b = DR.begin(), e = DR.end(); b != e; ++b)
      // Traverse the declaration using our AST visitor.
      Visitor.TraverseDecl(*b);
    return true;
  }

private:
  VarCheckVisitor Visitor;
};

void checkVariables(string filename, vector<Issue>& lineIssues){
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

  // Set the main file handled by the source manager to the input file.
  const FileEntry *FileIn = FileMgr.getFile(filename);
  SourceMgr.setMainFileID(
      SourceMgr.createFileID(FileIn, SourceLocation(), SrcMgr::C_User));
  TheCompInst.getDiagnosticClient().BeginSourceFile(
      TheCompInst.getLangOpts(), &TheCompInst.getPreprocessor());

  MyASTConsumer consm{lineIssues};

  ParseAST(TheCompInst.getPreprocessor(), &consm, TheCompInst.getASTContext());
}


VarCheckVisitor::VarCheckVisitor(vector<Issue>& lineIssues):
  lineIssues_{lineIssues}
{
  //Nothing to do
}

bool VarCheckVisitor::VisitVarDecl(VarDecl *v)
{
  string name = v->getName();
  if (v->isLocalVarDecl()) {
    boost::regex r{LOCAL_VAR};
    if (!boost::regex_match(name, r)){
      lineIssues_.push_back(Issue(-1,-1,"Incorrect Local Variable Name",
      "Local variable names should be in lower camel case.",
      WARNING));
    }
  } else if (v->isStaticDataMember()){
    //Nothing to do unless we can figure out how to find constness
  }
  return true;
}

bool VarCheckVisitor::VisitDecl(Decl *d)
{
  string type{d->getDeclKindName()};
  if (type == "Field") {
    FieldDecl* f = (FieldDecl*)d;
    string name = f->getName();
    boost::regex r{MEMBER_VAR};

    if(!boost::regex_match(name, r)) {
      lineIssues_.push_back(Issue(-1,-1,"Incorrect Field Name",
      "Field names should be in lower camel case with a trailing underscore.",
      WARNING));
    }
  }

  return true;
}
