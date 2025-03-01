#ifndef __FUNCTION_TABLE_H__
#define __FUNCTION_TABLE_H__

#include "tables.h"

FunctionTable createNewFunctionTable();

void freeFunctionTable(FunctionTable *t);

ReturnInfo isFunctionInTable(FunctionTable t, char *id, int *index);

ReturnInfo addFunctions(FunctionTable *t, Node *root, ProgTable pt);

ReturnInfo addDefaultFunctions(FunctionTable *t);

void printFunctionTable(FunctionTable t);

void printOneFunction(FunctionInfo fun);

#endif