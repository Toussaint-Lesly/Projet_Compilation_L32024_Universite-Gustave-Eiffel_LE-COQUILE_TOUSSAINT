/**
 * @file functionTable.c
 * @author Marc LE COQUIL - Lesly Jumelle TOUSSAINT
 * @brief Contain the function's table related functions of the project.
 * @date 2024-02-10
 */

#include "functionTable.h"
#include "progTable.h"
#include "symbolTable.h"
#include "utilitaries.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * @fn FunctionTable createNewFunctionTable()
 * @brief Create a new function table.
 * 
 * @return Empty FunctionTable 
 */
FunctionTable createNewFunctionTable()
{
    FunctionTable t = {NULL, 0, 0};
    return t;
}

/**
 * @fn void freeFunctionTable(FunctionTable *t)
 * @brief Free a function table.
 * 
 * @param t FunctionTable* to be freed.
 */
void freeFunctionTable(FunctionTable *t)
{
    if (!t)
        return;

    for (int i = 0; i < t->len; i++)
    {
        freeSymbolTable(&t->functions[i].args);
        freeSymbolTable(&t->functions[i].locals);
    }
    if (t->functions)
        free(t->functions);

    *t = createNewFunctionTable();
}

/* ---------------------------------------- ADDING SYMBOL ---------------------------------------- */

/**
 * @fn ReturnInfo addRowOfFun(FunctionTable *t)
 * @brief Add a row of function in a function table.
 * 
 * @param t FunctionTable* Function table to add row on.
 * @return ReturnInfo Eventual error code.
 */
ReturnInfo addRowOfFun(FunctionTable *t)
{
    if (!t)
        return NULL_ARGUMENT;

    if (addCell((void **)&t->functions, t->len, sizeof(FunctionInfo)) != SUCCESS)
        return ALLOC_ERROR;

    t->functions[t->len].args = createNewSymbolTable();
    t->functions[t->len].locals = createNewSymbolTable();
    t->len++;

    return SUCCESS;
}

/**
 * @fn ReturnInfo isFunctionInTable(FunctionTable t, char *id, int *index)
 * @brief Check if a function is in a function table based on it's id.
 * 
 * @param t FunctionTable Table to check.
 * @param id char* Id of the function.
 * @param index int* Index of the function in the table.
 * @return ReturnInfo Eventual error code.
 */
ReturnInfo isFunctionInTable(FunctionTable t, char *id, int *index)
{
    if (!id)
        return NULL_ARGUMENT;
    for (int i = 0; i < t.len; i++)
    {
        if (!strcmp(t.functions[i].id, id))
        {
            if (index)
                *index = i;
            return ID_IN_TABLE;
        }
    }
    if (index)
        *index = -1;
    return ID_NOT_IN_TABLE;
}

/**
 * @fn ReturnInfo getFunIdAndType(Node *declFun, char *id, char *type)
 * @brief Get the function id and type from a function node.
 * 
 * @param declFun Node* Function node to get id and type from.
 * @param id char* Id of the function.
 * @param type char* Type of the function.
 * @return ReturnInfo Eventual error code.
 */
ReturnInfo getFunIdAndType(Node *declFun, char *id, char *type)
{
    if (!declFun || !declFun->firstChild || !id || !type)
        return NULL_ARGUMENT;
    if (declFun->label != DeclFonct)
        return NOT_A_FUNCTION;

    Node *typeNode = declFun->firstChild;
    int isVoid = typeNode->label == Void;
    if (typeNode->label != Type && !isVoid)
        return NOT_A_TYPE;

    Node *idNode = isVoid ? typeNode->nextSibling : typeNode->firstChild;
    strcpy(type, isVoid ? "void" : typeNode->u.ident);
    strcpy(id, idNode->u.ident);
    return SUCCESS;
}

/**
 * @fn ReturnInfo addArgs(FunctionTable *t, Node *declFun, ProgTable pt)
 * @brief Add the arguments of a function in a function table.
 * 
 * @param t FunctionTable* Function table to add arguments on.
 * @param declFun Node* Function node to get arguments from.
 * @param pt ProgTable Program table to get the type of the arguments.
 * @return ReturnInfo Eventual error code.
 */
ReturnInfo addArgs(FunctionTable *t, Node *declFun, ProgTable pt)
{
    Node *paramList = getChildLabeled(declFun, ParamList);

    if (!t || !paramList)
        return NULL_ARGUMENT;
    if (declFun->label != DeclFonct)
        return NOT_A_FUNCTION;

    if (paramList->firstChild->label != Void)
    {
        ReturnInfo argsAdded = addListOfSymbol(&t->functions[t->len - 1].args, paramList, pt);
        if (argsAdded != SUCCESS)
            return argsAdded;
    }

    return SUCCESS;
}

/**
 * @fn ReturnInfo addLocals(FunctionTable *t, Node *declFun, ProgTable pt)
 * @brief Add the locals of a function in a function table.
 * 
 * @param t FunctionTable* Function table to add locals on.
 * @param declFun Node* Function node to get locals from.
 * @param pt ProgTable Program table to get the type of the locals.
 * @return ReturnInfo Eventual error code.
 */
ReturnInfo addLocals(FunctionTable *t, Node *declFun, ProgTable pt)
{
    Node *body = getChildLabeled(declFun, Body);

    if (!declFun)
        return NULL_ARGUMENT;
    if (declFun->label != DeclFonct)
        return NOT_A_FUNCTION;

    if (body && body->firstChild && body->firstChild->label == DeclVarsLocale)
    {
        ReturnInfo localsAdded = addListOfSymbol(&t->functions[t->len - 1].locals, body->firstChild, pt);
        if (localsAdded != SUCCESS)
            return localsAdded;
    }

    return SUCCESS;
}

/**
 * @fn ReturnInfo assignNewAddressToLastFunction(FunctionTable *t)
 * @brief Assign a new address to the last function in a function table.
 * 
 * @param t FunctionTable* Function table to assign address on.
 * @return ReturnInfo Eventual error code.
 */
ReturnInfo assignNewAddressToLastFunction(FunctionTable *t)
{
    if (!t)
        return NULL_ARGUMENT;

    int sizeTable = 0;
    for (int i = 0; i < t->functions[t->len - 2].args.len; i++)
        sizeTable += t->functions[t->len - 2].args.symbols[i].type;
    for (int i = 0; i < t->functions[t->len - 2].locals.len; i++)
        sizeTable += t->functions[t->len - 2].locals.symbols[i].type;

    t->functions[t->len - 1].address = t->functions[t->len - 2].address + sizeTable;

    return SUCCESS;
}

/**
 * @fn ReturnInfo assignAddress(FunctionTable *t)
 * @brief Assign a new address to the last function in a function table.
 * 
 * @param t FunctionTable* Function table to assign address on.
 * @return ReturnInfo Eventual error code.
 */
ReturnInfo assignAddress(FunctionTable *t)
{
    if (t->len == 1)
        t->functions[t->len - 1].address = 0;
    else
        return assignNewAddressToLastFunction(t);
    return SUCCESS;
}

/**
 * @fn ReturnInfo addFunction(FunctionTable *t, Node *node, ProgTable pt)
 * @brief Add a function in a function table.
 * 
 * @param t FunctionTable* Function table to add function on.
 * @param node Node* Function node to add function from.
 * @param pt ProgTable Program table to get the type of the function.
 * @return ReturnInfo Eventual error code.
 */
ReturnInfo addFunction(FunctionTable *t, Node *node, ProgTable pt)
{
    if (!t || !node)
        return NULL_ARGUMENT;

    if (node->label != DeclFonct)
        return NOT_A_FUNCTION;

    char id[SIZE_ID], type[SIZE_ID];
    ReturnInfo info = getFunIdAndType(node, id, type);
    if (info != SUCCESS)
        return info;

    ReturnInfo inTable = isFunctionInTable(*t, id, NULL);
    if (inTable != ID_NOT_IN_TABLE || isInTable(pt.glob, id, NULL) != ID_NOT_IN_TABLE)
    {
        fprintf(stderr, "Function %s is redefined here %d.\n", id, node->lineno);
        return inTable;
    }

    ReturnInfo rowAdded = addRowOfFun(t);
    if (rowAdded != SUCCESS)
        return rowAdded;

    sprintf(t->functions[t->len - 1].id, "%s", id);
    t->functions[t->len - 1].type = getType(type);

    ReturnInfo argsAdded = addArgs(t, node, pt);
    if (argsAdded != SUCCESS)
        return argsAdded;

    ReturnInfo argsLocals = addLocals(t, node, pt);
    if (argsLocals != SUCCESS)
        return argsLocals;

    ReturnInfo addAssigned = assignAddress(t);
    if (addAssigned != SUCCESS)
        return addAssigned;

    t->size += t->functions[t->len - 1].args.size + t->functions[t->len - 1].locals.size;

    return SUCCESS;
}

/* ---------------------------------------- PRINT TABLE ---------------------------------------- */

/**
 * @fn int getLengthOfLongestFunId(FunctionTable t)
 * @brief Get the length of the longest function id in a function table.
 * 
 * @param t FunctionTable Table to get the length of the longest function id from.
 * @return int Length of the longest function id.
 */
int getLengthOfLongestFunId(FunctionTable t)
{
    int max = 0;
    int l = 0;
    for (int i = 0; i < t.len; i++)
    {
        l = strlen(t.functions[i].id);
        max = max < l ? l : max;
    }
    return max;
}

/**
 * @fn void printFunctionTable(FunctionTable t)
 * @brief Print a function table.
 * 
 * @param t FunctionTable Table to print.
 */
void printFunctionTable(FunctionTable t)
{
    if (!t.len)
    {
        fprintf(stdout, "Empty table.\n");
        return;
    }

    fprintf(stdout, "Total size = %d\n", t.size);

    int maxLen = getLengthOfLongestFunId(t);
    int columnLength[5] = {maxLen + 2 <= 3 ? 3 : maxLen + 2, 8, 8, 9, 5};
    printTableLine(columnLength, 5);
    fprintf(stdout, "| %s%*s| %s%*s| %s%*s| %s%*s| %s%*s|\n", "ids",
            (int)(columnLength[0] - strlen("ids") - 1), " ", "type",
            (int)(columnLength[1] - strlen("type") - 1), " ", "nbArgs",
            (int)(columnLength[2] - strlen("nbArgs") - 1), " ", "nbLocal",
            (int)(columnLength[3] - strlen("nbLocal") - 1), " ", "add",
            (int)(columnLength[4] - strlen("add") - 1), " ");

    for (int i = 0; i < t.len; i++)
    {
        char typeStr[8];
        strcpy(typeStr, typeToString(t.functions[i].type));
        char nbArgs[7];
        sprintf(nbArgs, "%d", t.functions[i].args.len);
        char nbLocal[7];
        sprintf(nbLocal, "%d", t.functions[i].locals.len);
        char addressStr[4];
        sprintf(addressStr, "%d", t.functions[i].address);

        printTableLine(columnLength, 5);
        fprintf(stdout, "| %s%*s| %s%*s| %s%*s| %s%*s| %s%*s|\n", t.functions[i].id,
                (int)(columnLength[0] - strlen(t.functions[i].id)) - 1, " ", typeStr,
                (int)(columnLength[1] - strlen(typeStr)) - 1, " ", nbArgs,
                (int)(columnLength[2] - strlen(nbArgs)) - 1, " ", nbLocal,
                (int)(columnLength[3] - strlen(nbLocal)) - 1, " ", addressStr,
                (int)(columnLength[4] - strlen(addressStr)) - 1, " ");
    }
    printTableLine(columnLength, 5);
}

/**
 * @fn void printOneFunction(FunctionInfo fun)
 * @brief Print one function of a function table.
 * 
 * @param fun FunctionInfo Function to print.
 */
void printOneFunction(FunctionInfo fun)
{
    fprintf(stdout, "Function shown: %s\n", fun.id);
    fprintf(stdout, "Argument:\n");
    printSymbolTable(fun.args);
    fprintf(stdout, "\n");
    fprintf(stdout, "Local variable:\n");
    printSymbolTable(fun.locals);
    fprintf(stdout, "\n");
}

/* --------------------------- CREATION OF FUNCTION TABLES WITH ROOT
 * --------------------------- */

/**
 * @fn ReturnInfo addFunctions(FunctionTable *t, Node *root, ProgTable pt)
 * @brief Add functions in a function table.
 * 
 * @param t FunctionTable* Function table to add functions on.
 * @param root Node* Root of the tree to add functions from.
 * @param pt ProgTable Program table to get the type of the functions.
 * @return ReturnInfo Eventual error code.
 */
ReturnInfo addFunctions(FunctionTable *t, Node *root, ProgTable pt)
{
    if (!t || !root || !root->firstChild)
        return NULL_ARGUMENT;

    Node *child = root->firstChild;
    while (child && child->label != DeclFonct)
        child = child->nextSibling;

    for (; child != NULL; child = child->nextSibling)
    {
        ReturnInfo added = addFunction(t, child, pt);
        if (added != SUCCESS)
            return added;
    }
    return SUCCESS;
}

/**
 * @fn ReturnInfo setupFunction(FunctionTable *t, char *id, char *type)
 * @brief Setup a function in a function table.
 * 
 * @param t FunctionTable* Function table to setup function on.
 * @param id char* Id of the function.
 * @param type char* Type of the function.
 * @return ReturnInfo Eventual error code.
 */
ReturnInfo setupFunction(FunctionTable *t, char *id, char *type)
{
    sprintf(t->functions[t->len - 1].id, "%s", id);
    t->functions[t->len - 1].type = getType(type);
    return SUCCESS;
}

/**
 * @fn ReturnInfo addDefaultFunction(FunctionTable *t, char *id, char *type, char *args[][2], char *locals[][2])
 * @brief Add a default function in a function table.
 * 
 * @param t FunctionTable* Function table to add default function on.
 * @param id char* Id of the default function.
 * @param type char* Type of the default function.
 * @param args char*[][2] Arguments of the default function.
 * @param locals char*[][2] Locals of the default function.
 * @return ReturnInfo Eventual error code.
 */
ReturnInfo addDefaultFunction(FunctionTable *t, char *id, char *type, char *args[][2], char *locals[][2])
{
    if (!t || !id || !type)
        return NULL_ARGUMENT;

    ReturnInfo rowAdded = addRowOfFun(t);
    if (rowAdded != SUCCESS)
        return rowAdded;

    ReturnInfo setupResult = setupFunction(t, id, type);
    if (setupResult != SUCCESS)
        return setupResult;

    ReturnInfo argsResult = addSymbolsTableFromArray(&t->functions[t->len - 1].args, args);
    if (argsResult != SUCCESS)
        return argsResult;

    ReturnInfo localsResult = addSymbolsTableFromArray(&t->functions[t->len - 1].locals, locals);
    if (localsResult != SUCCESS)
        return localsResult;

    ReturnInfo addressResult = assignAddress(t);
    if (addressResult != SUCCESS)
        return addressResult;

    t->size += t->functions[t->len - 1].args.size + t->functions[t->len - 1].locals.size;

    return SUCCESS;
}

/**
 * @fn ReturnInfo addDefaultFunctions(FunctionTable *t)
 * @brief Add the default functions in a function table.
 * 
 * @param t FunctionTable* Function table to add default functions on.
 * @return ReturnInfo Eventual error code.
 */
ReturnInfo addDefaultFunctions(FunctionTable *t)
{
    if (!t)
        return NULL_ARGUMENT;

    char *getCharArgs[1][2] = {{NULL, NULL}};
    char *putCharArgs[2][2] = {{"char", "c"}, {NULL, NULL}};
    char *getIntArgs[1][2] = {{NULL, NULL}};
    char *putIntArgs[2][2] = {{"int", "i"}, {NULL, NULL}};

    ReturnInfo added = addDefaultFunction(t, "getChar", "char", getCharArgs, NULL);
    if (added != SUCCESS)
        return added;
    added = addDefaultFunction(t, "putChar", "void", putCharArgs, NULL);
    if (added != SUCCESS)
        return added;
    added = addDefaultFunction(t, "getInt", "int", getIntArgs, NULL);
    if (added != SUCCESS)
        return added;
    added = addDefaultFunction(t, "putInt", "void", putIntArgs, NULL);

    return added;
}
