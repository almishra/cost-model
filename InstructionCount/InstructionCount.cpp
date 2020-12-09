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

using namespace std;
using namespace clang;
using namespace llvm;

class InstructionCountVisitor : public RecursiveASTVisitor<InstructionCountVisitor> {
  private:
    SourceManager *SM;

    bool insideKernel;
    bool insideLoop;
    SourceLocation loopEnd;
    BeforeThanCompare<SourceLocation> isBefore;
    int num_collapse;

    template <typename T>
      bool kernel_found(Stmt *st)
      {
        auto cast_value = dyn_cast<T>(st);
        if (cast_value != nullptr) {
          /*llvm::errs() << "Kernel found at - ";
            cast_value->getBeginLoc().print(llvm::errs(), *SM);
            llvm::errs() << "\n";*/
          return true;
        }
        return false;
      }
  public:
    explicit InstructionCountVisitor(CompilerInstance *CI)
      : SM(&(CI->getASTContext().getSourceManager())), isBefore(*SM)
    {
      insideLoop = false;
      insideKernel = false;
      num_collapse = 0;
    }

    virtual bool VisitStmt(Stmt *st) {
      if(insideLoop && isBefore(loopEnd, st->getBeginLoc())) {
        insideLoop = false;
      }

      bool found = false;
      found |= kernel_found<OMPForDirective>(st);
      found |= kernel_found<OMPParallelForDirective>(st);
      /*found |= kernel_found<OMPTargetDirective>(st);
      found |= kernel_found<OMPTargetParallelDirective>(st);
      found |= kernel_found<OMPTargetParallelForDirective>(st);
      found |= kernel_found<OMPTargetTeamsDirective>(st);
      found |= kernel_found<OMPTargetTeamsDistributeDirective>(st);
      found |= kernel_found<OMPTargetTeamsDistributeParallelForDirective>(st);*/

      if(found) {
        llvm::errs() << "Kernel found at ";
        st->getBeginLoc().print(llvm::errs(), *SM);
        llvm::errs() << "\n";
        insideKernel = true;
        OMPExecutableDirective *omp = dyn_cast<OMPExecutableDirective>(st);
        omp->dump();
        for(unsigned int i=0; i<omp->getNumClauses(); i++) {
          if(OMPCollapseClause *collapse = dyn_cast<OMPCollapseClause>(omp->getClause(i))) {
            llvm::errs() << "Has collapse ";
            ConstantExpr *ex = dyn_cast<ConstantExpr>(collapse->getNumForLoops());
            if(IntegerLiteral *I = dyn_cast<IntegerLiteral>(ex->getSubExpr())) {
              I->getValue().print(llvm::errs(), true);
              double d = I->getValue().roundToDouble();
              llvm::errs() << " " << (int)d << "\n";
            }
          }
        }
        TraverseStmt(dyn_cast<OMPExecutableDirective>(st)->getAssociatedStmt());
        llvm::errs() << "Done Visiting associated statement\n";
        insideKernel = false;
      } else {
        if(ForStmt *f = dyn_cast<ForStmt>(st)) {
          if(insideKernel) {
          llvm::errs() << "For ";
          if(insideKernel)
            llvm::errs() << "inside kernel ";
          llvm::errs() << "ends at ";
          f->getEndLoc().print(llvm::errs(), *SM);
          llvm::errs() << "\n";
          llvm::errs() << "Iteration = ";
          if(BinaryOperator *bin = dyn_cast_or_null<BinaryOperator>(f->getCond())) {
            if(IntegerLiteral *I = dyn_cast<IntegerLiteral>(bin->getRHS())) {
              I->getValue().print(llvm::errs(), true);
            } else if(DeclRefExpr *decl = dyn_cast<DeclRefExpr>(dyn_cast<ImplicitCastExpr>(bin->getRHS())->getSubExpr())) {
              llvm::errs() << decl->getDecl()->getName();
            }
            llvm::errs() << "\n";
          } else {
            llvm::errs() << "Infinite\n";
          }
          }
          if(!insideLoop)
            loopEnd = f->getEndLoc();
          insideLoop = true;
        } else if(WhileStmt *w = dyn_cast<WhileStmt>(st)) {
          llvm::errs() << "While ends at ";
          w->getEndLoc().print(llvm::errs(), *SM);
          llvm::errs() << "\n";
          if(!insideLoop)
            loopEnd = w->getEndLoc();
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
    unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &CI,
        StringRef file) {
      return make_unique<InstructionCountASTConsumer>(&CI);
    }

    bool ParseArgs(const CompilerInstance &CI, const vector<string> &args) {
      return true;
    }
};

static FrontendPluginRegistry::Add<PluginInstructionCountAction>
X("-inst-count", "Plugin to count all instrcutions");
