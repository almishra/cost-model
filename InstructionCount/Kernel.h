#ifndef INSTRUCTIONCOUNT_KERNEL_H
#define INSTRUCTIONCOUNT_KERNEL_H

class Kernel {
  int id;
  bool inLoop;
  clang::Stmt *st;
  clang::Stmt *loop;
  clang::FunctionDecl *FD;

  int numIteration;

  int varDecl;
  int refExpr;
  int intLiteral;
  int floatLiteral;
  int fpLiteral;
  int charLiteral;
  int funcCall;

  enum TYPE { I8, I16, I32, I64, F32, F64, NONE }; 

  int add[7];
  int sub[7];
  int mul[7];
  int div[7];
  int rem[7];
  int shl[7];
  int shr[7];
  int lt[7];
  int le[7];
  int gt[7];
  int ge[7];
  int eq[7];
  int ne[7];
  int And[7];
  int Xor[7];
  int Or[7];
  int lAnd[7];
  int lOr[7];
  int assign[7];
  int mulAssign[7];
  int divAssign[7];
  int remAssign[7];
  int addAssign[7];
  int subAssign[7];
  int shlAssign[7];
  int shrAssign[7];
  int andAssign[7];
  int xorAssign[7];
  int orAssign[7];
  int comma[7];

  int postInc[7];
  int postDec[7];
  int preInc[7];
  int preDec[7];
  int addrOf[7];
  int deRef[7];
  int plus[7];
  int minus[7];
  int Not[7];
  int lNot[7];

  public:
  Kernel(int ID, clang::Stmt *stmt, clang::FunctionDecl *F) : id(ID), st(stmt), FD(F) {
    inLoop = false;
    loop = NULL;

    numIteration = 0;

    varDecl = 0;
    refExpr = 0;
    intLiteral = 0;
    floatLiteral = 0;
    fpLiteral = 0;
    charLiteral = 0;
    funcCall = 0;

    for(int i=I8; i<=NONE; i++) {
      add[i] = 0;
      sub[i] = 0;
      mul[i] = 0;
      div[i] = 0;
      rem[i] = 0;
      shl[i] = 0;
      shr[i] = 0;
      lt[i] = 0;
      le[i] = 0;
      gt[i] = 0;
      ge[i] = 0;
      eq[i] = 0;
      ne[i] = 0;
      And[i] = 0;
      Xor[i] = 0;
      Or[i] = 0;
      lAnd[i] = 0;
      lOr[i] = 0;
      assign[i] = 0;
      mulAssign[i] = 0;
      divAssign[i] = 0;
      remAssign[i] = 0;
      addAssign[i] = 0;
      subAssign[i] = 0;
      shlAssign[i] = 0;
      shrAssign[i] = 0;
      andAssign[i] = 0;
      xorAssign[i] = 0;
      orAssign[i] = 0;
      comma[i] = 0;
      postInc[i] = 0;
      postDec[i] = 0;
      preInc[i] = 0;
      preDec[i] = 0;
      addrOf[i] = 0;
      deRef[i] = 0;
      plus[i] = 0;
      minus[i] = 0;
      Not[i] = 0;
      lNot[i] = 0;
    }
  }

  int getID() { return id; }
  void setInLoop(bool in) { inLoop = in; }
  bool isInLoop() { return inLoop; }
  clang::Stmt *getStmt() { return st; }
  clang::Stmt *getLoop() { return loop; }
  void setLoop(clang::Stmt *l) { loop = l; }
  clang::FunctionDecl *getFunction() { return FD; }
  void setFuction(clang::FunctionDecl *F) { FD = F; }

  void setNumIteration(int num) { numIteration = num; }
  int getNumIteration() { return numIteration; }

  void incrStmt(clang::Stmt *st, int count) {
    TYPE type = NONE;
    const clang::BuiltinType *T;// = llvm::dyn_cast<clang::BuiltinType>(st->getType());
    if(auto b = llvm::dyn_cast<clang::BinaryOperator>(st)) 
      T = llvm::dyn_cast<clang::BuiltinType>(b->getType());
    if(auto u = llvm::dyn_cast<clang::UnaryOperator>(st))
      T = llvm::dyn_cast<clang::BuiltinType>(u->getType());
    switch (T->getKind()) {
      case clang::BuiltinType::Void:
      case clang::BuiltinType::Bool:
      case clang::BuiltinType::Char_S:
      case clang::BuiltinType::Char_U:
      case clang::BuiltinType::Char8:
      case clang::BuiltinType::UChar:
      case clang::BuiltinType::SChar:
        type = I8;
        break;
      case clang::BuiltinType::Char16:
      case clang::BuiltinType::UShort:
      case clang::BuiltinType::Short:
        type = I16;
        break;
      case clang::BuiltinType::WChar_S:
      case clang::BuiltinType::WChar_U:
      case clang::BuiltinType::Char32:
      case clang::BuiltinType::UInt:
      case clang::BuiltinType::Int:
        type = I32;
        break;
      case clang::BuiltinType::ULongLong:
      case clang::BuiltinType::LongLong:
      case clang::BuiltinType::ULong:
      case clang::BuiltinType::Long:
        type = I64;
        break;
      case clang::BuiltinType::Float:
        type = F32;
        break;
      case clang::BuiltinType::Double:
        type = F64;
        break;
      case clang::BuiltinType::Int128:
      case clang::BuiltinType::UInt128:
      case clang::BuiltinType::ShortAccum:
      case clang::BuiltinType::UShortAccum:
      case clang::BuiltinType::SatShortAccum:
      case clang::BuiltinType::SatUShortAccum:
      case clang::BuiltinType::Accum:
      case clang::BuiltinType::UAccum:
      case clang::BuiltinType::SatAccum:
      case clang::BuiltinType::SatUAccum:
      case clang::BuiltinType::LongAccum:
      case clang::BuiltinType::ULongAccum:
      case clang::BuiltinType::SatLongAccum:
      case clang::BuiltinType::SatULongAccum:
      case clang::BuiltinType::ShortFract:
      case clang::BuiltinType::UShortFract:
      case clang::BuiltinType::SatShortFract:
      case clang::BuiltinType::SatUShortFract:
      case clang::BuiltinType::Fract:
      case clang::BuiltinType::UFract:
      case clang::BuiltinType::SatFract:
      case clang::BuiltinType::SatUFract:
      case clang::BuiltinType::LongFract:
      case clang::BuiltinType::ULongFract:
      case clang::BuiltinType::SatLongFract:
      case clang::BuiltinType::SatULongFract:
      case clang::BuiltinType::BFloat16:
      case clang::BuiltinType::Float16:
      case clang::BuiltinType::Half:
      case clang::BuiltinType::LongDouble:
      case clang::BuiltinType::Float128:
      case clang::BuiltinType::NullPtr:
      case clang::BuiltinType::ObjCId:
      case clang::BuiltinType::ObjCClass:
      case clang::BuiltinType::ObjCSel:
      case clang::BuiltinType::OCLSampler:
      case clang::BuiltinType::OCLEvent:
      case clang::BuiltinType::OCLClkEvent:
      case clang::BuiltinType::OCLQueue:
      case clang::BuiltinType::OCLReserveID:
      default:
        break;
    }
    switch(st->getStmtClass()) {
      case clang::Stmt::CompoundAssignOperatorClass:
      case clang::Stmt::BinaryOperatorClass: {
        clang::BinaryOperator *bin = llvm::dyn_cast<clang::BinaryOperator>(st);
        clang::BinaryOperatorKind o = bin->getOpcode();
        switch(o) {
          case clang::BO_Mul: mul[type]+=count; break;
          case clang::BO_Div: div[type]+=count; break;
          case clang::BO_Rem: rem[type]+=count; break;
          case clang::BO_Add: add[type]+=count; break;
          case clang::BO_Sub: sub[type]+=count; break;
          case clang::BO_Shl: shl[type]+=count; break;
          case clang::BO_Shr: shr[type]+=count; break;
          case clang::BO_LT: lt[type]+=count; break;
          case clang::BO_LE: le[type]+=count; break;
          case clang::BO_GT: gt[type]+=count; break;
          case clang::BO_GE: ge[type]+=count; break;
          case clang::BO_EQ: eq[type]+=count; break;
          case clang::BO_NE: ne[type]+=count; break;
          case clang::BO_And: And[type]+=count; break;
          case clang::BO_Xor: Xor[type]+=count; break;
          case clang::BO_Or: Or[type]+=count; break;
          case clang::BO_LAnd: lAnd[type]+=count; break;
          case clang::BO_LOr: lOr[type]+=count; break;
          case clang::BO_Assign: assign[type]+=count; break;
          case clang::BO_MulAssign: mulAssign[type]+=count; break;
          case clang::BO_DivAssign: divAssign[type]+=count; break;
          case clang::BO_RemAssign: remAssign[type]+=count; break;
          case clang::BO_AddAssign: addAssign[type]+=count; break;
          case clang::BO_SubAssign: subAssign[type]+=count; break;
          case clang::BO_ShlAssign: shlAssign[type]+=count; break;
          case clang::BO_ShrAssign: shrAssign[type]+=count; break;
          case clang::BO_AndAssign: andAssign[type]+=count; break;
          case clang::BO_XorAssign: xorAssign[type]+=count; break;
          case clang::BO_OrAssign: orAssign[type]+=count; break;
          case clang::BO_Comma: comma[type]+=count; break;
          default:  break;
        }
        break;
      }
      case clang::Stmt::UnaryOperatorClass: {
        clang::UnaryOperator *un = llvm::dyn_cast<clang::UnaryOperator>(st);
        clang::UnaryOperatorKind o = un->getOpcode();
        switch(o) {
          case clang::UO_PostInc: postInc[type]+=count; break;
          case clang::UO_PostDec: postDec[type]+=count; break;
          case clang::UO_PreInc: preInc[type]+=count; break;
          case clang::UO_PreDec: preDec[type]+=count; break;
          case clang::UO_AddrOf: addrOf[type]+=count; break;
          case clang::UO_Deref: deRef[type]+=count; break;
          case clang::UO_Plus: plus[type]+=count; break;
          case clang::UO_Minus: minus[type]+=count; break;
          case clang::UO_Not: Not[type]+=count; break;
          case clang::UO_LNot: lNot[type]+=count; break;
          default: break;
        }
        break;
      }
      case clang::Stmt::IntegerLiteralClass: intLiteral+=count; break;
      case clang::Stmt::FloatingLiteralClass: floatLiteral+=count; break;
      case clang::Stmt::FixedPointLiteralClass: fpLiteral+=count; break;
      case clang::Stmt::CharacterLiteralClass: charLiteral+=count; break;
      case clang::Stmt::CallExprClass: funcCall+=count; break;
      case clang::Stmt::DeclRefExprClass: refExpr+=count; break;
      case clang::Stmt::DeclStmtClass: varDecl+=count; break;
      default: break;
    }
  }

  void print() {
    llvm::errs() << "Total Number of Iterations: " << numIteration << "\n\n";
    llvm::errs() << "varDecl : " << varDecl << "\n";
    llvm::errs() << "refExpr  : " << refExpr << "\n";
    llvm::errs() << "intLiteral  : " << intLiteral << "\n";
    llvm::errs() << "floatLiteral  : " << floatLiteral << "\n";
    llvm::errs() << "fpLiteral  : " << fpLiteral << "\n";
    llvm::errs() << "charLiteral  : " << charLiteral << "\n";
    llvm::errs() << "funcCall  : " << funcCall << "\n\n";

    char *STR[7] = { "i8", "i16", "i32", "i64", "f32", "f64", "NONE" };
    //for(int i=I8; i<=NONE; i++) {
    for(int i=I8; i<=NONE; i++)
      llvm::errs() << "add_" << STR[i] << "  : " << add[i] << "\n";

    for(int i=I8; i<=NONE; i++)
      llvm::errs() << "sub_" << STR[i] << "  : " << sub[i] << "\n";

    for(int i=I8; i<=NONE; i++)
      llvm::errs() << "mul_" << STR[i] << "  : " << mul[i] << "\n";

    for(int i=I8; i<=NONE; i++)
      llvm::errs() << "div_" << STR[i] << "  : " << div[i] << "\n";

    for(int i=I8; i<=NONE; i++)
      llvm::errs() << "rem_" << STR[i] << "  : " << rem[i] << "\n";

    for(int i=I8; i<=NONE; i++)
      llvm::errs() << "shl_" << STR[i] << "  : " << shl[i] << "\n";

    for(int i=I8; i<=NONE; i++)
      llvm::errs() << "shr_" << STR[i] << "  : " << shr[i] << "\n";

    for(int i=I8; i<=NONE; i++)
      llvm::errs() << "lt_" << STR[i] << "  : " << lt[i] << "\n";

    for(int i=I8; i<=NONE; i++)
      llvm::errs() << "le_" << STR[i] << "  : " << le[i] << "\n";

    for(int i=I8; i<=NONE; i++)
      llvm::errs() << "gt_" << STR[i] << "  : " << gt[i] << "\n";

    for(int i=I8; i<=NONE; i++)
      llvm::errs() << "ge_" << STR[i] << "  : " << ge[i] << "\n";

    for(int i=I8; i<=NONE; i++)
      llvm::errs() << "eq_" << STR[i] << "  : " << eq[i] << "\n";

    for(int i=I8; i<=NONE; i++)
      llvm::errs() << "ne_" << STR[i] << "  : " << ne[i] << "\n";

    for(int i=I8; i<=NONE; i++)
      llvm::errs() << "And_" << STR[i] << "  : " << And[i] << "\n";

    for(int i=I8; i<=NONE; i++)
      llvm::errs() << "Xor_" << STR[i] << "  : " << Xor[i] << "\n";

    for(int i=I8; i<=NONE; i++)
      llvm::errs() << "Or_" << STR[i] << "  : " << Or[i] << "\n";

    for(int i=I8; i<=NONE; i++)
      llvm::errs() << "lAnd_" << STR[i] << "  : " << lAnd[i] << "\n";

    for(int i=I8; i<=NONE; i++)
      llvm::errs() << "lOr_" << STR[i] << "  : " << lOr[i] << "\n";

    for(int i=I8; i<=NONE; i++)
      llvm::errs() << "assign_" << STR[i] << "  : " << assign[i] << "\n";

    for(int i=I8; i<=NONE; i++)
      llvm::errs() << "mulAssign_" << STR[i] << "  : " << mulAssign[i] << "\n";

    for(int i=I8; i<=NONE; i++)
      llvm::errs() << "divAssign_" << STR[i] << "  : " << divAssign[i] << "\n";

    for(int i=I8; i<=NONE; i++)
      llvm::errs() << "remAssign_" << STR[i] << "  : " << remAssign[i] << "\n";

    for(int i=I8; i<=NONE; i++)
      llvm::errs() << "addAssign_" << STR[i] << "  : " << addAssign[i] << "\n";

    for(int i=I8; i<=NONE; i++)
      llvm::errs() << "subAssign_" << STR[i] << "  : " << subAssign[i] << "\n";

    for(int i=I8; i<=NONE; i++)
      llvm::errs() << "shlAssign_" << STR[i] << "  : " << shlAssign[i] << "\n";

    for(int i=I8; i<=NONE; i++)
      llvm::errs() << "shrAssign_" << STR[i] << "  : " << shrAssign[i] << "\n";

    for(int i=I8; i<=NONE; i++)
      llvm::errs() << "andAssign_" << STR[i] << "  : " << andAssign[i] << "\n";

    for(int i=I8; i<=NONE; i++)
      llvm::errs() << "xorAssign_" << STR[i] << "  : " << xorAssign[i] << "\n";

    for(int i=I8; i<=NONE; i++)
      llvm::errs() << "orAssign_" << STR[i] << "  : " << orAssign[i] << "\n";

    for(int i=I8; i<=NONE; i++)
      llvm::errs() << "comma_" << STR[i] << "  : " << comma[i] << "\n";

    for(int i=I8; i<=NONE; i++)
      llvm::errs() << "postInc_" << STR[i] << "  : " << postInc[i] << "\n";

    for(int i=I8; i<=NONE; i++)
      llvm::errs() << "postDec_" << STR[i] << "  : " << postDec[i] << "\n";

    for(int i=I8; i<=NONE; i++)
      llvm::errs() << "preInc_" << STR[i] << "  : " << preInc[i] << "\n";

    for(int i=I8; i<=NONE; i++)
      llvm::errs() << "preDec_" << STR[i] << "  : " << preDec[i] << "\n";

    for(int i=I8; i<=NONE; i++)
      llvm::errs() << "addrOf_" << STR[i] << "  : " << addrOf[i] << "\n";

    for(int i=I8; i<=NONE; i++)
      llvm::errs() << "deRef_" << STR[i] << "  : " << deRef[i] << "\n";

    for(int i=I8; i<=NONE; i++)
      llvm::errs() << "plus_" << STR[i] << "  : " << plus[i] << "\n";

    for(int i=I8; i<=NONE; i++)
      llvm::errs() << "minus_" << STR[i] << "  : " << minus[i] << "\n";

    for(int i=I8; i<=NONE; i++)
      llvm::errs() << "Not_" << STR[i] << "  : " << Not[i] << "\n";

    for(int i=I8; i<=NONE; i++)
      llvm::errs() << "lNot_" << STR[i] << "  : " << lNot[i] << "\n";
    //}
  }

  // Overloading operator < for sorting of keys in map
  bool operator< (const Kernel& k) const {
    if(k.id < this->id) return true;
    return false;
  }
};

#endif // End of header
