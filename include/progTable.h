#ifndef __PROG_TABLE_H__
#define __PROG_TABLE_H__

#include "tables.h"
#include "symbolTable.h"

ProgTable createNewProgTable();

ReturnInfo fillProgTable(ProgTable *t, Node *root);

FunctionInfo getFunctionsTable(ProgTable t, char *funName);

void freeProgTable(ProgTable *t);

void printProgTable(ProgTable t, int showAllTables, int showAllFunctions, char *functionToShow, int showGlobals);

#endif

