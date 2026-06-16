#include "lexer.cpp"

int main() {
  while (true) {
    int tok = gettok();
    std::cout << "got token: " << tok << std::endl;
  }

  //  return 0;
}
