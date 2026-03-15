#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendPluginRegistry.h"
#include "llvm/Support/raw_ostream.h"

namespace {
class PylaevaSVisitor final : public clang::RecursiveASTVisitor<PylaevaSVisitor> {
public:
  explicit PylaevaSVisitor(clang::ASTContext *context) : m_context(context) {}
  bool VisitFunctionDecl(clang::FunctionDecl *func) {
    func->dump();
    return true;
  }

private:
  clang::ASTContext *m_context;
};

class PylaevaSConsumer final : public clang::ASTConsumer {
public:
  explicit PylaevaSConsumer(clang::ASTContext *context) : m_visitor(context) {}

  void HandleTranslationUnit(clang::ASTContext &context) override {
    m_visitor.TraverseDecl(context.getTranslationUnitDecl());
  }

private:
  PylaevaSVisitor m_visitor;
};

class PylaevaSAction final : public clang::PluginASTAction {
public:
  std::unique_ptr<clang::ASTConsumer>
  CreateASTConsumer(clang::CompilerInstance &ci, llvm::StringRef) override {
    return std::make_unique<PylaevaSConsumer>(&ci.getASTContext());
  }

  bool ParseArgs(const clang::CompilerInstance &ci,
                 const std::vector<std::string> &args) override {
    return true;
  }
};
} // namespace

static clang::FrontendPluginRegistry::Add<PylaevaSAction>
    X("pylaeva_s_lab1_plugin", "Detects resource leaks");
