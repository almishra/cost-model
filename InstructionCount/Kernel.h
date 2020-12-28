#ifndef DATAREUSE_KERNEL_H                                                      
#define DATAREUSE_KERNEL_H                                                      
#include "clang/Frontend/FrontendPluginRegistry.h"                              

class Kernel {                                                                  
  int id;                                                                     
  clang::Stmt *st;                                                                   
  bool inLoop;                                                                
  clang::Stmt *loop;                                                                 
  clang::FunctionDecl *FD;                                                           

  int iteration;                                                              

  int varDecl;                                                                
  int refExpr;                                                                
  int intLiteral;                                                             
  int floatLiteral;                                                           
  int fpLiteral;                                                              
  int charLiteral;                                                            
  int funcCall;                                                               

  int add;                                                                    
  int sub;                                                                    
  int mul;                                                                    
  int div;                                                                    
  int rem;                                                                    
  int shl;                                                                    
  int shr;                                                                    
  int lt;                                                                     
  int le;                                                                     
  int gt;                                                                     
  int ge;                                                                     
  int eq;                                                                     
  int ne;                                                                     
  int And;                                                                    
  int Xor;                                                                    
  int Or;                                                                     
  int lAnd;                                                                   
  int lOr;                                                                    
  int assign;                                                                 
  int mulAssign;                                                              
  int divAssign;                                                              
  int remAssign;                                                              
  int addAssign;                                                              
  int subAssign;                                                              
  int shlAssign;                                                              
  int shrAssign;                                                              
  int andAssign;                                                              
  int xorAssign;                                                              
  int orAssign;
  int comma;                                                                  

  int postInc;                                                                
  int postDec;                                                                
  int preInc;                                                                 
  int preDec;                                                                 
  int addrOf;                                                                 
  int deRef;                                                                  
  int plus;                                                                   
  int minus;                                                                  
  int Not;                                                                    
  int lNot;                                                                   

  public:                                                                         
  Kernel(int ID, clang::Stmt *stmt, clang::FunctionDecl *F) : id(ID), st(stmt), FD(F) {     
    inLoop = false;                                                         
    loop = NULL;                                                            

    iteration = 0;                                                          

    varDecl = 0;                                                            
    refExpr = 0;                                                            
    intLiteral = 0;                                                         
    floatLiteral = 0;                                                       
    fpLiteral = 0;                                                          
    charLiteral = 0;                                                        
    funcCall = 0;                                                           

    add = 0;                                                                
    sub = 0;                                                                
    mul = 0;                                                                
    div = 0;                                                                
    rem = 0;                                                                
    shl = 0;                                                                
    shr = 0;                                                                
    lt = 0;                                                                 
    le = 0;                                                                 
    gt = 0;                                                                 
    ge = 0;                                                                 
    eq = 0;                                                                 
    ne = 0;                                                                 
    And = 0;                                                                
    Xor = 0;                                                                
    Or = 0;                                                                 
    lAnd = 0;                                                               
    lOr = 0;                                                                
    assign = 0;                                                             
    mulAssign = 0;                                                          
    divAssign = 0;                                                          
    remAssign = 0;                                                          
    addAssign = 0;                                                          
    subAssign = 0;
    shlAssign = 0;                                                          
    shrAssign = 0;                                                          
    andAssign = 0;                                                          
    xorAssign = 0;                                                          
    orAssign = 0;                                                           
    comma = 0;                                                              

    postInc = 0;                                                            
    postDec = 0;                                                            
    preInc = 0;                                                             
    preDec = 0;                                                             
    addrOf = 0;                                                             
    deRef = 0;                                                              
    plus = 0;                                                               
    minus = 0;                                                              
    Not = 0;                                                                
    lNot = 0;                                                               
  };                                                                          

  int getID() { return id; }                                                  
  void setInLoop(bool in) { inLoop = in; }                                    
  bool isInLoop() { return inLoop; }                                          
  clang::Stmt *getStmt() { return st; }                                              
  clang::Stmt *getLoop() { return loop; }                                            
  void setLoop(clang::Stmt *l) { loop = l; }                                         
  clang::FunctionDecl *getFunction() { return FD; }                                  
  void setFuction(clang::FunctionDecl *F) { FD = F; }                                

  void setIteration(int num) { iteration = num; }                             
  int getIteration() { return iteration; }                                    

  void incrStmt(clang::Stmt *st, int count) {                                        
    switch(st->getStmtClass()) {                                              
      case clang::Stmt::BinaryOperatorClass: {                                       
        clang::BinaryOperator *bin = llvm::dyn_cast<clang::BinaryOperator>(st);                   
        clang::BinaryOperatorKind o = bin->getOpcode();                              
        switch(o) {                                                           
          case clang::BO_Mul: mul+=count; break;                                     
          case clang::BO_Div: div+=count; break;                                     
          case clang::BO_Rem: rem+=count; break;                                     
          case clang::BO_Add: add+=count; break;                                     
          case clang::BO_Sub: sub+=count; break;
          case clang::BO_Shl: shl+=count; break;                                     
          case clang::BO_Shr: shr+=count; break;                                     
          case clang::BO_LT: lt+=count; break;                                       
          case clang::BO_LE: le+=count; break;                                       
          case clang::BO_GT: gt+=count; break;                                       
          case clang::BO_GE: ge+=count; break;                                       
          case clang::BO_EQ: eq+=count; break;                                       
          case clang::BO_NE: ne+=count; break;                                       
          case clang::BO_And: And+=count; break;                                     
          case clang::BO_Xor: Xor+=count; break;                                     
          case clang::BO_Or: Or+=count; break;                                       
          case clang::BO_LAnd: lAnd+=count; break;                                   
          case clang::BO_LOr: lOr+=count; break;                                     
          case clang::BO_Assign: assign+=count; break;                               
          case clang::BO_MulAssign: mulAssign+=count; break;                         
          case clang::BO_DivAssign: divAssign+=count; break;                         
          case clang::BO_RemAssign: remAssign+=count; break;                         
          case clang::BO_AddAssign: addAssign+=count; break;                         
          case clang::BO_SubAssign: subAssign+=count; break;                         
          case clang::BO_ShlAssign: shlAssign+=count; break;                         
          case clang::BO_ShrAssign: shrAssign+=count; break;                         
          case clang::BO_AndAssign: andAssign+=count; break;                         
          case clang::BO_XorAssign: xorAssign+=count; break;                         
          case clang::BO_OrAssign: orAssign+=count; break;                           
          case clang::BO_Comma: comma+=count; break;                                 
          default:  break;                                                    
        }                                                                     
        break;                                                                
      }                                                                       
      case clang::Stmt::UnaryOperatorClass: {                                        
        clang::UnaryOperator *un = llvm::dyn_cast<clang::UnaryOperator>(st);                      
        clang::UnaryOperatorKind o = un->getOpcode();                                
        switch(o) {                                                           
          case clang::UO_PostInc: postInc+=count; break;                             
          case clang::UO_PostDec: postDec+=count; break;                             
          case clang::UO_PreInc: preInc+=count; break;                               
          case clang::UO_PreDec: preDec+=count; break;                               
          case clang::UO_AddrOf: addrOf+=count; break;                               
          case clang::UO_Deref: deRef+=count; break;                                 
          case clang::UO_Plus: plus+=count; break;                                   
          case clang::UO_Minus: minus+=count; break;                                 
          case clang::UO_Not: Not+=count; break;                                     
          case clang::UO_LNot: lNot+=count; break;                                   
          default: break;                                                     
        }                                                                     
        break;                                                                
      }                                                                       
      case clang::Stmt::IntegerLiteralClass: intLiteral+=count; break;               
      case clang::Stmt::FloatingLiteralClass: floatLiteral+=count; break;            
      case clang::Stmt::FixedPointLiteralClass: fpLiteral+=count; break;             
      case clang::Stmt::CharacterLiteralClass: charLiteral+=count; break;            
      case clang::Stmt::CallExprClass: funcCall+=count; break;
      default: break;                                                         
    }                                                                         
  }                                                                           

  void print() {                                                              
    llvm::errs() << "Total Iterations: " << iteration << "\n\n";              
    llvm::errs() << "varDecl : " << varDecl << "\n";                          
    llvm::errs() << "refExpr  : " << refExpr << "\n";                         
    llvm::errs() << "intLiteral  : " << intLiteral << "\n";                   
    llvm::errs() << "floatLiteral  : " << floatLiteral << "\n";               
    llvm::errs() << "fpLiteral  : " << fpLiteral << "\n";                     
    llvm::errs() << "charLiteral  : " << charLiteral << "\n";                 
    llvm::errs() << "funcCall  : " << funcCall << "\n\n";                     

    llvm::errs() << "add  : " << add << "\n";                                 
    llvm::errs() << "sub  : " << sub << "\n";                                 
    llvm::errs() << "mul  : " << mul << "\n";                                 
    llvm::errs() << "div  : " << div << "\n";                                 
    llvm::errs() << "rem  : " << rem << "\n";                                 
    llvm::errs() << "shl  : " << shl << "\n";                                 
    llvm::errs() << "shr  : " << shr << "\n";                                 
    llvm::errs() << "lt  : " << lt << "\n";                                   
    llvm::errs() << "le  : " << le << "\n";                                   
    llvm::errs() << "gt  : " << gt << "\n";                                   
    llvm::errs() << "ge  : " << ge << "\n";                                   
    llvm::errs() << "eq  : " << eq << "\n";                                   
    llvm::errs() << "ne  : " << ne << "\n";                                   
    llvm::errs() << "And : " << And << "\n";                                  
    llvm::errs() << "Xor : " << Xor << "\n";                                  
    llvm::errs() << "Or : " << Or << "\n";                                    
    llvm::errs() << "lAnd : " << lAnd << "\n";                                
    llvm::errs() << "lOr : " << lOr << "\n";                                  
    llvm::errs() << "assign : " << assign << "\n";                            
    llvm::errs() << "mulAssign : " << mulAssign << "\n";                      
    llvm::errs() << "divAssign : " << divAssign << "\n";                      
    llvm::errs() << "remAssign : " << remAssign << "\n";                      
    llvm::errs() << "addAssign : " << addAssign << "\n";                      
    llvm::errs() << "subAssign : " << subAssign << "\n";                      
    llvm::errs() << "shlAssign : " << shlAssign << "\n";                      
    llvm::errs() << "shrAssign : " << shrAssign << "\n";                      
    llvm::errs() << "andAssign : " << andAssign << "\n";                      
    llvm::errs() << "xorAssign : " << xorAssign << "\n";                      
    llvm::errs() << "orAssign : " << orAssign << "\n";                        
    llvm::errs() << "comma : " << comma << "\n\n";                            

    llvm::errs() << "postInc : " << postInc << "\n";                          
    llvm::errs() << "postDec : " << postDec << "\n";                          
    llvm::errs() << "preInc : " << preInc << "\n";                            
    llvm::errs() << "preDec : " << preDec << "\n";                            
    llvm::errs() << "addrOf : " << addrOf << "\n";                            
    llvm::errs() << "deRef : " << deRef << "\n";                              
    llvm::errs() << "plus : " << plus << "\n";
    llvm::errs() << "minus : " << minus << "\n";                              
    llvm::errs() << "Not : " << Not << "\n";                                  
    llvm::errs() << "lNot : " << lNot << "\n";                                
  }                                                                           

  // Overloading operator < for sorting of keys in map                        
  bool operator< (const Kernel& k) const {                                    
    if(k.id < this->id) return true;                                        
    return false;                                                           
  }                                                                           
};

#endif
