#ifndef VARCHECK_HPP_INC
#define VARCHECK_HPP_INC

#include <string>
#include <vector>

#include "issue.hpp"

#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Basic/Diagnostic.h"
#include "clang/Basic/FileManager.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Basic/TargetOptions.h"
#include "clang/Basic/TargetInfo.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Lex/Preprocessor.h"
#include "clang/Parse/ParseAST.h"
#include "clang/Rewrite/Core/Rewriter.h"
#include "clang/Rewrite/Frontend/Rewriters.h"
#include "llvm/Support/Host.h"
#include "llvm/Support/raw_ostream.h"

#include "boost/regex.hpp"

const std::string LOCAL_VAR = "[a-z][a-zA-Z0-9]*";
const std::string MEMBER_VAR = "[a-z][a-zA-Z0-9]*_";
const std::string CONST_VAR = "[A-Z_]*";
const std::string CLASS_NAME = "[A-Z][a-zA-Z]";

void checkVariables(std::string filename, std::vector<Issue>& lineIssues);

class VarCheckVisitor : public clang::RecursiveASTVisitor<VarCheckVisitor> {
public:
  VarCheckVisitor(std::vector<Issue>& lineIssues, clang::SourceManager& sm);

  bool VisitVarDecl(clang::VarDecl *v);

  bool VisitDecl(clang::Decl *d);

private:
  std::vector<Issue>& lineIssues_;
  clang::SourceManager& sm_;

  std::pair<int, int> getDeclLocation(clang::Decl* d);
};

#endif
