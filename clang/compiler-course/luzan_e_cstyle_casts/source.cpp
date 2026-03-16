#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendPluginRegistry.h"
#include "llvm/Support/raw_ostream.h"

namespace {
class LuzanECstyleCastsVisitor final : public clang::RecursiveASTVisitor<LuzanECstyleCastsVisitor> {
public:
  explicit LuzanECstyleCastsVisitor(clang::ASTContext *context) : m_context(context) {}
  bool VisitFunctionDecl(clang::FunctionDecl *func) {
    func->dump();
    return true;
  }

private:
  clang::ASTContext *m_context;
};

class LuzanECstyleCastsConsumer final : public clang::ASTConsumer {
public:
  explicit LuzanECstyleCastsConsumer(clang::ASTContext *context) : m_visitor(context) {}

  void HandleTranslationUnit(clang::ASTContext &context) override {
    m_visitor.TraverseDecl(context.getTranslationUnitDecl());
  }

private:
  LuzanECstyleCastsVisitor m_visitor;
};

class LuzanECstyleCastsAction final : public clang::PluginASTAction {
public:
  std::unique_ptr<clang::ASTConsumer>
  CreateASTConsumer(clang::CompilerInstance &ci, llvm::StringRef) override {
    return std::make_unique<LuzanECstyleCastsConsumer>(&ci.getASTContext());
  }

  bool ParseArgs(const clang::CompilerInstance &ci,
                 const std::vector<std::string> &args) override {
    return true;
  }
};
} // namespace

static clang::FrontendPluginRegistry::Add<LuzanECstyleCastsAction>
    X("luzan_e_cstyle_casts", "Description plugin");
