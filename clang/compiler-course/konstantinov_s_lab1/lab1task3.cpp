#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendPluginRegistry.h"
#include "llvm/Support/raw_ostream.h"

namespace {
  enum class ReplacementCastKind {
  Static,
  Const,
  Reinterpret
};

static std::string castKindToString(ReplacementCastKind kind) {
  switch (kind) {
  case ReplacementCastKind::Static:
    return "static_cast";
  case ReplacementCastKind::Const:
    return "const_cast";
  case ReplacementCastKind::Reinterpret:
    return "reinterpret_cast";
  }

  return "static_cast";
}

static ReplacementCastKind classifyCast(const clang::CStyleCastExpr *expr) {
  const clang::CastKind kind = expr->getCastKind();

  const clang::QualType sourceType = expr->getSubExpr()->getType();
  const clang::QualType targetType = expr->getType();

  return ReplacementCastKind::Static;
}

class CastRewriteVisitor final : public clang::RecursiveASTVisitor<CastRewriteVisitor> {
public:
  explicit CastRewriteVisitor(clang::ASTContext *context) : m_context(context) {}
  bool VisitFunctionDecl(clang::FunctionDecl *func) {
    func->dump();
    return true;
  }

private:
  clang::ASTContext *m_context;
};

class CastRewriteConsumer final : public clang::ASTConsumer {
public:
  explicit CastRewriteConsumer(clang::ASTContext *context) : m_visitor(context) {}

  void HandleTranslationUnit(clang::ASTContext &context) override {
    m_visitor.TraverseDecl(context.getTranslationUnitDecl());
  }

private:
  CastRewriteVisitor m_visitor;
};

class CastRewriteAction final : public clang::PluginASTAction {
public:
  std::unique_ptr<clang::ASTConsumer>
  CreateASTConsumer(clang::CompilerInstance &ci, llvm::StringRef) override {
    return std::make_unique<CastRewriteConsumer>(&ci.getASTContext());
  }

  bool ParseArgs(const clang::CompilerInstance &ci,
                 const std::vector<std::string> &args) override {
    return true;
  }
};
} // namespace

static clang::FrontendPluginRegistry::Add<CastRewriteAction>
    X("CastRewrite_plugin", "Description plugin");
