#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendPluginRegistry.h"
#include "clang/Lex/Lexer.h"
#include "clang/Rewrite/Core/Rewriter.h"
#include "llvm/Support/raw_ostream.h"

using namespace clang;

namespace {

class CastReplaceVisitor : public RecursiveASTVisitor<CastReplaceVisitor> {
public:
    explicit CastReplaceVisitor(ASTContext *Context, Rewriter &R)
        : Context(Context), Rewrite(R) {}

    bool VisitCStyleCastExpr(CStyleCastExpr *Cast) {
        SourceManager &SM = Context->getSourceManager();
        const Expr *SubExpr = Cast->getSubExpr();

        std::string CastType = "static_cast";
        CastKind Kind = Cast->getCastKind();

        if (Kind == CK_BitCast || Kind == CK_PointerToIntegral || Kind == CK_IntegralToPointer) {
            CastType = "reinterpret_cast";
        } else if (Kind == CK_NoOp) {
            QualType DestT = Cast->getType();
            QualType SrcT = SubExpr->getType();
            if (DestT.isConstQualified() != SrcT.isConstQualified() ||
                DestT.isVolatileQualified() != SrcT.isVolatileQualified()) {
                CastType = "const_cast";
            }
        }

        std::string DestTypeStr = Cast->getTypeAsWritten().getAsString();
        std::string Replacement = CastType + "<" + DestTypeStr + ">(";

        SourceRange CastRange(Cast->getLParenLoc(), Cast->getRParenLoc());
        Rewrite.ReplaceText(CastRange, Replacement);

        SourceLocation EndLoc = Lexer::getLocForEndOfToken(SubExpr->getEndLoc(), 0, SM, Context->getLangOpts());
        Rewrite.InsertTextAfterToken(EndLoc, ")");

        return true;
    }

private:
    ASTContext *Context;
    Rewriter &Rewrite;
};

class CastReplaceConsumer final : public ASTConsumer {
public:
    explicit CastReplaceConsumer(ASTContext *Context, Rewriter &R)
        : Visitor(Context, R) {}

    void HandleTranslationUnit(ASTContext &Context) override {
        Visitor.TraverseDecl(Context.getTranslationUnitDecl());
    }

private:
    CastReplaceVisitor Visitor;
};

class CastReplaceAction final : public PluginASTAction {
public:
    std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &CI, llvm::StringRef) override {
        TheRewriter.setSourceMgr(CI.getSourceManager(), CI.getLangOpts());
        return std::make_unique<CastReplaceConsumer>(&CI.getASTContext(), TheRewriter);
    }

    bool ParseArgs(const CompilerInstance &CI, const std::vector<std::string> &args) override {
        return true;
    }

    void EndSourceFileAction() override {
        SourceManager &SM = TheRewriter.getSourceMgr();
        // Просто выводим весь буфер напрямую в поток ошибок, без дополнительных переменных
        TheRewriter.getEditBuffer(SM.getMainFileID()).write(llvm::errs());
    }

private:
    Rewriter TheRewriter;
};

} // namespace

static FrontendPluginRegistry::Add<CastReplaceAction>
    X("cast_replace_plugin", "Replaces C-style casts with C++ casts");