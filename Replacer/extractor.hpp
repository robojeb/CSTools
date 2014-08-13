#ifndef SOURCE_EXTRACTOR_HPP_INC
#define SOURCE_EXTRACTOR_HPP_INC

#include <string>

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

class SourceExtractor {
public:
  SourceExtractor(std::string file, std::string desiredFunction);

  std::string getExtractedSource() const;

private:

  //The extracted source
  std::string extractedSource_;

  //Clang related things for reading the AST
  clang::SourceManager* sourceManager_;
  clang::LangOptions* languageOptions_;

};

class SourceExtractorASTVisitor: public clang::RecursiveASTVisitor<SourceExtractorASTVisitor> {
public:
  SourceExtractorASTVisitor(std::string desiredFunction,
                            clang::SourceManager& sourceManager,
                            clang::LangOptions& languageOptions);

  bool VisitFunctionDecl(clang::FunctionDecl *f);

  std::string getExtractedSource() const;

private:

  std::string declToString(clang::Decl *d);

  std::string desiredFunction_;
  clang::SourceManager& sourceManager_;
  clang::LangOptions& languageOptions_;

  std::string extractedSource_;
};

class SourceExtractorASTConsumer: public clang::ASTConsumer {
public:
  SourceExtractorASTConsumer(std::string desiredFunction,
                             clang::SourceManager& sourceManager,
                             clang::LangOptions& languageOptions);

  virtual bool HandleTopLevelDecl(clang::DeclGroupRef DR);

  std::string getExtractedSource() const;

private:
  SourceExtractorASTVisitor sourceExtractor_;
};

#endif
