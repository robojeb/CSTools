#include "extractor.hpp"
#include <iostream>

using namespace clang;

SourceExtractor::SourceExtractor(std::string file,
                                 std::string desiredFunction) {
  // CompilerInstance will hold the instance of the Clang compiler for us,
  // managing the various objects needed to run the compiler.
  CompilerInstance compilerInstance;
  compilerInstance.createDiagnostics();

  languageOptions_ = &compilerInstance.getLangOpts();
  languageOptions_->CPlusPlus = 1;
  languageOptions_->Bool = 1;
  languageOptions_->WChar = 1;
  languageOptions_->NoBuiltin = 0;

  // Initialize target info with the default triple for our platform.
  auto TO = std::make_shared<TargetOptions>();
  TO->Triple = llvm::sys::getDefaultTargetTriple();
  TargetInfo *TI =
      TargetInfo::CreateTargetInfo(compilerInstance.getDiagnostics(), TO);
  compilerInstance.setTarget(TI);

  compilerInstance.createFileManager();
  FileManager &FileMgr = compilerInstance.getFileManager();
  compilerInstance.createSourceManager(FileMgr);
  sourceManager_ = &compilerInstance.getSourceManager();
  compilerInstance.createPreprocessor(TU_Module);

  Preprocessor& pp = compilerInstance.getPreprocessor();

  pp.getBuiltinInfo().InitializeBuiltins(pp.getIdentifierTable(), *languageOptions_);

  compilerInstance.createASTContext();

  // Set the main file handled by the source manager to the input file.
  const FileEntry *FileIn = FileMgr.getFile(file.c_str());
  sourceManager_->setMainFileID(
      sourceManager_->createFileID(FileIn, SourceLocation(), SrcMgr::C_User));
  compilerInstance.getDiagnosticClient().BeginSourceFile(
      compilerInstance.getLangOpts(), &compilerInstance.getPreprocessor());

  // Create an AST consumer instance which is going to get called by
  // ParseAST.
  SourceExtractorASTConsumer consumer{desiredFunction,
                                      *sourceManager_,
                                      *languageOptions_};

  ParseAST(compilerInstance.getPreprocessor(), &consumer,
            compilerInstance.getASTContext());

  extractedSource_ = consumer.getExtractedSource();
}

std::string SourceExtractor::getExtractedSource() const {
  return extractedSource_;
}

SourceExtractorASTConsumer::SourceExtractorASTConsumer(
    std::string desiredFunction,
    SourceManager& sourceManager,
    LangOptions& languageOptions):
  sourceExtractor_(desiredFunction, sourceManager, languageOptions)
{
  //Nothing to do
}

bool SourceExtractorASTConsumer::HandleTopLevelDecl(DeclGroupRef DR) {
  for (DeclGroupRef::iterator b = DR.begin(), e = DR.end(); b != e; ++b)
    // Traverse the declaration using our AST visitor.
    sourceExtractor_.TraverseDecl(*b);
  return true;
}

std::string SourceExtractorASTConsumer::getExtractedSource() const {
  return sourceExtractor_.getExtractedSource();
}

SourceExtractorASTVisitor::SourceExtractorASTVisitor(
    std::string desiredFunction,
    SourceManager& sourceManager,
    LangOptions& languageOptions):
  desiredFunction_{desiredFunction},
  sourceManager_{sourceManager},
  languageOptions_{languageOptions}
{
  //Nothing to do
}

bool SourceExtractorASTVisitor::VisitFunctionDecl(FunctionDecl *f){
  if (f->hasBody()) {
    std::string funcName = f->getNameInfo().getName().getAsString();
    if (funcName == desiredFunction_){
      extractedSource_ = declToString(f);
    }
  }
  return true;
}

std::string SourceExtractorASTVisitor::getExtractedSource() const {
  return extractedSource_;
}

std::string SourceExtractorASTVisitor::declToString(Decl *d) {
  SourceLocation b(d->getLocStart()), _e(d->getLocEnd());
  SourceLocation e(clang::Lexer::getLocForEndOfToken(_e, 0, sourceManager_,
                                                      languageOptions_));
  return std::string(sourceManager_.getCharacterData(b),
      sourceManager_.getCharacterData(e)-sourceManager_.getCharacterData(b));
}
