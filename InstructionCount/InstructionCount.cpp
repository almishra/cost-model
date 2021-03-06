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

class For
{
private:
  ForStmt *st;
  int numIteration;
  SourceLocation endLocation;

public:
  For(ForStmt *fStmt, int num, SourceLocation loc)
    : st(fStmt), numIteration(num), endLocation(loc) {
    }

  int getNumIteration() { return numIteration; }
  ForStmt *getForStmt() { return st; };
  SourceLocation getEndLocation() { return endLocation; }
};

class InstructionCountVisitor :
  public RecursiveASTVisitor<InstructionCountVisitor> {
  private:
    ASTContext *astContext;
    SourceManager *SM;

    std::vector<For> innerFor;

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
      if(auto bin = dyn_cast<BinaryOperator>(s)) {
        if(IntegerLiteral *I = dyn_cast<IntegerLiteral>(bin->getRHS())) {
          ret = I->getValue().getLimitedValue(INT_MAX);
        }
      } else {
        if(!dyn_cast<DeclStmt>(s)->isSingleDecl()) {
          llvm::errs().changeColor(raw_ostream::RED);
          llvm::errs() << "Error: Only one init is expected in for loop at ";
          s->getBeginLoc().print(llvm::errs(), *SM);
          llvm::errs() << "\n";
          llvm::errs().changeColor(raw_ostream::WHITE);
          return INT_MIN;
        }
        VarDecl *d = dyn_cast<VarDecl>(dyn_cast<DeclStmt>(s)->getSingleDecl());
        if(IntegerLiteral *I = dyn_cast<IntegerLiteral>(d->getInit())) {
          ret = I->getValue().getLimitedValue(INT_MAX);
        }
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

    virtual bool VisitForStmt(ForStmt *st) {
      if(insideKernel) {
        llvm::errs().changeColor(raw_ostream::GREEN);
        llvm::errs() << "INFO: ForStmt inside kernel visited\n";
        llvm::errs().changeColor(raw_ostream::WHITE);
        int lb = getForLowerBound(st->getInit());
        if(lb == INT_MIN) {
          return false;
        }
        int ub = getForUpperBound(st->getCond());
        int stride = getForStride(st->getInc());
        int iter = (ub - (lb-stride)) / stride;
        llvm::errs().changeColor(raw_ostream::GREEN);
        llvm::errs() << "INFO:  LB  = " << lb << "\n";
        llvm::errs() << "INFO:  UB  = " << ub << "\n";
        llvm::errs() << "INFO:  INC = " << stride << "\n";
        llvm::errs() << "INFO:  ITR = " << iter << "\n";
        llvm::errs().changeColor(raw_ostream::WHITE);

        innerFor.push_back(For(st, iter, st->getEndLoc()));
        TraverseStmt(st->getBody());
        innerFor.pop_back();
      } else {
        if(!insideLoop)
          loopEnd = st->getEndLoc();
        insideLoop = true;
      }
      return true;
    }

    virtual bool VisitStmt(Stmt *st) {
      if(insideLoop && isBefore(loopEnd, st->getBeginLoc())) {
        insideLoop = false;
      }

      bool found = false;
      found |= kernel_found<OMPForDirective>(st);
      found |= kernel_found<OMPParallelForDirective>(st);

      if(found) {
        int id = lastKernel ? lastKernel->getID() + 1 : 1;
        lastKernel = new Kernel(id, st, currentFunction);
        llvm::errs().changeColor(raw_ostream::GREEN);
        llvm::errs() << "INFO: Kernel found at ";
        st->getBeginLoc().print(llvm::errs(), *SM);
        llvm::errs() << "\n";
        llvm::errs().changeColor(raw_ostream::WHITE);

        insideKernel = true;
        OMPLoopDirective *omp = dyn_cast<OMPLoopDirective>(st);
        for(unsigned int i = 0; i<omp->getNumClauses(); i++) {
          if(auto collapse = dyn_cast<OMPCollapseClause>(omp->getClause(i))) {
            Expr *ex = dyn_cast<Expr>(collapse->getNumForLoops());
            Expr::EvalResult Result;
            ex->EvaluateAsInt(Result, *astContext);
            llvm::errs().changeColor(raw_ostream::GREEN);
            llvm::errs() << "INFO: Kernel has collapse - " << Result.Val.getInt().getLimitedValue() << "\n";
            llvm::errs().changeColor(raw_ostream::WHITE);
          }
        }
        Expr::EvalResult result;
        omp->getNumIterations()->EvaluateAsInt(result, *astContext);
	      if(result.Val.isInt()) {
          lastKernel->setNumIteration(result.Val.getInt().getLimitedValue());
	      } else {
          llvm::errs().changeColor(raw_ostream::RED);
	        llvm::errs() << "ERROR: Expecting static int value in OMP Parallel for at ";
          st->getBeginLoc().print(llvm::errs(), *SM);
          llvm::errs() << "\n";
          llvm::errs().changeColor(raw_ostream::WHITE);
	        return false;
	      }

        bool ret = TraverseStmt(omp->getBody());
        if(ret == false)
          return false;
        llvm::errs().changeColor(raw_ostream::GREEN);
        llvm::errs() << "INFO: Done Visiting the kernel body\n";
        llvm::errs().changeColor(raw_ostream::WHITE);
        insideKernel = false;

        std::vector<Kernel*> vec;
        vec.push_back(lastKernel);
        kernel_map[id] = vec;
        //lastKernel->print();
        lastKernel->dump();
      } else {
        if(insideKernel) {
          int counter = 1;
          for(int i=0; i<innerFor.size(); i++)
            counter *= innerFor[i].getNumIteration();
          if(counter > 1) counter--;
          
          lastKernel->incrStmt(st, counter);
        }
      }

      return true;
    }
};

class InstructionCountASTConsumer : public ASTConsumer {
  private:
    InstructionCountVisitor *visitor;

  public:
    explicit InstructionCountASTConsumer(CompilerInstance *CI)
      : visitor(new InstructionCountVisitor(CI)) // initialize the visitor
    {}

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
