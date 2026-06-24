/* ============== CHAPTER 2 - PARSER =================*/

#include "lexer.cpp"
#include <algorithm>
#include <cstdio>
#include <memory>
#include <string>
#include <utility>
#include <vector>

class ExprAST {
public:
  virtual ~ExprAST() = default;
};

class NumberExprAST : public ExprAST { // inherit from ExprAST to num
  double Val;

public:
  NumberExprAST(double V) : Val(V) {} // constructor + init
};

/// Expression class for referencing a var, like 'a,b,c.'
class VariableExprAST : public ExprAST {
  std::string Name;

  /* More efficient than just copying the data in memory and pushing it on the
   * stack. Saves memory and processing time. */
public:
  VariableExprAST(const std::string &Name) : Name(Name) {}
};

/// Expression class for a bin operation
class BinaryExprAST : public ExprAST {
  char Op; // + - * / > <
  std::unique_ptr<ExprAST> LHS, RHS;

public:
  BinaryExprAST(char Op, std::unique_ptr<ExprAST> LHS,
                std::unique_ptr<ExprAST> RHS)
      : Op(Op), LHS(std::move(LHS)), RHS(std::move(RHS)) {}
};

/* Expression class for function calls */
class CallExprAST : public ExprAST {
  std::string Calee;
  std::vector<std::unique_ptr<ExprAST>> Args;

public:
  // vector to unique ptr to expr nodes
  CallExprAST(const std::string &Calee,
              std::vector<std::unique_ptr<ExprAST>> Args)
      : Calee(Calee), Args(std::move(Args)) {}
};

/* This class represents the "prototype" for a function,
 which captures its name, and its argument names (thus implicitly the number
 of arguments the function takes). */
class PrototypeAST {
  std::string Name;
  std::vector<std::string> Args;

public:
  PrototypeAST(const std::string &Name, std::vector<std::string> Args)
      : Name(Name), Args(std::move(Args)) {}
  const std::string &getName() const { return Name; }
};

/* This class represents a function definition itself. */
class FunctionAST {
  std::unique_ptr<PrototypeAST> Proto;
  std::unique_ptr<ExprAST> Body;

public:
  FunctionAST(std::unique_ptr<PrototypeAST> Proto,
              std::unique_ptr<ExprAST> Body)
      : Proto(std::move(Proto)), Body(std::move(Body)) {}
};

/* CurTok/getNextToken - Provide a simple token buffer.  CurTok is the current
 token the parser is looking at.  getNextToken reads another token from the
 lexer and updates CurTok with its results */
static int CurTok;
static int getNextToken() { return CurTok = gettok(); }

/* LogError* - These are little helper functions for error handling.*/
std::unique_ptr<ExprAST> LogError(const char *Str) {
  fprintf(stderr, "Error: %s\n", Str);
  return nullptr;
}
std::unique_ptr<PrototypeAST> LogErrorP(const char *Str) {
  LogError(Str);
  return nullptr;
}

/* Basic expr parsing -> NumberExpre ::= number */
static std::unique_ptr<ExprAST> ParseNumberExpr() {
  auto Result = std::make_unique<NumberExprAST>(
      NumVal); // kinda like using 'new' for the NumberExprAST object
               // (instantiation) maintaining the smart pointer approach
  getNextToken();
  return std::move(Result); // move ownership from ParseNumberExpr to Result
}

/*Parenexpr ::= '(' [expression] ')' */
static std::unique_ptr<ExprAST> ParseParenExpr() {
  getNextToken();
  auto V = ParseExpression();
  if (!V) {
    return nullptr;
  }
  if (CurTok == ')') {
    getNextToken();
    return V;
  } else {
    return LogError("expected ')");
  }
}

/* identifierexpr
   ::= identifier
   ::= identifier '(' expression* ')'*/
static std::unique_ptr<ExprAST> ParseIdentifierOrCallExpr() {
  std::string IdName = IdentifierStr;
  getNextToken(); // eat identifier

  if (CurTok == '(') {
    getNextToken(); // eat (
    std::vector<std::unique_ptr<ExprAST>> Args;
    if (CurTok != ')') {
      while (true) {
        auto Arg = ParseExpression();
        if (Arg) {
          Args.push_back(std::move(Arg));
        } else {
          return nullptr;
        }
        if (CurTok == ')') {
          getNextToken(); // eat )
          break;
        } else if (CurTok == ',') {
          getNextToken();
          continue;
        } else {
          return LogError("Expected ')' or ',' in argument list");
        }
      }
    }
    return std::make_unique<CallExprAST>(IdName, std::move(Args));
  } else {
    return std::make_unique<VariableExprAST>(IdName);
  }
}

/* primary
   ::= identifierexpr
   ::= numberexpr
   ::= parenexpr */
static std::unique_ptr<ExprAST> ParsePrimary() {
  switch (CurTok) {
  case tok_identifier:
    return ParseIdentifierOrCallExpr();
  case tok_number:
    return ParseNumberExpr();
  case '(':
    return ParseParenExpr();
  default:
    return LogError("unknown token when expecting an expression");
  }
}

// GetTokPrecedence - Get the precedence of the pending binary operator token.
static int GetTokPrecedence() {
  switch (CurTok) {
  case '<':
  case '>':
    return 10;
  case '+':
  case '-':
    return 20;
  case '*':
  case '/':
    return 40;
  default:
    return -1;
  }
}

/* binoprhs
   ::= ('+' primary)* */
static std::unique_ptr<ExprAST> ParseBinOpRHS(int ExprPrec,
                                              std::unique_ptr<ExprAST> LHS) {
  while (true) {
    int TokPrec = GetTokPrecedence(); // '+','-', ...

    if (TokPrec < ExprPrec) { // 'ignore' next op and return LHS
      return LHS;
    } else {
      int BinOp = CurTok;
      getNextToken();            // eats binop
      auto RHS = ParsePrimary(); // parses 'primary expr' after 'binop'
      if (RHS) {
        int NextPrec = GetTokPrecedence();
        if (TokPrec < NextPrec) {
          RHS = ParseBinOpRHS(TokPrec + 1, std::move(RHS));
          if (!RHS) {
            return nullptr;
          }
        }
        LHS = std::make_unique<BinaryExprAST>(BinOp, std::move(LHS),
                                              std::move(RHS));
      } else {
        return nullptr;
      }
    }
  }
}

/* expression
   ::= primary binoprhs*/
static std::unique_ptr<ExprAST> ParseExpression() {
  auto LHS = ParsePrimary();
  if (LHS) {
    // initialize with 0 so any op will be recognized in GetTokPrecedence();
    return ParseBinOpRHS(0, std::move(LHS));
  } else {
    return nullptr;
  }
}

/* prototype
   ::= id '(' id* ')'*/
static std::unique_ptr<PrototypeAST> ParsePrototype() {
  // expecting first tok to be identifier otherwise throw error
  if (CurTok != tok_identifier) {
    return LogErrorP("Expected function name in prototype");
  }
  std::string FnName = IdentifierStr;
  getNextToken(); // eats identifier 'foo?'

  if (CurTok != '(') {
    return LogErrorP("Expected '(' in prototype");
  }
  std::vector<std::string> ArgNames; // instanciated
  while (getNextToken() == tok_identifier) {
    ArgNames.push_back(IdentifierStr);
  }
  if (CurTok != ')') {
    return LogErrorP("Expected ')' in prototype");
  }
  getNextToken();
  return std::make_unique<PrototypeAST>(FnName, std::move(ArgNames));
}

/* definition ::= 'def' prototype expression*/
static std::unique_ptr<FunctionAST> ParseDefinition() {
  getNextToken(); // eats def
  auto Proto = ParsePrototype();
  if (!Proto) {
    return nullptr;
  }

  auto E = ParseExpression();
  if (E) {
    return std::make_unique<FunctionAST>(std::move(Proto), std::move(E));
    return nullptr;
  }
}

/*external ::= 'extern' prototype*/
std::unique_ptr<PrototypeAST> PauseExtern() {
  getNextToken(); // eats extern (assumes it's CurTok)
  return ParsePrototype();
}

/* toplevelexpr ::= expression*/
std::unique_ptr<FunctionAST> ParseTopLevelExpr() {
  auto E = ParseExpression();
  if (E) {
    // wrap in implicit top level unnamed func + empty vector of strings
    // for its args
    auto Proto = std::make_unique<PrototypeAST>("__anon_expr",
                                                std::vector<std::string>());
    return std::make_unique<FunctionAST>(std::move(Proto), std::move(E));
  }
  return nullptr;
}

/* top ::= definition | external | expression | ';' */
static void MainLoop() {
  while (true) {
    fprintf(stderr, "ready> ");
    switch (CurTok) {
    case tok_eof:
      return;
    case ';': // ignore top-level semicolons.
      getNextToken();
      break;
    case tok_def:
      HandleDefinition();
      break;
    case tok_extern:
      HandleExtern();
      break;
    default:
      HandleTopLevelExpression();
      break;
    }
  }
}
