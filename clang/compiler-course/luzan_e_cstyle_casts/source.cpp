#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendPluginRegistry.h"
#include "llvm/Support/raw_ostream.h"
#include "clang/Rewrite/Core/Rewriter.h"
#include "clang/Lex/Lexer.h"
#include <iostream>

namespace {
class LuzanECstyleCastsVisitor final : public clang::RecursiveASTVisitor<LuzanECstyleCastsVisitor> {
public:
  explicit LuzanECstyleCastsVisitor(clang::ASTContext *context, clang::Rewriter &rewriter_) : m_context(context), rewriter(rewriter_)  {}
  
  bool VisitCStyleCastExpr(clang::CStyleCastExpr *expr) {
    /// determine which cast type is it
    std::string castName = getCastName(expr->getCastKind());         
    /// get the target type of cast
    std::string targetType = expr->getTypeAsWritten().getAsString();

    /// get casting (expression)
    clang::Expr *subExpr = expr->getSubExpr();

    /// gett raw_text
    clang::SourceManager &source_manager = m_context->getSourceManager(); /// ast (sm)--> code  
    std::string subExprText =
        clang::Lexer::getSourceText(clang::CharSourceRange::getTokenRange(subExpr->getSourceRange()), source_manager, m_context->getLangOpts()).str();
        /// getTokenRange - get the whole token  

    /// new code gen
    std::string cpp_cast = castName + "<" + targetType + ">(" + subExprText + ")";

    /// replace
    // rewriter.ReplaceText(expr->getSourceRange(), cpp_cast);

    clang::CharSourceRange range =
    clang::CharSourceRange::getTokenRange(expr->getSourceRange());

    rewriter.ReplaceText(range, cpp_cast);
    

    return true;
  }

private:
  clang::ASTContext *m_context;
  clang::Rewriter &rewriter;

  std::string getCastName(clang::CastKind kind) {
    switch (kind) {

    case clang::CK_BitCast:
    case clang::CK_LValueBitCast:
      return "reinterpret_cast";

    case clang::CK_NoOp:
    case clang::CK_IntegralCast:
    case clang::CK_FloatingCast:
    case clang::CK_IntegralToFloating:
    case clang::CK_FloatingToIntegral:
    case clang::CK_BaseToDerived:
    case clang::CK_DerivedToBase:
      return "static_cast";

    // case clang::CK_ConstCast:
    //   return "const_cast";

    default:
      return "static_cast";
    }
  }
};

class LuzanECstyleCastsConsumer final : public clang::ASTConsumer {
public:
  explicit LuzanECstyleCastsConsumer(clang::ASTContext *context, clang::Rewriter &rewriter) 
            : m_visitor(context,rewriter) {}

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
    rewriter.setSourceMgr(ci.getSourceManager(),ci.getLangOpts());
    llvm::errs() << "Plugin loaded\n";
    return std::make_unique<LuzanECstyleCastsConsumer>(&ci.getASTContext(), rewriter);
  }

  bool ParseArgs(const clang::CompilerInstance &ci,
                 const std::vector<std::string> &args) override {
    return true;
  }

  void EndSourceFileAction() override {
    rewriter.getEditBuffer(rewriter.getSourceMgr().getMainFileID()).write(llvm::outs());
  }

  // void EndSourceFileAction() override {
  //   rewriter.getEditBuffer(rewriter.getSourceMgr().getMainFileID()).write(llvm::outs());
  // }

  private:
    clang::Rewriter rewriter;
};
} // namespace

static clang::FrontendPluginRegistry::Add<LuzanECstyleCastsAction>
    X("luzan_e_cstyle_casts", "Description plugin");
