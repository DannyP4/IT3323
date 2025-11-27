/* 
 * @copyright (c) 2008, Hedspi, Hanoi University of Technology
 * @author Huu-Duc Nguyen
 * @version 1.0
 */

#include <stdlib.h>

#include "reader.h"
#include "scanner.h"
#include "parser.h"
#include "error.h"

Token *currentToken;
Token *lookAhead;

// Đọc token tiếp theo
void scan(void) {
  Token* tmp = currentToken;
  currentToken = lookAhead;
  lookAhead = getValidToken();
  free(tmp);
}

// Kiểm tra và đọc token kỳ vọng
void eat(TokenType tokenType) {
  if (lookAhead->tokenType == tokenType) {
    printToken(lookAhead);
    scan();
  } else missingToken(tokenType, lookAhead->lineNo, lookAhead->colNo);
}

// Phân tích chương trình: PROGRAM IDENT ; Block .
void compileProgram(void) {
  assert("Parsing a Program ....");
  eat(KW_PROGRAM);
  eat(TK_IDENT);
  eat(SB_SEMICOLON);
  compileBlock();
  eat(SB_PERIOD);
  assert("Program parsed!");
}

// Phân tích khối CONST, TYPE, VAR, SubDecls, BEGIN, END
void compileBlock(void) {
  assert("Parsing a Block ....");
  if (lookAhead->tokenType == KW_CONST) {
    eat(KW_CONST);
    compileConstDecl();
    compileConstDecls();
    compileBlock2();
  } 
  else compileBlock2();
  assert("Block parsed!");
}

// Phân tích TYPE
void compileBlock2(void) {
  if (lookAhead->tokenType == KW_TYPE) {
    eat(KW_TYPE);
    compileTypeDecl();
    compileTypeDecls();
    compileBlock3();
  } 
  else compileBlock3();
}

// Phân tích VAR
void compileBlock3(void) {
  if (lookAhead->tokenType == KW_VAR) {
    eat(KW_VAR);
    compileVarDecl();
    compileVarDecls();
    compileBlock4();
  } 
  else compileBlock4();
}

// Phân tích phần khai báo hàm, thủ tục
void compileBlock4(void) {
  compileSubDecls();
  compileBlock5();
}

// Phân tích thân
void compileBlock5(void) {
  eat(KW_BEGIN);
  compileStatements();
  eat(KW_END);
}

// Phân tích danh sách khai báo hằng
void compileConstDecls(void) {
  while (lookAhead->tokenType == TK_IDENT) {
    compileConstDecl();
  }
}

// Phân tích một khai báo hằng IDENT, =, Constant, ;
void compileConstDecl(void) {
  // Kiểm tra token đầu phải là IDENT
  if (lookAhead->tokenType != TK_IDENT) {
    error(ERR_INVALIDCONSTDECL, lookAhead->lineNo, lookAhead->colNo);
  }
  eat(TK_IDENT);
  eat(SB_EQ);
  compileConstant();
  eat(SB_SEMICOLON);
}

// Phân tích danh sách khai báo kiểu
void compileTypeDecls(void) {
  while (lookAhead->tokenType == TK_IDENT) {
    compileTypeDecl();
  }
}

// Phân tích một khai báo kiểu: IDENT, =, Type, ;
void compileTypeDecl(void) {
  // Kiểm tra token đầu phải là IDENT
  if (lookAhead->tokenType != TK_IDENT) {
    error(ERR_INVALIDTYPEDECL, lookAhead->lineNo, lookAhead->colNo);
  }
  eat(TK_IDENT);
  eat(SB_EQ);
  compileType();
  eat(SB_SEMICOLON);
}

// Phân tích danh sách khai báo biến
void compileVarDecls(void) {
  while (lookAhead->tokenType == TK_IDENT) {
    compileVarDecl();
  }
}

// Phân tích một khai báo biến: IDENT, :, Type, ;
void compileVarDecl(void) {
  // Kiểm tra token đầu phải là IDENT
  if (lookAhead->tokenType != TK_IDENT) {
    error(ERR_INVALIDVARDECL, lookAhead->lineNo, lookAhead->colNo);
  }
  eat(TK_IDENT);
  eat(SB_COLON);
  compileType();
  eat(SB_SEMICOLON);
}

// Phân tích danh sách khai báo hàm, thủ tục
void compileSubDecls(void) {
  assert("Parsing subtoutines ....");
  if ((lookAhead->tokenType == KW_FUNCTION) || 
      (lookAhead->tokenType == KW_PROCEDURE)) {
    if (lookAhead->tokenType == KW_FUNCTION)
      compileFuncDecl();
    else
      compileProcDecl();
    compileSubDecls();
  }
  // Kiểm tra nếu gặp token không hợp lệ cho vị trí này
  else if (lookAhead->tokenType != KW_BEGIN && 
           lookAhead->tokenType != TK_EOF) {
    // Nếu không phải BEGIN hoặc EOF mà cũng không phải FUNCTION/PROCEDURE
    // thì có thể là lỗi khai báo subroutine
    if (lookAhead->tokenType == TK_IDENT) {
      error(ERR_INVALIDSUBDECL, lookAhead->lineNo, lookAhead->colNo);
    }
  }
  assert("Subtoutines parsed ....");
}

// Phân tích khai báo hàm FUNCTION, IDENT, Params, :, BasicType, ;, Block ;
void compileFuncDecl(void) {
  assert("Parsing a function ....");
  eat(KW_FUNCTION);
  eat(TK_IDENT);
  compileParams();
  eat(SB_COLON);
  compileBasicType();
  eat(SB_SEMICOLON);
  compileBlock();
  eat(SB_SEMICOLON);
  assert("Function parsed ....");
}

// Phân tích khai báo thủ tục PROCEDURE, IDENT, Params, ;, Block, ;
void compileProcDecl(void) {
  assert("Parsing a procedure ....");
  eat(KW_PROCEDURE);
  eat(TK_IDENT);
  compileParams();
  eat(SB_SEMICOLON);
  compileBlock();
  eat(SB_SEMICOLON);
  assert("Procedure parsed ....");
}

// Phân tích hằng không dấu NUMBER, |, CHAR
void compileUnsignedConstant(void) {
  switch (lookAhead->tokenType) {
  case TK_NUMBER:
    eat(TK_NUMBER);
    break;
  case TK_CHAR:
    eat(TK_CHAR);
    break;
  default:
    error(ERR_INVALIDCONSTANT, lookAhead->lineNo, lookAhead->colNo);
    break;
  }
}

// Phân tích hằng số [+|-], (NUMBER | IDENT) | CHAR
void compileConstant(void) {
  switch (lookAhead->tokenType) {
  case SB_PLUS:
    eat(SB_PLUS);
    compileConstant2();
    break;
  case SB_MINUS:
    eat(SB_MINUS);
    compileConstant2();
    break;
  case TK_CHAR:
    eat(TK_CHAR);
    break;
  default:
    compileConstant2();
    break;
  }
}

// Phân tích IDENT hoặc NUMBER
void compileConstant2(void) {
  switch (lookAhead->tokenType) {
  case TK_IDENT:
    eat(TK_IDENT);
    break;
  case TK_NUMBER:
    eat(TK_NUMBER);
    break;
  default:
    error(ERR_INVALIDCONSTANT, lookAhead->lineNo, lookAhead->colNo);
    break;
  }
}

// Phân tích kiểu INTEGER | CHAR | ARRAY | IDENT
void compileType(void) {
  switch (lookAhead->tokenType) {
  case KW_INTEGER:
  case KW_CHAR:
    compileBasicType();
    break;
  case KW_ARRAY:
    eat(KW_ARRAY);
    eat(SB_LSEL);
    eat(TK_NUMBER);
    eat(SB_RSEL);
    eat(KW_OF);
    compileType();
    break;
  case TK_IDENT:
    eat(TK_IDENT);
    break;
  default:
    error(ERR_INVALIDTYPE, lookAhead->lineNo, lookAhead->colNo);
    break;
  }
}

// Phân tích kiểu cơ bản INTEGER | CHAR
void compileBasicType(void) {
  switch (lookAhead->tokenType) {
  case KW_INTEGER:
    eat(KW_INTEGER);
    break;
  case KW_CHAR:
    eat(KW_CHAR);
    break;
  default:
    error(ERR_INVALIDBASICTYPE, lookAhead->lineNo, lookAhead->colNo);
    break;
  }
}

// Phân tích danh sách tham số (Param; Param; ...)
void compileParams(void) {
  if (lookAhead->tokenType == SB_LPAR) {
    eat(SB_LPAR);
    compileParam();
    compileParams2();
    eat(SB_RPAR);
  }
}

// Phân tích các tham số tiếp theo
void compileParams2(void) {
  while (lookAhead->tokenType == SB_SEMICOLON) {
    eat(SB_SEMICOLON);
    compileParam();
  }
}

// Phân tích một tham số IDENT, :, BasicType
void compileParam(void) {
  // Kiểm tra token đầu phải là IDENT
  if (lookAhead->tokenType != TK_IDENT) {
    error(ERR_INVALIDPARAM, lookAhead->lineNo, lookAhead->colNo);
  }
  eat(TK_IDENT);
  eat(SB_COLON);
  compileBasicType();
}

// Phân tích danh sách câu lệnh
void compileStatements(void) {
  compileStatement();
  compileStatements2();
}

// Phân tích các câu lệnh tiếp theo ngăn cách bởi ;
void compileStatements2(void) {
  while (lookAhead->tokenType == SB_SEMICOLON) {
    eat(SB_SEMICOLON);
    compileStatement();
  }
  // Kiểm tra nếu gặp đầu statement mới mà không có ; thì báo lỗi
  if (lookAhead->tokenType == TK_IDENT || 
      lookAhead->tokenType == KW_CALL ||
      lookAhead->tokenType == KW_BEGIN ||
      lookAhead->tokenType == KW_IF ||
      lookAhead->tokenType == KW_WHILE ||
      lookAhead->tokenType == KW_FOR) {
    missingToken(SB_SEMICOLON, lookAhead->lineNo, lookAhead->colNo);
  }
}

// Phân tích câu lệnh Assign | Call | Group | If | While | For
void compileStatement(void) {
  switch (lookAhead->tokenType) {
  case TK_IDENT:
    compileAssignSt();
    break;
  case KW_CALL:
    compileCallSt();
    break;
  case KW_BEGIN:
    compileGroupSt();
    break;
  case KW_IF:
    compileIfSt();
    break;
  case KW_WHILE:
    compileWhileSt();
    break;
  case KW_FOR:
    compileForSt();
    break;
  case SB_SEMICOLON:
  case KW_END:
  case KW_ELSE:
    break;
  default:
    error(ERR_INVALIDSTATEMENT, lookAhead->lineNo, lookAhead->colNo);
    break;
  }
}

// Phân tích câu lệnh gán IDENT(.Indexes.) := Expression
void compileAssignSt(void) {
  assert("Parsing an assign statement ....");
  eat(TK_IDENT);
  compileIndexes();
  eat(SB_ASSIGN);
  compileExpression();
  assert("Assign statement parsed ....");
}

// Phân tích câu lệnh gọi CALL IDENT(Arguments)
void compileCallSt(void) {
  assert("Parsing a call statement ....");
  eat(KW_CALL);
  eat(TK_IDENT);
  compileArguments();
  assert("Call statement parsed ....");
}

// Phân tích nhóm câu lệnh BEGIN, Statements, END
void compileGroupSt(void) {
  assert("Parsing a group statement ....");
  eat(KW_BEGIN);
  compileStatements();
  eat(KW_END);
  assert("Group statement parsed ....");
}

// Ph\u00e2n t\u00edch c\u00e2u l\u1ec7nh IF: IF Condition THEN Statement [ELSE Statement]
void compileIfSt(void) {
  assert("Parsing an if statement ....");
  eat(KW_IF);
  compileCondition();
  eat(KW_THEN);
  compileStatement();
  if (lookAhead->tokenType == KW_ELSE) 
    compileElseSt();
  assert("If statement parsed ....");
}

// Phân tích nhánh ELSE
void compileElseSt(void) {
  eat(KW_ELSE);
  compileStatement();
}

// Phân tích vòng lặp WHILE: WHILE Condition DO Statement
void compileWhileSt(void) {
  assert("Parsing a while statement ....");
  eat(KW_WHILE);
  compileCondition();
  eat(KW_DO);
  compileStatement();
  assert("While statement parsed ....");
}

// Phân tích vòng lặp FOR: FOR IDENT := Exp TO Exp DO Statement
void compileForSt(void) {
  assert("Parsing a for statement ....");
  eat(KW_FOR);
  eat(TK_IDENT);
  eat(SB_ASSIGN);
  compileExpression();
  eat(KW_TO);
  compileExpression();
  eat(KW_DO);
  compileStatement();
  assert("For statement parsed ....");
}

// Phân tích danh sách đối số: Exp, Exp, ...
void compileArguments(void) {
  if (lookAhead->tokenType == SB_LPAR) {
    eat(SB_LPAR);
    compileExpression();
    compileArguments2();
    eat(SB_RPAR);
  }
}

// Phân tích các đối số tiếp theo ngăn cách bởi dấu ,
void compileArguments2(void) {
  while (lookAhead->tokenType == SB_COMMA) {
    eat(SB_COMMA);
    compileExpression();
  }
}

// Phân tích điều kiện Expression (=|!=|<|<=|>|>=) Expression
void compileCondition(void) {
  compileExpression();
  compileCondition2();
}

// Phân tích toán tử so sánh
void compileCondition2(void) {
  switch (lookAhead->tokenType) {
  case SB_EQ:
    eat(SB_EQ);
    compileExpression();
    break;
  case SB_NEQ:
    eat(SB_NEQ);
    compileExpression();
    break;
  case SB_LT:
    eat(SB_LT);
    compileExpression();
    break;
  case SB_LE:
    eat(SB_LE);
    compileExpression();
    break;
  case SB_GT:
    eat(SB_GT);
    compileExpression();
    break;
  case SB_GE:
    eat(SB_GE);
    compileExpression();
    break;
  default:
    error(ERR_INVALIDCOMPARATOR, lookAhead->lineNo, lookAhead->colNo);
    break;
  }
}

// Phân tích biểu thức [+|-] Term {(+|-) Term}
void compileExpression(void) {
  assert("Parsing an expression");
  switch (lookAhead->tokenType) {
  case SB_PLUS:
    eat(SB_PLUS);
    compileTerm();
    compileExpression2();
    break;
  case SB_MINUS:
    eat(SB_MINUS);
    compileTerm();
    compileExpression2();
    break;
  default:
    compileTerm();
    compileExpression2();
    break;
  }
  assert("Expression parsed");
}

// Phân tích các phép +|- tiếp theo
void compileExpression2(void) {
  while ((lookAhead->tokenType == SB_PLUS) || 
         (lookAhead->tokenType == SB_MINUS)) {
    if (lookAhead->tokenType == SB_PLUS) {
      eat(SB_PLUS);
      compileTerm();
    } else {
      eat(SB_MINUS);
      compileTerm();
    }
  }
}

// Phân tích biểu thức (đệ quy)
void compileExpression3(void) {
  switch (lookAhead->tokenType) {
  case SB_PLUS:
    eat(SB_PLUS);
    compileTerm();
    compileExpression3();
    break;
  case SB_MINUS:
    eat(SB_MINUS);
    compileTerm();
    compileExpression3();
    break;
  default:
    break;
  }
}

// Phân tích term Factor {(*|/) Factor}
void compileTerm(void) {
  compileFactor();
  compileTerm2();
}

// Phân tích các phép *|/ tiếp theo
void compileTerm2(void) {
  while ((lookAhead->tokenType == SB_TIMES) || 
         (lookAhead->tokenType == SB_SLASH)) {
    if (lookAhead->tokenType == SB_TIMES) {
      eat(SB_TIMES);
      compileFactor();
    } else {
      eat(SB_SLASH);
      compileFactor();
    }
  }
}

// Phân tích facto: NUMBER | CHAR | IDENT | IDENT(.Indexes.) | IDENT(Args) | (Expression)
void compileFactor(void) {
  switch (lookAhead->tokenType) {
  case TK_NUMBER:
  case TK_CHAR:
    compileUnsignedConstant();
    break;
  case TK_IDENT:
    eat(TK_IDENT);
    switch (lookAhead->tokenType) {
    case SB_LSEL:
      compileIndexes();
      break;
    case SB_LPAR:
      compileArguments();
      break;
    default:
      break;
    }
    break;
  case SB_LPAR:
    eat(SB_LPAR);
    compileExpression();
    eat(SB_RPAR);
    break;
  default:
    error(ERR_INVALIDFACTOR, lookAhead->lineNo, lookAhead->colNo);
    break;
  }
}

// Phân tích chỉ số mảng {(. Expression .)}
void compileIndexes(void) {
  while (lookAhead->tokenType == SB_LSEL) {
    eat(SB_LSEL);
    compileExpression();
    eat(SB_RSEL);
  }
}

// Khởi động parser
int compile(char *fileName) {
  if (openInputStream(fileName) == IO_ERROR)
    return IO_ERROR;

  currentToken = NULL;
  lookAhead = getValidToken();

  compileProgram();

  free(currentToken);
  free(lookAhead);
  closeInputStream();
  return IO_SUCCESS;

}
