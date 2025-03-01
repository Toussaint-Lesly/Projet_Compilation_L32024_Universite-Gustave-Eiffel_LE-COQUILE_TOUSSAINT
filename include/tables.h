#ifndef __TABLE_H__
#define __TABLE_H__

#include "tree.h"

typedef enum
{
    VOID_TYPE = 0,
    CHAR = 1,
    UNAUTHORIZED = 2,
    INT = 4
} AuthorizedType;

typedef struct _symbol
{
    char id[SIZE_ID];
    AuthorizedType type;
    int address;
    int numberOfValues;
    int isArray;
    int isAddress;
} Symbol;

typedef struct _table
{
    Symbol *symbols;
    int len;
    int size;
} SymbolTable;

typedef struct _function_info
{
    char id[SIZE_ID];
    AuthorizedType type;
    SymbolTable args;
    SymbolTable locals;
    int address;
} FunctionInfo;

typedef struct _function_table
{
    FunctionInfo *functions;
    int len;
    int size;
} FunctionTable;

typedef struct _prog_table
{
    SymbolTable glob;
    FunctionTable functions;
    int size;
} ProgTable;

typedef enum
{
    NULL_ARGUMENT,
    ALLOC_ERROR,
    ID_IN_TABLE,
    ID_NOT_IN_TABLE,
    ID_UNAUTHORIZED,
    SUCCESS,
    FAILURE,
    TOO_LONG_ID,
    NOT_A_FUNCTION,
    NOT_A_LIST,
    NOT_A_TYPE,
    NOT_A_BODY,
    COULD_NOT_OPEN_FILE,
    UNKOWN_BOOLEAN_OPERATION,
    ARG_WRONG_TYPE,
    TOO_MANY_ARGUMENT,
    TOO_FEW_ARGUMENT,
    VOID_OPERATION,
    MISSING_RETURN_VALUE,
    INVALID_ARGUMENT_TYPE,
    NO_MAIN_FUNCTION,
    ARRAY_UNEXPECTED,
    ARRAY_EXPECTED,
    VOID_ADDSUB,
    VOID_ASSIGNMENT,
    VOID_COMPARATION,
    VOID_RETURN_ILLEGAL,
    VOID_ARGUMENT_PASSED,
    VOID_INDEX,
    VOID_DIVSTA
} ReturnInfo;

#endif
