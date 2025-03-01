/**
 * @file progTable.c
 * @author Marc LE COQUIL - Lesly Jumelle TOUSSAINT
 * @brief Program table 's function dedicated file.
 * @date 2024-02-10
 */

#include "progTable.h"
#include "functionTable.h"
#include "symbolTable.h"
#include "utilitaries.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * @fn ProgTable createNewProgTable()
 * @brief Create a new program table.
 * 
 * @return ProgTable New program table.
 */
ProgTable createNewProgTable()
{
    ProgTable t = {createNewSymbolTable(), createNewFunctionTable(), 0};
    return t;
}

/**
 * @fn ReturnInfo fillProgTable(ProgTable *t, Node *root)
 * @brief Fill a program table with the tree.
 * 
 * @param t ProgTable* Program table to fill.
 * @param root Node* Root of the tree to fill the program table with.
 * @return ReturnInfo Eventual error code.
 */
ReturnInfo fillProgTable(ProgTable *t, Node *root)
{
    if (!t || !root)
        return NULL_ARGUMENT;
    Node *declGlob = getChildLabeled(root, DeclVarsGlobale);
    if (declGlob)
    {
        ReturnInfo localsAdded = addListOfSymbol(&t->glob, declGlob, *t);
        if (localsAdded != SUCCESS)
            return localsAdded;
    }

    ReturnInfo defaultFunctionsAdded = addDefaultFunctions(&t->functions);
    if (defaultFunctionsAdded != SUCCESS)
        return defaultFunctionsAdded;

    ReturnInfo functionsAdded = addFunctions(&t->functions, root, *t);
    if (functionsAdded != SUCCESS)
        return functionsAdded;

    t->size += t->glob.size;
    t->size += t->functions.size;

    return SUCCESS;
}

/**
 * @fn FunctionInfo getFunctionsTable(ProgTable t, char *funName)
 * @brief Get a function from a program table based on it's name.
 * 
 * @param t ProgTable Table to get the function from.
 * @param funName char* Name of the function.
 * @return FunctionInfo Function found.
 */
FunctionInfo getFunctionsTable(ProgTable t, char *funName)
{
    for (int i = 0; i < t.functions.len || !funName; i++)
    {
        if (!strcmp(t.functions.functions[i].id, funName))
            return t.functions.functions[i];
    }
    FunctionInfo err = {"err"};
    return err;
}

/**
 * @fn void freeProgTable(ProgTable *t)
 * @brief Free a program table.
 * 
 * @param t ProgTable* Program table to free.
 */
void freeProgTable(ProgTable *t)
{
    if (!t)
        return;
    freeSymbolTable(&t->glob);
    freeFunctionTable(&t->functions);
    t->size = 0;
}

/**
 * @fn void printProgTable(ProgTable t, int showAllTables, int showAllFunctions, char *functionToShow, int showGlobals)
 * @brief Print a program table.
 * 
 * @param t ProgTable Table to print.
 * @param showAllTables int Show all the tables.
 * @param showAllFunctions int Show all the functions.
 * @param functionToShow char* Function to show.
 * @param showGlobals int Show the globals.
 */
void printProgTable(ProgTable t, int showAllTables, int showAllFunctions,
                    char *functionToShow, int showGlobals)
{
    if (showGlobals || showAllTables)
    {
        fprintf(stdout, "Global variable's table:\n");
        printSymbolTable(t.glob);
        fprintf(stdout, "\n");
    }

    if (showAllFunctions || showAllTables)
    {
        fprintf(stdout, "Function's table:\n");
        printFunctionTable(t.functions);
        fprintf(stdout, "\n");
    }

    if (showAllTables)
    {
        for (int i = 0; i < t.functions.len; i++)
        {
            printOneFunction(t.functions.functions[i]);
            fprintf(stdout, "\n");
        }
    }

    if (strlen(functionToShow))
    {
        FunctionInfo toShow = getFunctionsTable(t, functionToShow);
        if (strcmp(toShow.id, "err"))
        {
            printOneFunction(toShow);
            fprintf(stdout, "\n");
        }
        else
            fprintf(stderr, "Function %s not found\n", functionToShow);
    }
}
