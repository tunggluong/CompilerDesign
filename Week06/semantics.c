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

/**
 * Lookup an object by name with static scoping rule:
 * - search from current scope outward (lexical nesting)
 * - if not found, search global predefined objects list
 */
Object* lookupObject(char *name) {
  Scope* scope = symtab->currentScope;

  while (scope != NULL) {
    Object* obj = findObject(scope->objList, name);
    if (obj != NULL) return obj;
    scope = scope->outer;
  }

  return findObject(symtab->globalObjectList, name);
}

/**
 * A fresh identifier is an identifier that has not been used
 * in the CURRENT scope.
 * Used when declaring: const/type/var/param/function/procedure.
 */
void checkFreshIdent(char *name) {
  if (findObject(symtab->currentScope->objList, name) != NULL) {
    error(ERR_DUPLICATE_IDENT, currentToken->lineNo, currentToken->colNo);
  }
}

/**
 * Check any declared identifier (any kind).
 */
Object* checkDeclaredIdent(char* name) {
  Object* obj = lookupObject(name);
  if (obj == NULL) {
    error(ERR_UNDECLARED_IDENT, currentToken->lineNo, currentToken->colNo);
  }
  return obj;
}

/**
 * Check a declared constant.
 */
Object* checkDeclaredConstant(char* name) {
  Object* obj = lookupObject(name);
  if (obj == NULL) {
    error(ERR_UNDECLARED_CONSTANT, currentToken->lineNo, currentToken->colNo);
    return NULL;
  }
  if (obj->kind != OBJ_CONSTANT) {
    error(ERR_UNDECLARED_CONSTANT, currentToken->lineNo, currentToken->colNo);
    return NULL;
  }
  return obj;
}

/**
 * Check a declared type.
 */
Object* checkDeclaredType(char* name) {
  Object* obj = lookupObject(name);
  if (obj == NULL) {
    error(ERR_UNDECLARED_TYPE, currentToken->lineNo, currentToken->colNo);
    return NULL;
  }
  if (obj->kind != OBJ_TYPE) {
    error(ERR_UNDECLARED_TYPE, currentToken->lineNo, currentToken->colNo);
    return NULL;
  }
  return obj;
}

/**
 * Check a declared variable.
 * In semantic rules, a "variable reference" also covers parameters.
 */
Object* checkDeclaredVariable(char* name) {
  Object* obj = lookupObject(name);
  if (obj == NULL) {
    error(ERR_UNDECLARED_VARIABLE, currentToken->lineNo, currentToken->colNo);
    return NULL;
  }
  if (obj->kind != OBJ_VARIABLE && obj->kind != OBJ_PARAMETER) {
    error(ERR_UNDECLARED_VARIABLE, currentToken->lineNo, currentToken->colNo);
    return NULL;
  }
  return obj;
}

/**
 * Check a declared function.
 */
Object* checkDeclaredFunction(char* name) {
  Object* obj = lookupObject(name);
  if (obj == NULL) {
    error(ERR_UNDECLARED_FUNCTION, currentToken->lineNo, currentToken->colNo);
    return NULL;
  }
  if (obj->kind != OBJ_FUNCTION) {
    error(ERR_UNDECLARED_FUNCTION, currentToken->lineNo, currentToken->colNo);
    return NULL;
  }
  return obj;
}

/**
 * Check a declared procedure.
 */
Object* checkDeclaredProcedure(char* name) {
  Object* obj = lookupObject(name);
  if (obj == NULL) {
    error(ERR_UNDECLARED_PROCEDURE, currentToken->lineNo, currentToken->colNo);
    return NULL;
  }
  if (obj->kind != OBJ_PROCEDURE) {
    error(ERR_UNDECLARED_PROCEDURE, currentToken->lineNo, currentToken->colNo);
    return NULL;
  }
  return obj;
}

/**
 * Check an identifier that appears in an LValue position:
 * - either it is a declared variable/parameter
 * - OR it is the CURRENT function name (assignment to function result)
 */
Object* checkDeclaredLValueIdent(char* name) {
  Object* owner = symtab->currentScope->owner;

  // special case: assignment to current function name is allowed
  if (owner != NULL && owner->kind == OBJ_FUNCTION && strcmp(owner->name, name) == 0) {
    return owner;
  }

  // otherwise must be a variable/parameter
  Object* obj = lookupObject(name);
  if (obj == NULL) {
    error(ERR_UNDECLARED_VARIABLE, currentToken->lineNo, currentToken->colNo);
    return NULL;
  }
  if (obj->kind != OBJ_VARIABLE && obj->kind != OBJ_PARAMETER) {
    error(ERR_UNDECLARED_VARIABLE, currentToken->lineNo, currentToken->colNo);
    return NULL;
  }
  return obj;
}