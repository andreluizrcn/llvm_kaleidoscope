#include <cstdio>
#include <iostream>
#include <string>

/* ============== CHAPTER 1 - LEXER =================*/

// There's no strings in LLVM Kaleidoscope

/* author uses enum w/integers is to use the enum type
to double as the characters if token doesn't fit any
of the described types */

/* The lexer returns tokens [0-255] if it is an unknown character, otherwise one
 of these for known things */

enum Token {
  tok_eof = -1,
  // commands
  tok_def = -2,
  tok_extern = -3,
  // primary
  tok_identifier = -4, // name of functions or variable names
  tok_number = -5,
};

// two globals val (not a great design)
static std::string IdentifierStr;
static double NumVal;

// gettok - return/parse char in str as int (tok)
static int gettok() {
  static int LastChar =
      ' '; // sets as static storage w/limited scope -> whitespace

  // skip spaces
  while (std::isspace(LastChar))
    LastChar = getchar();

  // Get identifier
  // identifier: [a-zA-Z][a-zA-Z0-9]* -> standard
  if (std::isalpha(LastChar)) {
    IdentifierStr = LastChar; // converts int (because is an ASCII code) to str
    while (isalnum((LastChar = getchar()))) {
      IdentifierStr += LastChar;
    }

    if (IdentifierStr == "def") {
      return tok_def;
    }
    if (IdentifierStr == "extern") {
      return tok_extern;
    }
    return tok_identifier;
  }

  // Get number
  if (std::isdigit(LastChar) || LastChar == '.') {
    std::string NumStr;
    do {
      NumStr += LastChar; // append
      LastChar = getchar();
    } while (isdigit(LastChar) || LastChar == '.');

    NumVal = strtod(NumStr.c_str(), 0); // converts str to num
    return tok_number;
  }

  // Process comments
  if (LastChar == '#') {
    // Comment until end of line
    do {
      LastChar = getchar();
    } while (LastChar != EOF && LastChar != '\n' && LastChar != '\r');

    if (LastChar != EOF) {
      return gettok();
    }
  }

  /* Check for end of file.  Don't eat the EOF */
  if (LastChar == EOF) {
    return tok_eof;
  }

  /* Otherwise, just return the character as its ascii value.

   * Store current char (already read ahead in previous call), then read next
   * char for future lookahead. */
  int ThisChar = LastChar;
  LastChar = getchar();
  return ThisChar;
}

/* ============== CHAPTER 2 - PARSER =================*/
