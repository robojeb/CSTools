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
  MyASTConsumer(vector<Issue>& lineIssues, clang::SourceManager& sm) :
    Visitor(lineIssues, sm) {}

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

  MyASTConsumer consm{lineIssues, SourceMgr};

  ParseAST(TheCompInst.getPreprocessor(), &consm, TheCompInst.getASTContext());
}


VarCheckVisitor::VarCheckVisitor(vector<Issue>& lineIssues,
                                  clang::SourceManager& sm):
  lineIssues_{lineIssues},
  sm_{sm}
{
  //Nothing to do
}

bool VarCheckVisitor::VisitDecl(Decl *d)
{
  string type{d->getDeclKindName()};

  if (type == "Field") {
    checkField(d);
  } else if (type == "Var") {
    checkVariable(d);
  } else if (type == "CXXRecord") {
    checkClass(d);
  } else if (type == "CXXMethod" || type == "Function") {
    //Check method names
  } else if (type == "UsingDirective") {
    //We should do stuff if this is a header file
  }

  return true;
}

void VarCheckVisitor::checkField(clang::Decl* d)
{
  auto loc = getDeclLocation(d);

  FieldDecl* f = (FieldDecl*)d;
  string name = f->getName();
  boost::regex r{MEMBER_VAR};

  if(!boost::regex_match(name, r)) {
    lineIssues_.push_back(Issue(loc.first,loc.second,"Incorrect Field Name",
    "Field names should be in lower camel case with a trailing underscore.",
    WARNING));
  }
}

void VarCheckVisitor::checkVariable(clang::Decl* d)
{
  auto loc = getDeclLocation(d);

  VarDecl* v = (VarDecl*)d;

  string name = v->getName();
  if (v->isLocalVarDecl()) {
    boost::regex r{LOCAL_VAR};
    if (!boost::regex_match(name, r)){
      lineIssues_.push_back(Issue(loc.first, loc.second,
      "Incorrect Local Variable Name",
      "Local variable names should be in lower camel case.",
      WARNING));
    }
  } else if (v->isStaticDataMember()){
    //Get the type of the variable so we can check if it is constant
    clang::QualType type = v->getType();
    if (type.isConstQualified()) {
      //If it is constant and static it must conform to the CONST_VAR naming
      //convention
      boost::regex r{CONST_VAR};
      if (!boost::regex_match(name, r)) {
        lineIssues_.push_back(Issue(loc.first, loc.second,
        "Incorrect Static Constant Variable Name",
        "Static constant variables should be in all caps with underscores.",
        WARNING));
      }
    }
  }
}

void VarCheckVisitor::checkFunction(clang::Decl* d)
{
  auto loc = getDeclLocation(d);

  FunctionDecl* v = (FunctionDecl*)d;

  string name = v->getName();

  boost::regex r{LOCAL_VAR};

  if (!boost::regex_match(name, r)){
    lineIssues_.push_back(Issue(loc.first, loc.second,
    "Incorrect Function/Method name",
    "Functions and Methods should be in lower camel case.",
    WARNING));
  }
}

void VarCheckVisitor::checkClass(clang::Decl* d)
{
  auto loc = getDeclLocation(d);

  CXXRecordDecl* c = (CXXRecordDecl*)d;

  string name = c->getName();

  boost::regex r{CLASS_NAME};

  if(!boost::regex_match(name, r)){
    lineIssues_.push_back(Issue(loc.first, loc.second,
    "Incorrect Class/Struct name",
    "Classes and Structures should be named in upper camel case.",
    WARNING));
  }
}

pair<int, int> VarCheckVisitor::getDeclLocation(clang::Decl* d)
{
    clang::SourceLocation sl = d->getLocation();
    string s = sl.printToString(sm_);

    boost::regex r{"[.\\/a-zA-Z]*:([0-9]+):([0-9]+)"};

    boost::smatch m;

    if(boost::regex_match(s, m, r)){
      auto colM = m[1];
      auto rowM = m[2];

      int col = atoi(colM.str().c_str());
      int row = atoi(rowM.str().c_str());

      return pair<int, int>{col, row};
    } else {
      return pair<int, int>{-1, -1};
    }

}
