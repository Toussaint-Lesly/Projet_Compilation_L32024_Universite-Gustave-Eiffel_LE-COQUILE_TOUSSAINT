/**
 * @file symbolTable.c
 * @author Marc LE COQUIL - Lesly Jumelle TOUSSAINT
 * @brief Symbol table 's functions dedicated file.
 * @date 2024-02-10
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "progTable.h"
#include "symbolTable.h"
#include "utilitaries.h"

/**
 * @fn SymbolTable createNewSymbolTable()
 * @brief Create a new symbol table.
 *
 * @return SymbolTable New symbol table.
 */
SymbolTable createNewSymbolTable()
{
    SymbolTable t = {NULL, 0, 0};
    return t;
}

/**
 * @fn void freeSymbolTable(SymbolTable *t)
 * @brief Free a symbol table.
 *
 * @param t SymbolTable* Symbol table to free.
 */
void freeSymbolTable(SymbolTable *t)
{
    if (!t)
        return;
    if (t->symbols)
        free(t->symbols);

    *t = createNewSymbolTable();
}

/* --------------------------------------- ADDING SYMBOL --------------------------------------- */

/**
 * @fn ReturnInfo addRow(SymbolTable *t)
 * @brief Add a row in a symbol table.
 *
 * @param t SymbolTable* Symbol table to add a row on.
 * @return ReturnInfo Eventual error code.
 */
ReturnInfo addRow(SymbolTable *t)
{
    if (!t)
        return NULL_ARGUMENT;

    if (addCell((void **)&t->symbols, t->len, sizeof(Symbol)) != SUCCESS)
        return ALLOC_ERROR;
    t->len++;
    return SUCCESS;
}

/**
 * @fn ReturnInfo isInTable(SymbolTable t, char *id, int *index)
 * @brief Check if a symbol is in a symbol table.
 *
 * @param t SymbolTable Table to check.
 * @param id char* Id of the symbol to check.
 * @param index int* Index of the symbol in the table.
 * @return ReturnInfo Eventual error code.
 */
ReturnInfo isInTable(SymbolTable t, char *id, int *index)
{
    if (!id)
        return NULL_ARGUMENT;
    for (int i = 0; i < t.len; i++)
    {
        if (!strcmp(t.symbols[i].id, id))
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
 * @fn ReturnInfo putSymbolInTable(SymbolTable *t, char *id, AuthorizedType type, int size, int isArray, int isAdress)
 * @brief Put a symbol in a symbol table.
 *
 * @param t SymbolTable* Symbol table to put the symbol on.
 * @param id char* Id of the symbol.
 * @param type AuthorizedType Type of the symbol.
 * @param size int Size of the symbol.
 * @param isArray int Is the symbol an array.
 * @param isAdress int Is the symbol an address (for function argument).
 * @return ReturnInfo Eventual error code.
 */
ReturnInfo putSymbolInTable(SymbolTable *t, char *id, AuthorizedType type, int size, int isArray, int isAdress)
{
    if (!t || !id || !t->symbols || size <= 0)
        return NULL_ARGUMENT;

    if (type == UNAUTHORIZED)
        return FAILURE;

    if (strlen(id) >= SIZE_ID)
        return TOO_LONG_ID;

    sprintf(t->symbols[t->len - 1].id, "%s", id);
    t->symbols[t->len - 1].type = type;
    t->symbols[t->len - 1].numberOfValues = size;
    t->symbols[t->len - 1].isArray = isArray;
    t->symbols[t->len - 1].isAddress = isAdress;

    if (t->len == 1)
        t->symbols[t->len - 1].address = 0;
    else
    {
        int prev_add = t->symbols[t->len - 2].address;
        int prev_size = t->symbols[t->len - 2].isAddress ? 8 : (t->symbols[t->len - 2].isArray ? t->symbols[t->len - 2].type * t->symbols[t->len - 2].numberOfValues : t->symbols[t->len - 2].type);
        t->symbols[t->len - 1].address = prev_add + prev_size;
    }
    t->size += (isArray && size == 1) ? 8 : t->symbols[t->len - 1].type * size;

    return SUCCESS;
}

/**
 * @fn ReturnInfo addSymbol(SymbolTable *t, char *id, char *type, int size, int isArray, int isAdress)
 * @brief Add a symbol in a symbol table and check every field.
 *
 * @param t SymbolTable* Symbol table to add the symbol on.
 * @param id char* Id of the symbol.
 * @param type char* Type of the symbol.
 * @param size int Size of the symbol.
 * @param isArray int Is the symbol an array.
 * @param isAdress int Is the symbol an address (for function argument).
 * @return ReturnInfo Eventual error code.
 */
ReturnInfo addSymbol(SymbolTable *t, char *id, char *type, int size, int isArray, int isAdress)
{
    if (!t || !id || !type)
        return NULL_ARGUMENT;

    ReturnInfo inTable = isInTable(*t, id, NULL);
    if (inTable != ID_NOT_IN_TABLE)
        return inTable;

    AuthorizedType typeKown = getType(type);
    if (typeKown == UNAUTHORIZED)
        return ID_UNAUTHORIZED;

    ReturnInfo rowAdded = addRow(t);
    if (rowAdded != SUCCESS)
        return rowAdded;

    ReturnInfo symbolAdded = putSymbolInTable(t, id, typeKown, size, isArray, isAdress);
    if (symbolAdded != SUCCESS)
        return symbolAdded;

    return SUCCESS;
}

/* ---------------------------------------- PRINT TABLE ---------------------------------------- */

/**
 * @fn int getLengthOfLongestId(SymbolTable t)
 * @brief Get the length of the longest id in a symbol table.
 *
 * @param t SymbolTable Table to get the length of the longest id from.
 * @return int Length of the longest id.
 */
int getLengthOfLongestId(SymbolTable t)
{
    int max = 0;
    int l = 0;
    for (int i = 0; i < t.len; i++)
    {
        l = strlen(t.symbols[i].id);
        max = max < l ? l : max;
    }
    return max;
}

/**
 * @fn void printSymbolTable(SymbolTable t)
 * @brief Print a symbol table.
 *
 * @param t SymbolTable Table to print.
 */
void printSymbolTable(SymbolTable t)
{
    if (!t.len)
    {
        fprintf(stdout, "Empty table.\n");
        return;
    }

    fprintf(stdout, "Total size = %d\n", t.size);

    int maxLen = getLengthOfLongestId(t);
    int columnLength[3] = {maxLen + 2 <= 5 ? 5 : maxLen + 2, 8, 5};
    printTableLine(columnLength, 3);
    fprintf(stdout, "| %s%*s| %s%*s| %s%*s|\n", "ids", (int)(columnLength[0] - strlen("ids")) - 1, " ",
            "type", (int)(columnLength[1] - strlen("type")) - 1, " ",
            "add", (int)(columnLength[2] - strlen("add")) - 1, " ");
    for (int i = 0; i < t.len; i++)
    {
        char typeStr[8];
        strcpy(typeStr, typeToString(t.symbols[i].type));
        char addressStr[4];
        sprintf(addressStr, "%d", t.symbols[i].address);

        printTableLine(columnLength, 3);
        fprintf(stdout, "| %s%*s| %s%*s| %s%*s|\n", t.symbols[i].id, (int)(columnLength[0] - strlen(t.symbols[i].id)) - 1, " ",
                typeStr, (int)(columnLength[1] - strlen(typeStr)) - 1, " ",
                addressStr, (int)(columnLength[2] - strlen(addressStr)) - 1, " ");
    }
    printTableLine(columnLength, 3);
}

/* -------------------------------- CREATION OF TABLES WITH ROOT -------------------------------- */

/**
 * @fn ReturnInfo addParamList(SymbolTable *t, Node *param, ProgTable pt)
 * @brief Add a parameter list in a symbol table.
 *
 * @param t SymbolTable* Symbol table to add the parameter list on.
 * @param param Node* Parameter list to add.
 * @param pt ProgTable Program table to get the type of the parameters.
 * @return ReturnInfo Eventual error code.
 */
ReturnInfo addParamList(SymbolTable *t, Node *param, ProgTable pt)
{
    if (!t || !param)
        return NULL_ARGUMENT;

    for (Node *child = param->firstChild; child != NULL; child = child->nextSibling)
    {
        for (Node *var = child->firstChild; var != NULL; var = var->nextSibling)
        {
            if (var->label == Ident || var->label == Array)
            {
                if (isInTable(*t, var->u.ident, NULL) == ID_IN_TABLE)
                {
                    fprintf(stderr, "Redefinition of %s at line %d.\n", var->u.ident, var->lineno);
                    return ID_IN_TABLE;
                }

                ReturnInfo added = addSymbol(t, var->u.ident, child->u.ident, 1, 0, var->label == Array);
                if (added != SUCCESS)
                    return added;
            }
        }
    }
    return SUCCESS;
}

/**
 * @fn ReturnInfo addListGlobal(SymbolTable *t, Node *globs, ProgTable pt)
 * @brief Add a list of global variables in a symbol table.
 *
 * @param t SymbolTable* Symbol table to add the list of global variables on.
 * @param globs Node* List of global variables to add.
 * @param pt ProgTable Program table to get the type of the global variables.
 * @return ReturnInfo Eventual error code.
 */
ReturnInfo addListGlobal(SymbolTable *t, Node *globs, ProgTable pt)
{
    if (!t || !globs)
        return NULL_ARGUMENT;

    for (Node *child = globs->firstChild; child != NULL; child = child->nextSibling)
    {
        for (Node *var = child->firstChild; var != NULL; var = var->nextSibling)
        {
            if (var->label == Ident || var->label == Array)
            {
                if (isInTable(*t, var->u.ident, NULL) == ID_IN_TABLE)
                {
                    fprintf(stderr, "Redefinition of %s at line %d.\n", var->u.ident, var->lineno);
                    return ID_IN_TABLE;
                }

                ReturnInfo added = addSymbol(t, var->u.ident, child->u.ident, (var->label == Array && var->nextSibling->label == Num) ? var->nextSibling->u.num : 1, var->label == Array, 0);
                if (added != SUCCESS)
                    return added;
            }
        }
    }
    return SUCCESS;
}

/**
 * @fn ReturnInfo addListOfSymbol(SymbolTable *t, Node *node, ProgTable pt)
 * @brief Add a list of symbols in a symbol table.
 *
 * @param t SymbolTable* Symbol table to add the list of symbols on.
 * @param node Node* List of symbols to add.
 * @param pt ProgTable Program table to get the type of the symbols.
 * @return ReturnInfo Eventual error code.
 */
ReturnInfo addListOfSymbol(SymbolTable *t, Node *node, ProgTable pt)
{
    if (!t || !node)
        return NULL_ARGUMENT;
    if (node->label != ParamList && node->label != DeclVarsGlobale && node->label != DeclVarsLocale)
        return NOT_A_LIST;

    if (node->label == ParamList)
        return addParamList(t, node, pt);
    if (node->label == DeclVarsGlobale)
        return addListGlobal(t, node, pt);

    for (Node *child = node->firstChild; child != NULL; child = child->nextSibling)
    {
        for (Node *var = child->firstChild; var != NULL; var = var->nextSibling)
        {
            if (var->label == Ident || var->label == Array)
            {
                if (isInTable(*t, var->u.ident, NULL) == ID_IN_TABLE)
                {
                    fprintf(stderr, "Redefinition of %s at line %d.\n", var->u.ident, var->lineno);
                    return ID_IN_TABLE;
                }

                ReturnInfo added = addSymbol(t, var->u.ident, child->u.ident, (var->firstChild && var->firstChild->label == Num) ? var->firstChild->u.num : 1, var->label == Array, 0);
                if (added != SUCCESS)
                    return added;
            }
        }
    }
    return SUCCESS;
}

/**
 * @fn ReturnInfo addSymbolsTableFromArray(SymbolTable *table, char *symbols[][2])
 * @brief Add a symbol table from an array of symbols.
 *
 * @param table SymbolTable* Symbol table to add the symbols on.
 * @param symbols char*[][2] Array of symbols to add.
 * @return ReturnInfo Eventual error code.
 */
ReturnInfo addSymbolsTableFromArray(SymbolTable *table, char *symbols[][2])
{
    if (symbols)
    {
        for (char **symbol = symbols[0]; symbol[0] != NULL; symbol += 2)
        {
            ReturnInfo symbolAdded = addSymbol(table, symbol[1], symbol[0], 1, 0, 0);
            if (symbolAdded != SUCCESS)
                return symbolAdded;
        }
    }
    return SUCCESS;
}
