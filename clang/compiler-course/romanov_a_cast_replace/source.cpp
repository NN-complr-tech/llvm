#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendPluginRegistry.h"
#include "llvm/Support/raw_ostream.h"
#include "clang/Lex/Lexer.h"
#include "clang/Rewrite/Core/Rewriter.h"

#include <string>

namespace {

static std::string getCppCastWord(const clang::CStyleCastExpr *expr) {
  switch (expr->getCastKind()) {

  case clang::CK_BitCast:
  case clang::CK_LValueBitCast:
  case clang::CK_LValueToRValueBitCast:
  case clang::CK_IntegralToPointer:
  case clang::CK_PointerToIntegral:
  case clang::CK_ReinterpretMemberPointer:
    return "reinterpret_cast";

  case clang::CK_NoOp:
    return "const_cast";

  default:
    return "static_cast";
  }
}

class RomanovACastReplaceVisitor final : public clang::RecursiveASTVisitor<RomanovACastReplaceVisitor> {
public:
  explicit RomanovACastReplaceVisitor(clang::ASTContext *context, clang::Rewriter &rewriter): m_context(context), m_rewriter(rewriter) {}

  bool VisitCStyleCastExpr(clang::CStyleCastExpr *expr) {
    clang::SourceManager &sm        = m_context->getSourceManager();
    const clang::LangOptions &opts  = m_context->getLangOpts();

    std::string cast_keyword = getCppCastWord(expr);
    std::string cast_type_keyword = expr->getTypeAsWritten().getAsString(m_context->getPrintingPolicy());

    clang::Expr *sub_expr = expr->getSubExprAsWritten();
    clang::StringRef sub_expr_text = clang::Lexer::getSourceText(clang::CharSourceRange::getTokenRange(sub_expr->getSourceRange()), sm, opts);

    std::string replacement_cast = cast_keyword + "<" + cast_type_keyword + ">(" + sub_expr_text.str() + ")";

    m_rewriter.ReplaceText(clang::CharSourceRange::getTokenRange(expr->getSourceRange()), replacement_cast);

    return true;
  }

private:
  clang::ASTContext *m_context;
  clang::Rewriter   &m_rewriter;
};

class RomanovACastReplaceConsumer final : public clang::ASTConsumer {
public:
  explicit RomanovACastReplaceConsumer(clang::ASTContext *context, clang::Rewriter &rewriter): m_visitor(context, rewriter) {}

  void HandleTranslationUnit(clang::ASTContext &context) override {
    m_visitor.TraverseDecl(context.getTranslationUnitDecl());
  }

private:
  RomanovACastReplaceVisitor m_visitor;
};

class RomanovACastReplaceAction final : public clang::PluginASTAction {
public:
  std::unique_ptr<clang::ASTConsumer>
  CreateASTConsumer(clang::CompilerInstance &ci, llvm::StringRef) override {
    m_rewriter.setSourceMgr(ci.getSourceManager(), ci.getLangOpts());
    return std::make_unique<RomanovACastReplaceConsumer>(&ci.getASTContext(), m_rewriter);
  }

  bool ParseArgs(const clang::CompilerInstance &, const std::vector<std::string> &) override {
    return true;
  }

  void EndSourceFileAction() override {
    m_rewriter.getEditBuffer(m_rewriter.getSourceMgr().getMainFileID()).write(llvm::outs());
  }

private:
  clang::Rewriter m_rewriter;
};
} // namespace

static clang::FrontendPluginRegistry::Add<RomanovACastReplaceAction>
    X("romanov_a_cast_replace_plugin", "Plugin to replace C-style castes in C++-style castes");
