#ifndef __UTILITARIES_H__
#define __UTILITARIES_H__

#include "tables.h"

void printRowsOfChar(char c, int nb);

void printTableLine(int *columnLength, int columnNb);

AuthorizedType getType(char *type);

char *typeToString(AuthorizedType type);

ReturnInfo addCell(void **arr, int len, unsigned long size);

int optionHandler(int argc, char **argv, int *showAllTables,
                  int *showAllFunctions, char *functionToShow, int *showGlobals,
                  int *printTreeOption, char *outputName);

Node *getChildLabeled(Node *node, label_t label);

ReturnInfo printReturnInfo(ReturnInfo info);

ReturnInfo getFunId(Node *declFun, char *funId);

int charToAsciiCode(const char *character);

int getErrorCode(ReturnInfo info);

int min(int a, int b);
int max(int a, int b);

char *sizeToAsm(int size);

label_t getExpressionType(Node *expr, ProgTable pt, FunctionInfo funTable);

#endif
