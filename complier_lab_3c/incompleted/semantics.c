/* 
 * @copyright (c) 2008, Hedspi, Hanoi University of Technology
 * @author Huu-Duc Nguyen
 * @version 1.0
 */

#include <stdlib.h>
#include <string.h>
#include "semantics.h"
#include "error.h"

extern SymTab* symtab;
extern Token* currentToken;

// Tìm kiếm object theo tên trong tất cả các scope
Object* lookupObject(char *name) {
  Scope* scope = symtab->currentScope;
  Object* obj;
  
  while (scope != NULL) {
    obj = findObject(scope->objList, name);
    if (obj != NULL) return obj;
    scope = scope->outer;
  }
  
  obj = findObject(symtab->globalObjectList, name);
  return obj;
}

// Kiểm tra xem identifier có phải là tên mới trong block hiện tại không
void checkFreshIdent(char *name) {
  Object* obj = findObject(symtab->currentScope->objList, name);
  if (obj != NULL) 
    error(ERR_DUPLICATE_IDENT, currentToken->lineNo, currentToken->colNo);
}

// Kiểm tra identifier đã được khai báo chưa
Object* checkDeclaredIdent(char* name) {
  Object* obj = lookupObject(name);
  if (obj == NULL) 
    error(ERR_UNDECLARED_IDENT, currentToken->lineNo, currentToken->colNo);
  return obj;
}

// Kiểm tra constant đã được khai báo chưa
Object* checkDeclaredConstant(char* name) {
  Object* obj = lookupObject(name);
  if (obj == NULL) 
    error(ERR_UNDECLARED_CONSTANT, currentToken->lineNo, currentToken->colNo);
  if (obj->kind != OBJ_CONSTANT)
    error(ERR_INVALID_CONSTANT, currentToken->lineNo, currentToken->colNo);
  return obj;
}

// Kiểm tra type đã được khai báo chưa
Object* checkDeclaredType(char* name) {
  Object* obj = lookupObject(name);
  if (obj == NULL) 
    error(ERR_UNDECLARED_TYPE, currentToken->lineNo, currentToken->colNo);
  if (obj->kind != OBJ_TYPE)
    error(ERR_INVALID_TYPE, currentToken->lineNo, currentToken->colNo);
  return obj;
}

// Kiểm tra variable đã được khai báo chưa
Object* checkDeclaredVariable(char* name) {
  Object* obj = lookupObject(name);
  if (obj == NULL) 
    error(ERR_UNDECLARED_VARIABLE, currentToken->lineNo, currentToken->colNo);
  if (obj->kind != OBJ_VARIABLE)
    error(ERR_INVALID_VARIABLE, currentToken->lineNo, currentToken->colNo);
  return obj;
}

// Kiểm tra function đã được khai báo chưa
Object* checkDeclaredFunction(char* name) {
  Object* obj = lookupObject(name);
  if (obj == NULL) 
    error(ERR_UNDECLARED_FUNCTION, currentToken->lineNo, currentToken->colNo);
  if (obj->kind != OBJ_FUNCTION)
    error(ERR_INVALID_FUNCTION, currentToken->lineNo, currentToken->colNo);
  return obj;
}

// Kiểm tra procedure đã được khai báo chưa
Object* checkDeclaredProcedure(char* name) {
  Object* obj = lookupObject(name);
  if (obj == NULL) 
    error(ERR_UNDECLARED_PROCEDURE, currentToken->lineNo, currentToken->colNo);
  if (obj->kind != OBJ_PROCEDURE)
    error(ERR_INVALID_PROCEDURE, currentToken->lineNo, currentToken->colNo);
  return obj;
}

// Kiểm tra LValue (variable, parameter, function) đã được khai báo chưa
Object* checkDeclaredLValueIdent(char* name) {
  Object* obj = lookupObject(name);
  if (obj == NULL) 
    error(ERR_UNDECLARED_IDENT, currentToken->lineNo, currentToken->colNo);
  
  switch (obj->kind) {
    case OBJ_VARIABLE:
    case OBJ_PARAMETER:
    case OBJ_FUNCTION:
      return obj;
    default:
      error(ERR_INVALID_IDENT, currentToken->lineNo, currentToken->colNo);
  }
  return NULL;
}

