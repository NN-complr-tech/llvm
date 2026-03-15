#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendPluginRegistry.h"
#include "clang/AST/Expr.h"
#include "clang/AST/ExprCXX.h"
#include "clang/AST/Decl.h"
#include "clang/AST/ParentMapContext.h"
#include "clang/Analysis/CFG.h"
#include "llvm/Support/raw_ostream.h"

#include <set>
#include <vector>
#include <map>

namespace {

// Структура для хранения информации о ресурсе
struct ResourceInfo {
  clang::SourceLocation allocLoc;      // место выделения
  clang::VarDecl *variable;            // переменная, хранящая ресурс
  std::string type;                    // "memory" или "file"
  bool isFreed;                        // освобожден ли ресурс
  std::vector<clang::SourceLocation> freeLocs; // места освобождения
  
  ResourceInfo(clang::SourceLocation loc, clang::VarDecl *var, const std::string &t)
    : allocLoc(loc), variable(var), type(t), isFreed(false) {}
};

// Контекст функции для анализа путей выполнения
class FunctionContext {
public:
  std::vector<ResourceInfo> resources;
  std::map<clang::VarDecl*, size_t> varToResourceIndex;
  
  // void addResource(clang::SourceLocation loc, clang::VarDecl *var, const std::string &type) {
  //   // Если переменная уже имеет ресурс, помечаем старый как потенциально потерянный
  //   if (var && varToResourceIndex.count(var)) {
  //     size_t oldIndex = varToResourceIndex[var];
  //     if (!resources[oldIndex].isFreed) {
  //       resources[oldIndex].isFreed = true; // не совсем корректно, но для предупреждения
  //     }
  //   }
    
  //   size_t index = resources.size();
  //   resources.emplace_back(loc, var, type);
  //   if (var) {
  //     varToResourceIndex[var] = index;
  //   }
  // }

  void addResource(clang::SourceLocation loc, clang::VarDecl *var, const std::string &type) {
  // Если переменная уже имеет ресурс, это потенциальная утечка!
  // НЕ помечаем старый как freed, просто добавляем новый ресурс
  // Старый останется в списке и будет обнаружен при проверке
  
  size_t index = resources.size();
  resources.emplace_back(loc, var, type);
  if (var) {
    // Если у переменной уже был ресурс, старая запись останется
    // и будет считаться утечкой (если не freed)
    varToResourceIndex[var] = index;
  }
}
  
  void markFreed(clang::Expr *arg, const std::string &deallocType) {
    clang::VarDecl *var = getVarDeclFromExpr(arg);
    if (var && varToResourceIndex.count(var)) {
      size_t index = varToResourceIndex[var];
      if (!resources[index].isFreed && resources[index].type == deallocType) {
        resources[index].isFreed = true;
        resources[index].freeLocs.push_back(arg->getExprLoc());
      }
    }
  }
  
  void markFreed(clang::Expr *arg) {
    clang::VarDecl *var = getVarDeclFromExpr(arg);
    if (var && varToResourceIndex.count(var)) {
      size_t index = varToResourceIndex[var];
      if (!resources[index].isFreed) {
        resources[index].isFreed = true;
        resources[index].freeLocs.push_back(arg->getExprLoc());
      }
    }
  }
  
private:
  clang::VarDecl *getVarDeclFromExpr(clang::Expr *e) {
    if (!e) return nullptr;
    e = e->IgnoreParenCasts();
    
    if (auto *declRef = clang::dyn_cast<clang::DeclRefExpr>(e)) {
      return clang::dyn_cast<clang::VarDecl>(declRef->getDecl());
    }
    return nullptr;
  }
};

class PylaevaSVisitor final : public clang::RecursiveASTVisitor<PylaevaSVisitor> {
public:
  explicit PylaevaSVisitor(clang::ASTContext *context)
      : m_context(context), m_sourceManager(context->getSourceManager()) {}

  bool VisitFunctionDecl(clang::FunctionDecl *func) {
    if (!func->hasBody() || !m_sourceManager.isInMainFile(func->getLocation()))
      return true;
    
    // Создаем контекст для текущей функции
    FunctionContext currentContext;
    m_funcContexts.push_back(currentContext);
    
    // Обходим тело функции
    TraverseStmt(func->getBody());
    
    // Проверяем утечки в функции
    checkLeaksInCurrentFunction();
    
    // Удаляем контекст
    m_funcContexts.pop_back();
    
    return true;
  }

  bool VisitCallExpr(clang::CallExpr *call) {
    if (m_funcContexts.empty()) return true;
    clang::FunctionDecl *func = call->getDirectCallee();
    if (!func) return true;
    
    std::string name = func->getNameInfo().getName().getAsString();
    clang::SourceLocation loc = call->getExprLoc();
    
    if (!m_sourceManager.isInMainFile(loc))
      return true;
    
    // Выделение ресурсов
    if (name == "malloc" || name == "calloc" || name == "realloc") {
      // Пытаемся найти переменную, которой присваивается результат
      clang::VarDecl *var = findVarDeclForCall(call);
      getCurrentContext().addResource(loc, var, "memory");
    }
    else if (name == "fopen") {
      clang::VarDecl *var = findVarDeclForCall(call);
      getCurrentContext().addResource(loc, var, "file");
    }
    // Освобождение ресурсов
    else if (name == "free") {
      if (call->getNumArgs() > 0) {
        getCurrentContext().markFreed(call->getArg(0), "memory");
      }
    }
    else if (name == "fclose") {
      if (call->getNumArgs() > 0) {
        getCurrentContext().markFreed(call->getArg(0), "file");
      }
    }
    
    return true;
  }

  bool VisitCXXNewExpr(clang::CXXNewExpr *newExpr) {
    if (m_funcContexts.empty()) return true;
    clang::SourceLocation loc = newExpr->getExprLoc();
    if (m_sourceManager.isInMainFile(loc)) {
      // Для new сложнее найти переменную, но попробуем
      getCurrentContext().addResource(loc, nullptr, "memory");
    }
    return true;
  }

  bool VisitCXXDeleteExpr(clang::CXXDeleteExpr *deleteExpr) {
    if (m_funcContexts.empty()) return true;
    clang::Expr *arg = deleteExpr->getArgument();
    getCurrentContext().markFreed(arg);
    return true;
  }

  bool VisitBinaryOperator(clang::BinaryOperator *op) {
    if (m_funcContexts.empty()) return true;
    if (op->isAssignmentOp()) {
      // Проверяем, не присваивается ли результат выделения
      if (isAllocation(op->getRHS())) {
        clang::VarDecl *var = getVarDeclFromExpr(op->getLHS());
        clang::SourceLocation loc = op->getExprLoc();
        
        if (var && m_sourceManager.isInMainFile(loc)) {
          // Определяем тип ресурса
          std::string type = getAllocationType(op->getRHS());
          getCurrentContext().addResource(loc, var, type);
        }
      }
    }
    return true;
  }

  bool VisitReturnStmt(clang::ReturnStmt *ret) {
    if (m_funcContexts.empty()) return true;
    // Проверяем неосвобожденные ресурсы при return
    auto &ctx = getCurrentContext();
    for (const auto &res : ctx.resources) {
      if (!res.isFreed) {
        // Проверяем, не возвращается ли ресурс из функции
        clang::Expr *retValue = ret->getRetValue();
        if (retValue && res.variable) {
          clang::VarDecl *retVar = getVarDeclFromExpr(retValue);
          if (retVar == res.variable) {
            // Возврат ресурса - это не утечка (владение передается)
            continue;
          }
        }
        
        // Иначе - потенциальная утечка
        clang::DiagnosticsEngine &DE = m_context->getDiagnostics();
        unsigned diagID = DE.getCustomDiagID(
            clang::DiagnosticsEngine::Warning,
            "Ресурс (%1) выделенный в строке %0 может не освободиться при выходе из функции");
        
        DE.Report(ret->getReturnLoc(), diagID)
            << m_sourceManager.getSpellingLineNumber(res.allocLoc)
            << res.type;
      }
    }
    return true;
  }

private:
  clang::ASTContext *m_context;
  clang::SourceManager &m_sourceManager;
  std::vector<FunctionContext> m_funcContexts;
  
  FunctionContext &getCurrentContext() {
    return m_funcContexts.back();
  }
  
// Поиск переменной, которой присваивается результат вызова
clang::VarDecl *findVarDeclForCall(clang::CallExpr *call) {
    auto parents = m_context->getParents(*call);
    for (const auto &parent : parents) {
        // Проверяем, не является ли родитель бинарным оператором присваивания
        if (auto *binOp = parent.get<clang::BinaryOperator>()) {
            if (binOp->isAssignmentOp() && binOp->getRHS() == call) {
                return getVarDeclFromExpr(binOp->getLHS());
            }
        }
        // Проверяем, не является ли родитель объявлением переменной
        else if (auto *varDecl = parent.get<clang::VarDecl>()) {
            if (varDecl->getInit() == call) {
                // Убираем константность через const_cast
                return const_cast<clang::VarDecl*>(varDecl);
            }
        }
    }
    return nullptr;
}
  
  clang::VarDecl *getVarDeclFromExpr(clang::Expr *e) {
    if (!e) return nullptr;
    e = e->IgnoreParenCasts();
    
    if (auto *declRef = clang::dyn_cast<clang::DeclRefExpr>(e)) {
      return clang::dyn_cast<clang::VarDecl>(declRef->getDecl());
    }
    return nullptr;
  }
  
  bool isAllocation(clang::Expr *e) {
    if (!e) return false;
    e = e->IgnoreParenCasts();
    
    if (auto *call = clang::dyn_cast<clang::CallExpr>(e)) {
      if (auto *func = call->getDirectCallee()) {
        std::string name = func->getNameInfo().getName().getAsString();
        return name == "malloc" || name == "calloc" || 
               name == "realloc" || name == "fopen";
      }
    }
    
    return clang::isa<clang::CXXNewExpr>(e);
  }
  
  std::string getAllocationType(clang::Expr *e) {
    if (!e) return "unknown";
    e = e->IgnoreParenCasts();
    
    if (auto *call = clang::dyn_cast<clang::CallExpr>(e)) {
      if (auto *func = call->getDirectCallee()) {
        std::string name = func->getNameInfo().getName().getAsString();
        if (name == "fopen") return "file";
        return "memory";
      }
    }
    
    if (clang::isa<clang::CXXNewExpr>(e)) {
      return "memory";
    }
    
    return "unknown";
  }
  
  // void checkLeaksInCurrentFunction() {
  //   auto &ctx = getCurrentContext();
    
  //   for (const auto &res : ctx.resources) {
  //     if (!res.isFreed) {
  //       // Проверяем, не было ли присваивания новой переменной
  //       bool found = false;
  //       for (const auto &res2 : ctx.resources) {
  //         if (res2.variable == res.variable && &res != &res2) {
  //           found = true;
  //           break;
  //         }
  //       }
        
  //       if (found) continue; // ресурс переприсвоен, мог быть потерян ранее
        
  //       clang::DiagnosticsEngine &DE = m_context->getDiagnostics();
  //       unsigned diagID = DE.getCustomDiagID(
  //           clang::DiagnosticsEngine::Warning,
  //           "Потенциальная утечка %0: ресурс выделен в строке %1 не освобожден");
        
  //       DE.Report(res.allocLoc, diagID)
  //           << res.type
  //           << m_sourceManager.getSpellingLineNumber(res.allocLoc);
  //     }
  //   }
  // }

  void checkLeaksInCurrentFunction() {
  auto &ctx = getCurrentContext();
  
  for (const auto &res : ctx.resources) {
    if (!res.isFreed) {
      // Убираем эту проверку - она больше не нужна
      // bool found = false;
      // for (const auto &res2 : ctx.resources) {
      //   if (res2.variable == res.variable && &res != &res2) {
      //     found = true;
      //     break;
      //   }
      // }
      // if (found) continue; // ресурс переприсвоен, мог быть потерян ранее
      
      clang::DiagnosticsEngine &DE = m_context->getDiagnostics();
      unsigned diagID = DE.getCustomDiagID(
          clang::DiagnosticsEngine::Warning,
          "Потенциальная утечка %0: ресурс выделен в строке %1 не освобожден");
      
      DE.Report(res.allocLoc, diagID)
          << res.type
          << m_sourceManager.getSpellingLineNumber(res.allocLoc);
    }
  }
}
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
    X("pylaeva_s_lab1_plugin", "Resource leak analyzer");