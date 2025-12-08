/* 
 * @copyright (c) 2008, Hedspi, Hanoi University of Technology
 * @author Huu-Duc Nguyen
 * @version 1.0
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "symtab.h"

void freeObject(Object* obj);
void freeScope(Scope* scope);
void freeObjectList(ObjectNode *objList);
void freeReferenceList(ObjectNode *objList);

SymTab* symtab;
Type* intType;
Type* charType;

/******************* Type utilities ******************************/

Type* makeIntType(void) {
  Type* type = (Type*) malloc(sizeof(Type));
  type->typeClass = TP_INT;
  return type;
}

Type* makeCharType(void) {
  Type* type = (Type*) malloc(sizeof(Type));
  type->typeClass = TP_CHAR;
  return type;
}

Type* makeArrayType(int arraySize, Type* elementType) {
  Type* type = (Type*) malloc(sizeof(Type));
  type->typeClass = TP_ARRAY;
  type->arraySize = arraySize;
  type->elementType = elementType;
  return type;
}

Type* duplicateType(Type* type) {
  if (type == NULL) return NULL;
  switch (type->typeClass) {
  case TP_INT:
    return makeIntType();
  case TP_CHAR:
    return makeCharType();
  case TP_ARRAY:
    return makeArrayType(type->arraySize, duplicateType(type->elementType));
  default:
    return NULL;
  }
}

int compareType(Type* type1, Type* type2) {
  if (type1 == type2) return 1;
  if (type1 == NULL || type2 == NULL) return 0;
  if (type1->typeClass != type2->typeClass) return 0;

  switch (type1->typeClass) {
  case TP_INT:
  case TP_CHAR:
    return 1;
  case TP_ARRAY:
    if (type1->arraySize != type2->arraySize) return 0;
    return compareType(type1->elementType, type2->elementType);
  default:
    return 0;
  }
}

void freeType(Type* type) {
  if (type == NULL) return;
  if (type->typeClass == TP_ARRAY) {
    if (type->elementType != NULL)
      freeType(type->elementType);
  }
  free(type);
}

/******************* Constant utility ******************************/

ConstantValue* makeIntConstant(int i) {
  ConstantValue* value = (ConstantValue*) malloc(sizeof(ConstantValue));
  if (value == NULL) return NULL;
  value->type = TP_INT;
  value->intValue = i;
  return value;
}

ConstantValue* makeCharConstant(char ch) {
  ConstantValue* value = (ConstantValue*) malloc(sizeof(ConstantValue));
  if (value == NULL) return NULL;
  value->type = TP_CHAR;
  value->charValue = ch;
  return value;
}

ConstantValue* duplicateConstantValue(ConstantValue* v) {
  if (v == NULL) return NULL;
  ConstantValue* value = (ConstantValue*) malloc(sizeof(ConstantValue));
  if (value == NULL) return NULL;
  value->type = v->type;
  switch (v->type) {
  case TP_INT:
    value->intValue = v->intValue;
    break;
  case TP_CHAR:
    value->charValue = v->charValue;
    break;
  default:
    break;
  }
  return value;
}


/******************* Object utilities ******************************/

Scope* createScope(Object* owner, Scope* outer) {
  Scope* scope = (Scope*) malloc(sizeof(Scope));
  scope->objList = NULL;
  scope->owner = owner;
  scope->outer = outer;
  return scope;
}

Object* createProgramObject(char *programName) {
  Object* program = (Object*) malloc(sizeof(Object));
  strcpy(program->name, programName);
  program->kind = OBJ_PROGRAM;
  program->progAttrs = (ProgramAttributes*) malloc(sizeof(ProgramAttributes));
  program->progAttrs->scope = createScope(program,NULL);
  symtab->program = program;

  return program;
}

Object* createConstantObject(char *name) {
  Object* obj = (Object*) malloc(sizeof(Object));
  if (obj == NULL) return NULL;
  strcpy(obj->name, name);
  obj->kind = OBJ_CONSTANT;
  obj->constAttrs = (ConstantAttributes*) malloc(sizeof(ConstantAttributes));
  if (obj->constAttrs == NULL) {
    free(obj);
    return NULL;
  }
  obj->constAttrs->value = NULL;
  return obj;
}

Object* createTypeObject(char *name) {
  Object* obj = (Object*) malloc(sizeof(Object));
  if (obj == NULL) return NULL;
  strcpy(obj->name, name);
  obj->kind = OBJ_TYPE;
  obj->typeAttrs = (TypeAttributes*) malloc(sizeof(TypeAttributes));
  if (obj->typeAttrs == NULL) {
    free(obj);
    return NULL;
  }
  obj->typeAttrs->actualType = NULL;
  return obj;
}

Object* createVariableObject(char *name) {
  Object* obj = (Object*) malloc(sizeof(Object));
  if (obj == NULL) return NULL;
  strcpy(obj->name, name);
  obj->kind = OBJ_VARIABLE;
  obj->varAttrs = (VariableAttributes*) malloc(sizeof(VariableAttributes));
  if (obj->varAttrs == NULL) {
    free(obj);
    return NULL;
  }
  obj->varAttrs->type = NULL;
  obj->varAttrs->scope = symtab->currentScope;
  return obj;
}

Object* createFunctionObject(char *name) {
  Object* obj = (Object*) malloc(sizeof(Object));
  if (obj == NULL) return NULL;
  strcpy(obj->name, name);
  obj->kind = OBJ_FUNCTION;
  obj->funcAttrs = (FunctionAttributes*) malloc(sizeof(FunctionAttributes));
  if (obj->funcAttrs == NULL) {
    free(obj);
    return NULL;
  }
  obj->funcAttrs->paramList = NULL;
  obj->funcAttrs->returnType = NULL;
  obj->funcAttrs->scope = createScope(obj, symtab->currentScope);
  return obj;
}

Object* createProcedureObject(char *name) {
  Object* obj = (Object*) malloc(sizeof(Object));
  if (obj == NULL) return NULL;
  strcpy(obj->name, name);
  obj->kind = OBJ_PROCEDURE;
  obj->procAttrs = (ProcedureAttributes*) malloc(sizeof(ProcedureAttributes));
  if (obj->procAttrs == NULL) {
    free(obj);
    return NULL;
  }
  obj->procAttrs->paramList = NULL;
  obj->procAttrs->scope = createScope(obj, symtab->currentScope);
  return obj;
}

Object* createParameterObject(char *name, enum ParamKind kind, Object* owner) {
  Object* obj = (Object*) malloc(sizeof(Object));
  if (obj == NULL) return NULL;
  strcpy(obj->name, name);
  obj->kind = OBJ_PARAMETER;
  obj->paramAttrs = (ParameterAttributes*) malloc(sizeof(ParameterAttributes));
  if (obj->paramAttrs == NULL) {
    free(obj);
    return NULL;
  }
  obj->paramAttrs->kind = kind;
  obj->paramAttrs->type = NULL;
  obj->paramAttrs->function = owner;
  return obj;
}

void freeObject(Object* obj) {
  if (obj == NULL) return;

  switch (obj->kind) {
  case OBJ_CONSTANT:
    if (obj->constAttrs != NULL) {
      if (obj->constAttrs->value != NULL)
        free(obj->constAttrs->value);
      free(obj->constAttrs);
    }
    break;

  case OBJ_TYPE:
    if (obj->typeAttrs != NULL) {
      if (obj->typeAttrs->actualType != NULL)
        freeType(obj->typeAttrs->actualType);
      free(obj->typeAttrs);
    }
    break;

  case OBJ_VARIABLE:
    if (obj->varAttrs != NULL) {
      if (obj->varAttrs->type != NULL)
        freeType(obj->varAttrs->type);
      /* không free varAttrs->scope vì scope đó được quản lý nơi khác */
      free(obj->varAttrs);
    }
    break;

  case OBJ_PARAMETER:
    if (obj->paramAttrs != NULL) {
      if (obj->paramAttrs->type != NULL)
        freeType(obj->paramAttrs->type);
      /* không free paramAttrs->function */
      free(obj->paramAttrs);
    }
    break;

  case OBJ_FUNCTION:
    if (obj->funcAttrs != NULL) {
      if (obj->funcAttrs->paramList != NULL)
        freeReferenceList(obj->funcAttrs->paramList);
      if (obj->funcAttrs->returnType != NULL)
        freeType(obj->funcAttrs->returnType);
      if (obj->funcAttrs->scope != NULL)
        freeScope(obj->funcAttrs->scope);
      free(obj->funcAttrs);
    }
    break;

  case OBJ_PROCEDURE:
    if (obj->procAttrs != NULL) {
      if (obj->procAttrs->paramList != NULL)
        freeReferenceList(obj->procAttrs->paramList);
      if (obj->procAttrs->scope != NULL)
        freeScope(obj->procAttrs->scope);
      free(obj->procAttrs);
    }
    break;

  case OBJ_PROGRAM:
    if (obj->progAttrs != NULL) {
      if (obj->progAttrs->scope != NULL)
        freeScope(obj->progAttrs->scope);
      free(obj->progAttrs);
    }
    break;

  default:
    break;
  }

  free(obj);
}

void freeScope(Scope* scope) {
  if (scope == NULL) return;
  if (scope->objList != NULL)
    freeObjectList(scope->objList);
  /* không free scope->outer ở đây */
  free(scope);
}

void freeObjectList(ObjectNode *objList) {
  ObjectNode *node = objList;
  while (node != NULL) {
    ObjectNode *next = node->next;
    if (node->object != NULL)
      freeObject(node->object);
    free(node);
    node = next;
  }
}

void freeReferenceList(ObjectNode *objList) {
  ObjectNode *node = objList;
  while (node != NULL) {
    ObjectNode *next = node->next;
    /* KHÔNG free node->object, vì object được quản lý ở scope->objList */
    free(node);
    node = next;
  }
}


void addObject(ObjectNode **objList, Object* obj) {
  ObjectNode* node = (ObjectNode*) malloc(sizeof(ObjectNode));
  node->object = obj;
  node->next = NULL;
  if ((*objList) == NULL) 
    *objList = node;
  else {
    ObjectNode *n = *objList;
    while (n->next != NULL) 
      n = n->next;
    n->next = node;
  }
}

Object* findObject(ObjectNode *objList, char *name) {
  ObjectNode *node = objList;
  while (node != NULL) {
    if (node->object != NULL && strcmp(node->object->name, name) == 0)
      return node->object;
    node = node->next;
  }
  return NULL;
}

/******************* others ******************************/

void initSymTab(void) {
  Object* obj;
  Object* param;

  symtab = (SymTab*) malloc(sizeof(SymTab));
  symtab->globalObjectList = NULL;
  
  obj = createFunctionObject("READC");
  obj->funcAttrs->returnType = makeCharType();
  addObject(&(symtab->globalObjectList), obj);

  obj = createFunctionObject("READI");
  obj->funcAttrs->returnType = makeIntType();
  addObject(&(symtab->globalObjectList), obj);

  obj = createProcedureObject("WRITEI");
  param = createParameterObject("i", PARAM_VALUE, obj);
  param->paramAttrs->type = makeIntType();
  addObject(&(obj->procAttrs->paramList),param);
  addObject(&(symtab->globalObjectList), obj);

  obj = createProcedureObject("WRITEC");
  param = createParameterObject("ch", PARAM_VALUE, obj);
  param->paramAttrs->type = makeCharType();
  addObject(&(obj->procAttrs->paramList),param);
  addObject(&(symtab->globalObjectList), obj);

  obj = createProcedureObject("WRITELN");
  addObject(&(symtab->globalObjectList), obj);

  intType = makeIntType();
  charType = makeCharType();
}

void cleanSymTab(void) {
  freeObject(symtab->program);
  freeObjectList(symtab->globalObjectList);
  free(symtab);
  freeType(intType);
  freeType(charType);
}

void enterBlock(Scope* scope) {
  symtab->currentScope = scope;
}

void exitBlock(void) {
  symtab->currentScope = symtab->currentScope->outer;
}

void declareObject(Object* obj) {
  if (obj->kind == OBJ_PARAMETER) {
    Object* owner = symtab->currentScope->owner;
    switch (owner->kind) {
    case OBJ_FUNCTION:
      addObject(&(owner->funcAttrs->paramList), obj);
      break;
    case OBJ_PROCEDURE:
      addObject(&(owner->procAttrs->paramList), obj);
      break;
    default:
      break;
    }
  }
 
  addObject(&(symtab->currentScope->objList), obj);
}


