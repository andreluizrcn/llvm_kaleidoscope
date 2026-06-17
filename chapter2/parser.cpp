/* ============== CHAPTER 2 - PARSER =================*/

#include <memory>
#include <string>
#include <utility>

class ExprAST {
public:
  virtual ~ExprAST() = default;
};

class NumberExprAST : public ExprAST { // inherit from ExprAST to num
  double Val;

public:
  NumberExprAST(double V) : Val(V) {} // constructor + init
};

class VariableExprAST : public ExprAST {
  std::string Name;

  /* More efficient than just copying the data in memory and pushing it on the
   * stack. Saves memory and processing time. */
public:
  VariableExprAST(const std::string &Name) : Name(Name) {}
};

class BinaryExprAST : public ExprAST {
  char Op; // + - * / > <
  std::unique_ptr<ExprAST> LHS, RHS;

public:
  BinaryExprAST(char Op, std::unique_ptr<ExprAST> LHS,
                std::unique_ptr<ExprAST> RHS)
      : Op(Op), LHS(std::move(LHS)), RHS(std::move(RHS)) {}
};
