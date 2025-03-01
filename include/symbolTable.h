#ifndef __SYMBOL_TABLE_H__
#define __SYMBOL_TABLE_H__

#define EMPTY_ID "                                                               "

#include "tables.h"

SymbolTable createNewSymbolTable();

void freeSymbolTable(SymbolTable *t);

ReturnInfo isInTable(SymbolTable t, char *id, int *index);

AuthorizedType getType(char *type);

char *typeToString(AuthorizedType type);

void printSymbolTable(SymbolTable t);

ReturnInfo addListOfSymbol(SymbolTable *t, Node *node, ProgTable pt);

ReturnInfo addSymbolsTableFromArray(SymbolTable *table, char *symbols[][2]);

#endif
