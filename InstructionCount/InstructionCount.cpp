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
    SourceLocation startLocation;
    SourceLocation endLocation;

  public:
    For(ForStmt *fStmt, int num, SourceLocation start, SourceLocation end)
      : st(fStmt), numIteration(num), startLocation(start), endLocation(end) {
      }

    int getNumIteration() { return numIteration; }
    ForStmt *getForStmt() { return st; };
    SourceLocation getEndLocation() { return endLocation; }
    SourceLocation getStartLocation() { return startLocation; }
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
          } else if(BinaryOperator *b = dyn_cast<BinaryOperator>(rhs)) {
            int l, r;
            //llvm::errs() << "Binary Calculation\n";
            if(IntegerLiteral *L = dyn_cast<IntegerLiteral>(b->getLHS())) l = L->getValue().getLimitedValue(INT_MAX);
            else l = INT_MAX;
            if(IntegerLiteral *R = dyn_cast<IntegerLiteral>(b->getRHS())) r = R->getValue().getLimitedValue(INT_MAX);
            else r = INT_MAX;
            //llvm::errs() << "L: " << l << ", R: " << r << "\n";
            BinaryOperatorKind o = b->getOpcode();
            switch(o) {
              case BO_Add: ret = l + r; break;
              case BO_Sub: ret = l - r; break;
              default: break;
            }
            //llvm::errs() << "Ret: " << ret << "\n";
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
          //llvm::errs() << "INFO: ForStmt inside kernel visited\n";
          llvm::errs().changeColor(raw_ostream::WHITE);
          int lb = getForLowerBound(st->getInit());
          if(lb == INT_MIN) {
            return false;
          }
          int ub = getForUpperBound(st->getCond());
          int stride = getForStride(st->getInc());
          int iter = (ub - (lb-stride)) / stride;
          llvm::errs().changeColor(raw_ostream::GREEN);
          //llvm::errs() << "INFO:  LB  = " << lb << "\n";
          //llvm::errs() << "INFO:  UB  = " << ub << "\n";
          //llvm::errs() << "INFO:  INC = " << stride << "\n";
          //llvm::errs() << "INFO:  ITR = " << iter << "\n";
          //llvm::errs() << "INFO: Start Location ";
          //st->getBody()->getBeginLoc().print(llvm::errs(), *SM);
          //llvm::errs() << "\nINFO: End Location ";
          //st->getEndLoc().print(llvm::errs(), *SM);
          //llvm::errs() << "\n";
          llvm::errs().changeColor(raw_ostream::WHITE);

          innerFor.push_back(For(st, iter, st->getInit()->getEndLoc(), st->getEndLoc()));
          //llvm::errs().changeColor(raw_ostream::GREEN);
          //llvm::errs() << "For Body\n";
          //st->getBody()->dumpColor();
          //llvm::errs() << "---------------------------------------------------\n";
          //llvm::errs().changeColor(raw_ostream::WHITE);
          //TraverseStmt(st->getBody());
          //innerFor.pop_back();
        } else {
          if(!insideLoop)
            loopEnd = st->getEndLoc();
          insideLoop = true;
        }
        return true;
      }

      virtual bool VisitStmt(Stmt *st) {
        //if(insideKernel) {
          //llvm::errs().changeColor(raw_ostream::RED);
          //llvm::errs() << "VisitStmt\n";
          //st->dumpColor();
          //llvm::errs() << "######$$$$$$$$$$$$###########$$$$$$$$$$$##########$\n";
          //llvm::errs().changeColor(raw_ostream::WHITE);
        //}
        if(insideLoop && isBefore(loopEnd, st->getBeginLoc())) {
          insideLoop = false;
        }

        bool found = false;
        //found |= kernel_found<OMPForDirective>(st);
        //found |= kernel_found<OMPParallelForDirective>(st);
        found |= kernel_found<OMPTargetTeamsDistributeParallelForDirective>(st);
        found |= kernel_found<OMPTargetTeamsDistributeDirective>(st);
        found |= kernel_found<OMPTargetDirective>(st);
        //found |= kernel_found<OMPTeamsDistributeDirective>(st);

        //if(found) llvm::errs() << "Kernel Found\n";

        if(found) {
          int id = lastKernel ? lastKernel->getID() + 1 : 1;
          lastKernel = new Kernel(id, st, currentFunction);
          llvm::errs().changeColor(raw_ostream::GREEN);
          llvm::errs() << "INFO: Kernel found at ";
          st->getBeginLoc().print(llvm::errs(), *SM);
          llvm::errs() << "\n";
          llvm::errs().changeColor(raw_ostream::WHITE);
          //st->dumpColor();
          //llvm::errs() << "////////////////////////////////////////////////////////////////\n";

          insideKernel = true;
          bool ret;
          OMPLoopDirective *omp = dyn_cast<OMPLoopDirective>(st);
          if(omp) {
            unsigned long total_to = 0;
            unsigned long total_from = 0;
            for(auto *M : omp->getClausesOfKind<OMPMapClause>()) {
              //////////////////////////////////////ALOK
              //llvm::errs() << "Map Clause found\n";
              /*switch(M->getMapType()) {
                case OMPC_MAP_to: llvm::errs() << "Map to\n"; break;
                case OMPC_MAP_tofrom: llvm::errs() << "Map tofrom\n"; break;
                case OMPC_MAP_from:
                case OMPC_MAP_alloc:
                case OMPC_MAP_delete:
                case OMPC_MAP_release:
                case OMPC_MAP_unknown:
                break;
                }*/
              for(const auto *EI : M->getVarRefs()) {
                const Stmt *E = (M->getMapLoc().isValid()) ? EI : nullptr;
                E->dumpColor();
                int start=-1, end;
                for(auto *i : E->children()) {
                  const Stmt *S = i;
                  if(S == NULL) {
                    continue;
                  }
                  if(const IntegerLiteral *I = dyn_cast<IntegerLiteral>(S)) {
                    //I->dump();
                    if(start == -1)
                      start = I->getValue().getLimitedValue(INT_MAX);
                    else
                      end = I->getValue().getLimitedValue(INT_MAX);
                    //llvm::errs() << "------------------\n";
                  }
                }

                if(M->getMapType() == OMPC_MAP_to)
                  total_to += sizeof(double)*(end-start);
                else if(M->getMapType() == OMPC_MAP_from)
                  total_from += sizeof(double)*(end-start);
                else if(M->getMapType() == OMPC_MAP_tofrom) {
                  total_to += sizeof(double)*(end-start);
                  total_from += sizeof(double)*(end-start);
                }
                //llvm::errs() << "==================\n";
              }
              //M->dump();
            }
            llvm::errs() << "Total size transferred = " << total_to << "+" << total_from << "\n";
            lastKernel->setMemTo(total_to);
            lastKernel->setMemFrom(total_from);

            // Get collapse
            llvm::errs().changeColor(raw_ostream::GREEN);
            //llvm::errs() << "INFO: Collpase - " << omp->getLoopsNumber() << "\n";
            llvm::errs().changeColor(raw_ostream::WHITE);
            /*for(unsigned int i = 0; i<omp->getNumClauses(); i++) {
              if(auto collapse = dyn_cast<OMPCollapseClause>(omp->getClause(i))) {
                Expr *ex = dyn_cast<Expr>(collapse->getNumForLoops());
                Expr::EvalResult Result;
                ex->EvaluateAsInt(Result, *astContext);
                llvm::errs().changeColor(raw_ostream::GREEN);
                llvm::errs() << "INFO: Kernel has collapse - " << Result.Val.getInt().getLimitedValue() << "\n";
                llvm::errs().changeColor(raw_ostream::WHITE);
              }
            }*/

            // Getting number of iterations
            Expr::EvalResult result;
            omp->getNumIterations()->EvaluateAsInt(result, *astContext);
            if(result.Val.isInt()) {
              lastKernel->setNumIteration(result.Val.getInt().getLimitedValue());
              llvm::errs().changeColor(raw_ostream::GREEN);
              //llvm::errs() << "INFO: Iteration = " << result.Val.getInt().getLimitedValue() << "\n";
              llvm::errs().changeColor(raw_ostream::WHITE);
            } else {
              llvm::errs().changeColor(raw_ostream::RED);
              llvm::errs() << "ERROR: Expecting static int value in OMP Parallel for at ";
              st->getBeginLoc().print(llvm::errs(), *SM);
              llvm::errs() << "\n";
              llvm::errs().changeColor(raw_ostream::WHITE);
              return false;
            }

            //llvm::errs().changeColor(raw_ostream::YELLOW);
            //llvm::errs() << "Entering TraverseStmt in loop\n"; 
            //llvm::errs().changeColor(raw_ostream::WHITE);
            //omp->getBody()->dumpColor();
            //llvm::errs() << "=========================================================================================\n"; 
            ret = TraverseStmt(omp->getBody());
            //llvm::errs().changeColor(raw_ostream::YELLOW);
            //llvm::errs() << "Exiting TraverseStmt in loop" << result.Val.getInt().getLimitedValue() << "\n";
            //llvm::errs().changeColor(raw_ostream::WHITE);
          } else {
            //llvm::errs().changeColor(raw_ostream::YELLOW);
            //llvm::errs() << "Entering TraverseStmt\n"; 
            //llvm::errs().changeColor(raw_ostream::WHITE);
            //dyn_cast<OMPExecutableDirective>(st)->getAssociatedStmt()->dumpColor();
            ret = TraverseStmt(dyn_cast<OMPExecutableDirective>(st)->getAssociatedStmt());
            //llvm::errs().changeColor(raw_ostream::YELLOW);
            //llvm::errs() << "Exiting TraverseStmt" << "\n";
            //llvm::errs().changeColor(raw_ostream::WHITE);
          }
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
          //llvm::errs() << "Kernel, Iter, VarDecl, refExpr, intLiteral, floatLiteral, mem_to, mem_from, add_sub_int, add_sub_double, mul_int, mul_double, div_int, div_double, bit_int, bit_double, rel_int, rel_double, logical_int, logical_double, assign_int, assign_double\n";
          llvm::errs() << "Kernel, Iter, VarDecl, refExpr, intLiteral, floatLiteral, mem_to, mem_from, add_sub_int, add_sub_double, mul_int, mul_double, div_int, div_double, assign_int, assign_double\n";
          lastKernel->dump();
        } else {
          if(insideKernel) {
            //llvm::errs().changeColor(raw_ostream::YELLOW);
            //llvm::errs() << "Inside Kernel - " << innerFor.size() << "\n";
            //llvm::errs().changeColor(raw_ostream::WHITE);

            if(innerFor.size() > 0 && isBefore(innerFor.back().getEndLocation(), st->getBeginLoc())) {
              innerFor.pop_back();
            }
            int counter = 1;
            for(unsigned int i=0; i<innerFor.size(); i++) {
              if(isBefore(st->getBeginLoc(), innerFor[i].getStartLocation())) continue;
              counter *= innerFor[i].getNumIteration();
              llvm::errs().changeColor(raw_ostream::GREEN);
              //llvm::errs() << "Iter - " << innerFor[i].getNumIteration() << "\n";
              llvm::errs().changeColor(raw_ostream::WHITE);

            }
            //if(counter > 1) counter--;

            llvm::errs().changeColor(raw_ostream::YELLOW);
            //llvm::errs() << "Entering incrStmt - " << counter << " " << innerFor.size() << ": ";
            //st->getBeginLoc().print(llvm::errs(), *SM);
            //llvm::errs() << "\n";
            llvm::errs().changeColor(raw_ostream::WHITE);
            lastKernel->incrStmt(st, counter);
            llvm::errs().changeColor(raw_ostream::YELLOW);
            //llvm::errs() << "Leaving\n";
            llvm::errs().changeColor(raw_ostream::WHITE);
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
      //      for (unsigned i = 0, e = args.size(); i != e; ++i) {
      //        if(args[i] == "-debug") {
      //          debug = true;
      //        }
      //      }
      return true;
    }
};

static FrontendPluginRegistry::Add<PluginInstructionCountAction>
X("-inst-count", "Plugin to count all instrcutions");
