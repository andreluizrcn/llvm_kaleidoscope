/* ============== CHAPTER 2 - PARSER =================*/

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

/// Expression class for function calls
class CallExprAST : public ExprAST {
  std::string Calee;
  std::vector<std::unique_ptr<ExprAST>> Args;

public:
  CallExprAST(const std::string &Calee,
              std::vector<std::unique_ptr<ExprAST>> Args)
      : Calee(Calee), Args(std::move(Args)) {}
};
