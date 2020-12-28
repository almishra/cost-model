#include "clang/Driver/Options.h"
#include "clang/AST/AST.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/ASTConsumers.h"
#include "clang/Frontend/FrontendActions.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendPluginRegistry.h"
#include "clang/Rewrite/Core/Rewriter.h"
#include "clang/AST/Stmt.h"
#include "clang/AST/Expr.h"
#include "Kernel.h"

using namespace clang;
using namespace llvm;

class InstructionCountVisitor :
  public RecursiveASTVisitor<InstructionCountVisitor> {
  private:
    ASTContext *astContext;
    SourceManager *SM;

    bool insideKernel;
    bool insideLoop;
    SourceLocation loopEnd;
    BeforeThanCompare<SourceLocation> isBefore;
    int num_collapse;
    clang::FunctionDecl *currentFunction;
    Kernel *lastKernel;
    std::map<int, std::vector<Kernel*>> kernel_map;

    template <typename T>
    bool kernel_found(Stmt *st)
    {
      if (dyn_cast<T>(st) != nullptr)
        return true;

      return false;
    }

    int getForLowerBound(Stmt* s)
    {
      int ret = 0;
      VarDecl *d = dyn_cast<VarDecl>(dyn_cast<DeclStmt>(s)->getSingleDecl());
      d->getInit()->dump();
      if(IntegerLiteral *I = dyn_cast<IntegerLiteral>(d->getInit())) {
        ret = I->getValue().getLimitedValue(INT_MAX);
      }

      return ret;
    }

    int getForUpperBound(Expr *e)
    {
      int ret;
      if(BinaryOperator *bin = dyn_cast<BinaryOperator>(e)) {
        Expr *rhs = bin->getRHS();
        if(IntegerLiteral *I = dyn_cast<IntegerLiteral>(rhs)) {
          ret = I->getValue().getLimitedValue(INT_MAX);
        } else {
          ret = INT_MAX;
        }
        BinaryOperatorKind o = bin->getOpcode();
        switch(o) {
          case BO_LT:
            ret--;
            break;
          default:
            break;
        }
      }

      return ret;
    }

    int getForStride(Expr *e) {
      int ret = 1;
      if(auto u = dyn_cast<UnaryOperator>(e)) {
        UnaryOperatorKind o = u->getOpcode();
        switch(o) {
          case UO_PostInc:
          case UO_PreInc:
            ret = 1;
            break;
          case UO_PostDec:
          case UO_PreDec:
            ret = -1;
            break;
          default:
            break;
        }
      } else if (auto c = dyn_cast<CompoundAssignOperator>(e)) {
        IntegerLiteral *I = dyn_cast<IntegerLiteral>(c->getRHS());
        int val = I->getValue().getLimitedValue(INT_MAX);
        BinaryOperatorKind o = c->getOpcode();
        switch(o) {
          case BO_AddAssign:
            ret = val;
            break;
          case BO_SubAssign:
            ret = -val;
            break;
          default:
            break;
        }
      } else if (auto bin = dyn_cast<BinaryOperator>(e)) {
        BinaryOperator *rhs = dyn_cast<BinaryOperator>(bin->getRHS());
        IntegerLiteral *I = dyn_cast<IntegerLiteral>(c->getRHS());
        int val = I->getValue().getLimitedValue(INT_MAX);
        BinaryOperatorKind o = rhs->getOpcode();
        switch(o) {
          case BO_Add:
            ret = val;
            break;
          case BO_Sub:
            ret = -val;
            break;
          default:
            break;
        }
      }

      return ret;
    }

  public:
    explicit InstructionCountVisitor(CompilerInstance *CI)
      : astContext(&(CI->getASTContext())),
        SM(&(CI->getASTContext().getSourceManager())), isBefore(*SM)
    {
      insideLoop = false;
      insideKernel = false;
      num_collapse = 0;
      lastKernel = NULL;
    }

    std::map<int, std::vector<Kernel*>> getKernelMap() { return kernel_map; }

    virtual bool VisitFunctionDecl(FunctionDecl *FD) {
      currentFunction = FD;
      return true;
    }

    virtual bool VisitStmt(Stmt *st) {
      if(insideLoop && isBefore(loopEnd, st->getBeginLoc())) {
        insideLoop = false;
      }

      bool found = false;
      //found |= kernel_found<OMPForDirective>(st);
      found |= kernel_found<OMPParallelForDirective>(st);

      if(found) {
        int id = lastKernel ? lastKernel->getID() + 1 : 1;
        lastKernel = new Kernel(id, st, currentFunction);
        llvm::errs() << "Kernel found at ";
        st->getBeginLoc().print(llvm::errs(), *SM);
        llvm::errs() << "\n";
        insideKernel = true;
        OMPParallelForDirective *omp = dyn_cast<OMPParallelForDirective>(st);
        for(unsigned int i = 0; i<omp->getNumClauses(); i++) {
          if(auto collapse = dyn_cast<OMPCollapseClause>(omp->getClause(i))) {
            llvm::errs() << "Has collapse ";
            Expr *ex = dyn_cast<Expr>(collapse->getNumForLoops());
            Expr::EvalResult Result;
            ex->EvaluateAsInt(Result, *astContext);
            llvm::errs() << " " << Result.Val.getInt().getLimitedValue() << "\n";
          }
        }
        Expr::EvalResult result;
        omp->getNumIterations()->EvaluateAsInt(result, *astContext);
        lastKernel->setIteration(result.Val.getInt().getLimitedValue());

        TraverseStmt(omp->getBody());
        llvm::errs() << "Done Visiting the kernel body\n";
        insideKernel = false;

        std::vector<Kernel*> vec;
        vec.push_back(lastKernel);
        kernel_map[id] = vec;
        lastKernel->print();
      } else {
        if(insideKernel) {
          int count = 1;
          if(ForStmt *f = dyn_cast<ForStmt>(st)) {
            int lb = getForLowerBound(f->getInit());
            int ub = getForUpperBound(f->getCond());
            int stride = getForStride(f->getInc());
            int iter = (ub - (lb-stride)) / stride;
            llvm::errs() << "LB = " << lb << "\n";
            llvm::errs() << "UB = " << ub << "\n";
            llvm::errs() << "INC = " << stride << "\n";
            llvm::errs() << "Iteration = " << iter << "\n";
            count = iter;
          }
          lastKernel->incrStmt(st, count);
        }
        if(dyn_cast<ForStmt>(st) || dyn_cast<WhileStmt>(st)) {
          if(!insideLoop)
            loopEnd = st->getEndLoc();
          insideLoop = true;
        }
      }

      return true;
    }
};

class InstructionCountASTConsumer : public ASTConsumer {
  private:
    InstructionCountVisitor *visitor; // doesn't have to be private
  public:
    explicit InstructionCountASTConsumer(CompilerInstance *CI)
      : visitor(new InstructionCountVisitor(CI)) // initialize the visitor
    { }

    virtual void HandleTranslationUnit(ASTContext &Context) {
      visitor->TraverseDecl(Context.getTranslationUnitDecl());
    }
};

class PluginInstructionCountAction : public PluginASTAction {
  protected:
    std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &CI,
                                                   StringRef file) {
      return std::make_unique<InstructionCountASTConsumer>(&CI);
    }

    bool ParseArgs(const CompilerInstance &CI, 
                   const std::vector<std::string> &args) {
      return true;
    }
};

static FrontendPluginRegistry::Add<PluginInstructionCountAction>
X("-inst-count", "Plugin to count all instrcutions");
