/* Scanner
 * @copyright (c) 2008, Hedspi, Hanoi University of Technology
 * @author Huu-Duc Nguyen
 * @version 1.0
 */

#include <stdio.h>
#include <stdlib.h>

#include "reader.h"
#include "charcode.h"
#include "token.h"
#include "error.h"


extern int lineNo;
extern int colNo;
extern int currentChar;

extern CharCode charCodes[];

/***************************************************************/

void skipBlank() {
  // Skip tất cả các ký tự khoảng trắng liên tiếp
  while(currentChar != EOF && charCodes[currentChar] == CHAR_SPACE) {
    readChar();
  }
}

void skipComment() {
  // Skip comment
  // Dùng vòng lặp để tìm cặp *) kết thúc comment
  int state = 0; // 0: đang tìm *, 1: đã thấy * đang chờ )
  
  readChar(); // Skip ký tự * sau (
  
  while(currentChar != EOF) {
    if(state == 0) {
      // Đang tìm dấu *
      if(charCodes[currentChar] == CHAR_TIMES) {
        state = 1; // Đã thấy *, chờ )
      }
      readChar();
    } else { // state == 1
      // Đã thấy *, kiểm tra ký tự tiếp theo
      if(charCodes[currentChar] == CHAR_RPAR) {
        readChar(); // Skip ) và kết thúc
        return;
      } else if(charCodes[currentChar] == CHAR_TIMES) {
        // Trường hợp ** vẫn giữ state = 1
        readChar();
      } else {
        // Không phải *) thì quay lại tìm *
        state = 0;
        readChar();
      }
    }
  }
  
  // Nếu đến EOF mà chưa gặp *) thì báo lỗi
  if(currentChar == EOF) {
    error(ERR_ENDOFCOMMENT, lineNo, colNo);
#ifndef UNIT_TEST_MODE
    exit(-1);
#endif
  }
}

void skipLineComment() {
  // Skip comment dạng // cho đến cuối dòng
  while(currentChar != EOF && currentChar != '\n') {
    readChar();
  }
  // Sau khi skip, currentChar là \n hoặc EOF
  // readChar() sẽ được gọi trong getToken() để xử lý \n
  if(currentChar == '\n') {
    readChar(); // Skip ký tự xuống dòng
  }
}

Token* readIdentKeyword(void) {
  // Đọc identifier hoặc keyword
  // Bắt đầu bằng chữ cái, theo sau là chữ cái hoặc chữ số
  Token *token = makeToken(TK_NONE, lineNo, colNo);
  int counter = 0;

  // Đọc các ký tự cho đến khi gặp ký tự không phải chữ cái hoặc chữ số
  while((charCodes[currentChar] == CHAR_LETTER || charCodes[currentChar] == CHAR_DIGIT) && currentChar != EOF) {
    // Kiểm tra độ dài có vượt quá MAX_IDENT_LEN không
    if(counter >= MAX_IDENT_LEN) {
      // Báo lỗi nhưng vẫn tiếp tục đọc để skip hết identifier
      error(ERR_IDENTTOOLONG, lineNo, colNo);
      // Chỉ báo lỗi 1 lần
      while((charCodes[currentChar] == CHAR_LETTER || charCodes[currentChar] == CHAR_DIGIT) && currentChar != EOF) {
        readChar();
      }
      break;
    }
    
    token->string[counter++] = (char)currentChar;
    readChar();
  }

  // Kết thúc chuỗi
  token->string[counter] = '\0';
  
  // Kiểm tra xem có phải keyword không
  token->tokenType = checkKeyword(token->string);
  if(token->tokenType == TK_NONE) {
    // Không phải keyword thì là identifier
    token->tokenType = TK_IDENT;
  }

  return token;
}

Token* readNumber(void) {
  // Đọc số nguyên (dãy các chữ số)
  Token *token = makeToken(TK_NUMBER, lineNo, colNo);
  int counter = 0;

  // Đọc tất cả các chữ số liên tiếp
  while(charCodes[currentChar] == CHAR_DIGIT && currentChar != EOF) {
    // Giới hạn độ dài để tránh buffer overflow
    if(counter < MAX_IDENT_LEN) {
      token->string[counter++] = (char)currentChar;
    }
    readChar();
  }
  
  // Kết thúc chuỗi
  token->string[counter] = '\0';
  // Chuyển chuỗi thành số nguyên
  token->value = atoi(token->string);

  return token;
}

Token* readConstChar(void) {
  // Đọc hằng ký tự dạng 'c'
  // Format: dấu nháy đơn, một ký tự, dấu nháy đơn
  Token *token = makeToken(TK_CHAR, lineNo, colNo);
  int ln = lineNo;
  int cn = colNo;
  
  readChar(); // Skip dấu nháy đơn mở '
  
  // Kiểm tra ký tự bên trong
  if (currentChar == EOF || charCodes[currentChar] == CHAR_SINGLEQUOTE) {
    // Lỗi: rỗng '' hoặc chưa có ký tự đã EOF
    error(ERR_INVALIDCHARCONSTANT, ln, cn);
    return token;
  }
  
  // Lưu ký tự
  token->string[0] = (char)currentChar;
  token->string[1] = '\0';
  
  readChar(); // Đọc ký tự tiếp theo (phải là dấu nháy đơn đóng)
  
  // Kiểm tra dấu nháy đơn đóng
  if (currentChar == EOF || charCodes[currentChar] != CHAR_SINGLEQUOTE) {
    // Lỗi: thiếu dấu nháy đơn đóng hoặc có nhiều hơn 1 ký tự
    error(ERR_INVALIDCHARCONSTANT, ln, cn);
    return token;
  }
  
  readChar(); // Skip dấu nháy đơn đóng
  
  return token;
}

// Đọc một token tính từ vị trí hiện tại
Token* getToken(void) {
  Token *token;
  int ln, cn;

  if (currentChar == EOF) 
    return makeToken(TK_EOF, lineNo, colNo);

  switch (charCodes[currentChar]) {
  case CHAR_SPACE: skipBlank(); return getToken();
  case CHAR_LETTER: return readIdentKeyword();
  case CHAR_DIGIT: return readNumber();
  case CHAR_PLUS: 
    token = makeToken(SB_PLUS, lineNo, colNo);
    readChar(); 
    return token;
    
  case CHAR_MINUS:
    token = makeToken(SB_MINUS, lineNo, colNo);
    readChar();
    return token;
    
  case CHAR_TIMES:
    token = makeToken(SB_TIMES, lineNo, colNo);
    readChar();
    return token;
    
  case CHAR_SLASH:
    // Xử lý dấu / : có thể là phép chia hoặc comment dòng //
    ln = lineNo;
    cn = colNo;
    readChar();
    if(charCodes[currentChar] == CHAR_SLASH) {
      // Gặp // thì skip comment đến cuối dòng
      skipLineComment();
      return getToken(); // Đọc token tiếp theo sau comment
    }
    // Không phải comment thì là phép chia /
    return makeToken(SB_SLASH, ln, cn);
    
  case CHAR_LPAR:
    // Xử lý dấu ( : có thể là (, (., hoặc (* comment *)
    ln = lineNo;
    cn = colNo;
    readChar();
    
    if(charCodes[currentChar] == CHAR_TIMES) {
      // Gặp (* thì đây là comment
      skipComment();
      return getToken(); // Đọc token tiếp theo sau comment
    }
    
    if(charCodes[currentChar] == CHAR_PERIOD) {
      // Gặp (. thì đây là LSEL (left selector)
      readChar();
      return makeToken(SB_LSEL, ln, cn);
    }
    
    // Chỉ là dấu ( thông thường
    return makeToken(SB_LPAR, ln, cn);
    
  case CHAR_RPAR:
    // Dấu đóng ngoặc )
    token = makeToken(SB_RPAR, lineNo, colNo);
    readChar();
    return token;
    
  case CHAR_COMMA:
    // Dấu ,
    token = makeToken(SB_COMMA, lineNo, colNo);
    readChar();
    return token;
    
  case CHAR_SEMICOLON:
    // Dấu ;
    token = makeToken(SB_SEMICOLON, lineNo, colNo);
    readChar();
    return token;   
  case CHAR_PERIOD:
    // Xử lý dấu . : có thể là . hoặc .) (right selector)
    ln = lineNo;
    cn = colNo;
    readChar();
    if(charCodes[currentChar] == CHAR_RPAR && currentChar != EOF) {
      // Gặp .) thì đây là RSEL (right selector)
      readChar();
      return makeToken(SB_RSEL, ln, cn);
    }
    // Chỉ là dấu . thông thường
    return makeToken(SB_PERIOD, ln, cn);
    
  case CHAR_COLON:
    // Xử lý dấu : : có thể là : hoặc :=
    token = makeToken(SB_COLON, lineNo, colNo);
    readChar();
    if(charCodes[currentChar] == CHAR_EQ) {
      // Gặp := thì đây là phép gán
      token->tokenType = SB_ASSIGN;
      readChar();
    }
    // Chỉ là dấu : thông thường (dùng để khai báo kiểu)
    return token;
    
  case CHAR_SINGLEQUOTE:
    // Hằng ký tự 'c'
    return readConstChar();
    
  case CHAR_EQ:
    // Dấu bằng = (so sánh)
    token = makeToken(SB_EQ, lineNo, colNo);
    readChar();
    return token;
    
  case CHAR_EXCLAIMATION:
    // Dấu ! : chỉ hợp lệ khi theo sau là = tạo thành !=
    ln = lineNo;
    cn = colNo;
    readChar();
    if(charCodes[currentChar] != CHAR_EQ) {
      // Lỗi: dấu ! đứng một mình không hợp lệ
      error(ERR_INVALIDSYMBOL, ln, cn);
      return makeToken(TK_NONE, ln, cn);
    }
    // Gặp != thì đây là phép so sánh khác
    readChar();
    return makeToken(SB_NEQ, ln, cn);
    
  case CHAR_GT:
    // Xử lý dấu > : có thể là > hoặc >=
    token = makeToken(SB_GT, lineNo, colNo);
    readChar();
    if(charCodes[currentChar] == CHAR_EQ) {
      // Gặp >= thì đây là lớn hơn hoặc bằng
      token->tokenType = SB_GE;
      readChar();
    }
    // Chỉ là dấu > thông thường (lớn hơn)
    return token;
    
  case CHAR_LT:
    // Xử lý dấu < : có thể là <, <=, hoặc <>
    token = makeToken(SB_LT, lineNo, colNo);
    readChar();
    if(charCodes[currentChar] == CHAR_EQ) {
      // Gặp <= thì đây là nhỏ hơn hoặc bằng
      token->tokenType = SB_LE;
      readChar();
    } 
    else if(charCodes[currentChar] == CHAR_GT) {
      // Gặp <> thì đây là khác (cách viết khác của !=)
      token->tokenType = SB_NEQ;
      readChar();
    }
    // Chỉ là dấu < thông thường (nhỏ hơn)
    return token;
    
  default:
    // Ký tự không hợp lệ/không nhận dạng được
    token = makeToken(TK_NONE, lineNo, colNo);
    error(ERR_INVALIDSYMBOL, lineNo, colNo);
    readChar(); // Skip ký tự lỗi và tiếp tục
    return token;
  }
}


/******************************************************************/

void printToken(Token *token) {

  printf("%d-%d:", token->lineNo, token->colNo);

  switch (token->tokenType) {
  case TK_NONE: printf("TK_NONE\n"); break;
  case TK_IDENT: printf("TK_IDENT(%s)\n", token->string); break;
  case TK_NUMBER: printf("TK_NUMBER(%s)\n", token->string); break;
  case TK_CHAR: printf("TK_CHAR(\'%s\')\n", token->string); break;
  case TK_EOF: printf("TK_EOF\n"); break;

  case KW_PROGRAM: printf("KW_PROGRAM\n"); break;
  case KW_CONST: printf("KW_CONST\n"); break;
  case KW_TYPE: printf("KW_TYPE\n"); break;
  case KW_VAR: printf("KW_VAR\n"); break;
  case KW_INTEGER: printf("KW_INTEGER\n"); break;
  case KW_CHAR: printf("KW_CHAR\n"); break;
  case KW_ARRAY: printf("KW_ARRAY\n"); break;
  case KW_OF: printf("KW_OF\n"); break;
  case KW_FUNCTION: printf("KW_FUNCTION\n"); break;
  case KW_PROCEDURE: printf("KW_PROCEDURE\n"); break;
  case KW_BEGIN: printf("KW_BEGIN\n"); break;
  case KW_END: printf("KW_END\n"); break;
  case KW_CALL: printf("KW_CALL\n"); break;
  case KW_IF: printf("KW_IF\n"); break;
  case KW_THEN: printf("KW_THEN\n"); break;
  case KW_ELSE: printf("KW_ELSE\n"); break;
  case KW_WHILE: printf("KW_WHILE\n"); break;
  case KW_DO: printf("KW_DO\n"); break;
  case KW_FOR: printf("KW_FOR\n"); break;
  case KW_TO: printf("KW_TO\n"); break;

  case SB_SEMICOLON: printf("SB_SEMICOLON\n"); break;
  case SB_COLON: printf("SB_COLON\n"); break;
  case SB_PERIOD: printf("SB_PERIOD\n"); break;
  case SB_COMMA: printf("SB_COMMA\n"); break;
  case SB_ASSIGN: printf("SB_ASSIGN\n"); break;
  case SB_EQ: printf("SB_EQ\n"); break;
  case SB_NEQ: printf("SB_NEQ\n"); break;
  case SB_LT: printf("SB_LT\n"); break;
  case SB_LE: printf("SB_LE\n"); break;
  case SB_GT: printf("SB_GT\n"); break;
  case SB_GE: printf("SB_GE\n"); break;
  case SB_PLUS: printf("SB_PLUS\n"); break;
  case SB_MINUS: printf("SB_MINUS\n"); break;
  case SB_TIMES: printf("SB_TIMES\n"); break;
  case SB_SLASH: printf("SB_SLASH\n"); break;
  case SB_LPAR: printf("SB_LPAR\n"); break;
  case SB_RPAR: printf("SB_RPAR\n"); break;
  case SB_LSEL: printf("SB_LSEL\n"); break;
  case SB_RSEL: printf("SB_RSEL\n"); break;
  }
}

int scan(char *fileName) {
  Token *token;

  if (openInputStream(fileName) == IO_ERROR)
    return IO_ERROR;

  token = getToken();
  while (token->tokenType != TK_EOF) {
    printToken(token);
    free(token);
    token = getToken();
  }

  free(token);
  closeInputStream();
  return IO_SUCCESS;
}

/******************************************************************/

#ifndef UNIT_TEST_MODE
int main(int argc, char *argv[]) {
  if (argc <= 1) {
    printf("scanner: no input file.\n");
    return -1;
  }

  if (scan(argv[1]) == IO_ERROR) {
    printf("Can\'t read input file!\n");
    return -1;
  }
    
  return 0;
}
#endif



