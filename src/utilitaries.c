/**
 * @file utilitaries.c
 * @author Marc LE COQUIL - Lesly Jumelle TOUSSAINT
 * @brief Contain the utilitaries functions of the project.
 * @date 2024-02-10
 */

#include "utilitaries.h"
#include "progTable.h"

#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * @fn void printRowsOfChar(char c, int nb)
 * @brief Print nb times the character c.
 *
 * @param c The character to print.
 * @param nb The number of times to print the character.
 */
void printRowsOfChar(char c, int nb)
{
    for (int i = 0; i < nb; i++)
        fprintf(stdout, "%c", c);
}

/**
 * @fn void printTableLine(int *columnLength, int columnNb)
 * @brief Print a line of the table.
 *
 * @param columnLength The length of each column.
 * @param columnNb The number of columns.
 */
void printTableLine(int *columnLength, int columnNb)
{
    for (int i = 0; i < columnNb; i++)
    {
        fprintf(stdout, "+");
        printRowsOfChar('-', columnLength[i]);
    }
    fprintf(stdout, "+\n");
}

/**
 * @fn ReturnInfo addCell(void **arr, int len, unsigned long size)
 * @brief Add a cell to the array.
 *
 * @param arr The array to add to.
 * @param len The length of the array.
 * @param size The size of the cell.
 * @return ReturnInfo The return info.
 */
ReturnInfo addCell(void **arr, int len, unsigned long size)
{
    if (!arr || !size)
        return NULL_ARGUMENT;

    if (!(*arr))
        *arr = malloc(size);
    else
        *arr = realloc(*arr, (len + 1) * size);

    return *arr == NULL ? ALLOC_ERROR : SUCCESS;
}

/**
 * @fn void help(const char *programName)
 * @brief Print the help of the program.
 *
 * @param programName The name of the program.
 */
void help(const char *programName)
{
    fprintf(stdout,
            "\n----------------------------------------------------------"
            "-------------------\n");
    fprintf(stdout, "\nDescription of the program user interface :\n\n");
    fprintf(stdout, "   %s [OPTIONS]\n", programName);
    fprintf(stdout, "\nAvailable options :\n\n");
    fprintf(stdout,
            "   -s, --symtabs : Print all table of symbol of the program.\n");
    fprintf(
        stdout,
        "   -F, --all-functions : Print the table of function of the program.\n");
    fprintf(stdout,
            "   -f [function_name], --function-table [function_name]: "
            "Print the table of fsymbol of a specific function.\n");
    fprintf(stdout,
            "   -g, --global-table : Print the table of symbol of the "
            "global variables of the program.\n");
    fprintf(stdout,
            "   -t, --tree : Print the abstract tree created after the "
            "analysis of the program.\n");
    fprintf(stdout,
            "   -h, --help : Displays a description of the user interface "
            "and terminates execution.\n");
    fprintf(stdout, "\nReturn :\n\n");
    fprintf(stdout,
            "   0 : The source program contains no lexical or syntactic errors\n");
    fprintf(stdout,
            "   1 : The source program contains at least one lexical or "
            "syntactic error\n");
    fprintf(stdout,
            "   2 : The source program contains at least one semantic error\n");
    fprintf(stdout,
            "   3 (or more) : For other kinds of errors (error in command line, "
            "\n\t feature not implemented, out of memory, and so on...)\n");
    fprintf(stdout, "\nTo analyze a file, you can directly enter :\n");
    fprintf(stdout, "   %s [OPTIONS] < file_path\n", programName);
    fprintf(stdout,
            "\n----------------------------------------------------------"
            "--------------------\n\n");
}

/**
 * @fn AuthorizedType getType(char *type)
 * @brief Get the type of the node.
 *
 * @param type The type of the node.
 * @return AuthorizedType The authorized type.
 */
AuthorizedType getType(char *type)
{
    if (!type)
        return UNAUTHORIZED;

    if (!strcmp(type, "char"))
        return CHAR;
    else if (!strcmp(type, "int"))
        return INT;
    else if (!strcmp(type, "void"))
        return VOID_TYPE;
    else
        return UNAUTHORIZED;
}

/**
 * @fn char *typeToString(AuthorizedType type)
 * @brief Convert the type to a string.
 *
 * @param type The type to convert.
 * @return char* The string of the type.
 */
char *typeToString(AuthorizedType type)
{
    switch (type)
    {
    case CHAR:
        return "char";
    case INT:
        return "int";
    case VOID_TYPE:
        return "void";
    default:
        return "Unkown";
    }
}

/**
 * @fn int optionSwitch(int opt, char *exec, int *showAllTables, int *showAllFunctions, char *functionToShow, int *showGlobals, int *printTreeOption, char *outputName)
 * @brief Handle the option switch.
 *
 * @param opt The option to handle.
 * @param exec The name of the executable.
 * @param showAllTables The flag to show all the tables.
 * @param showAllFunctions The flag to show all the functions.
 * @param functionToShow The function to show.
 * @param showGlobals The flag to show the globals.
 * @param printTreeOption The flag to print the tree.
 * @param outputName The name of the output file.
 * @return int The return verification value.
 */
int optionSwitch(int opt, char *exec, int *showAllTables, int *showAllFunctions, char *functionToShow, int *showGlobals, int *printTreeOption, char *outputName)
{
    switch (opt)
    {
    case 's':
        *showAllTables = 1;
        break;
    case 'F':
        *showAllFunctions = 1;
        break;
    case 'f':
        if (functionToShow && strlen(optarg) < SIZE_ID)
            strcpy(functionToShow, optarg);
        break;
    case 'g':
        *showGlobals = 1;
        break;
    case 't':
        *printTreeOption = 1;
        break;
    case 'o':
        if (outputName && strlen(optarg) < SIZE_ID)
            strcpy(outputName, optarg);
        break;
    case '?':
        fprintf(
            stderr,
            "\n/!\\ an unknow option is provided in the command line /!\\\n\n");
    case 'h':
        help(exec);
        exit(EXIT_SUCCESS);
    }
    return 0;
}

/**
 * @fn int optionHandler(int argc, char **argv, int *showAllTables, int *showAllFunctions, char *functionToShow, int *showGlobals, int *printTreeOption, char *outputName)
 * @brief Handle the options of the program.
 *
 * @param argc The number of arguments.
 * @param argv The arguments.
 * @param showAllTables The flag to show all the tables.
 * @param showAllFunctions The flag to show all the functions.
 * @param functionToShow The function to show.
 * @param showGlobals The flag to show the globals.
 * @param printTreeOption The flag to print the tree.
 * @param outputName The name of the output file.
 * @return int The return verification value.
 */
int optionHandler(int argc, char **argv, int *showAllTables, int *showAllFunctions, char *functionToShow, int *showGlobals, int *printTreeOption, char *outputName)
{
    int opt;

    static struct option long_options[] = {
        {"symtabs", no_argument, NULL, 's'},
        {"all-functions", no_argument, NULL, 'F'},
        {"function-table", required_argument, NULL, 'f'},
        {"global-table", no_argument, NULL, 'g'},
        {"tree", no_argument, NULL, 't'},
        {"output", required_argument, NULL, 'o'},
        {"help", no_argument, NULL, 'h'},
        {0, 0, 0, 0}};

    while ((opt = getopt_long(argc, argv, "sFf:gtho:", long_options, NULL)) != -1)
    {
        int switchRet =
            optionSwitch(opt, argv[0], showAllTables, showAllFunctions, functionToShow, showGlobals, printTreeOption, outputName);
        if (switchRet)
            return switchRet;
    }
    return 0;
}

/**
 * @fn Node *getChildLabeled(Node *node, label_t label)
 * @brief Get the first child labeled.
 *
 * @param node The node to get the child from.
 * @param label The expected label of the child.
 * @return Node* The child.
 */
Node *getChildLabeled(Node *node, label_t label)
{
    if (!node || !node->firstChild)
        return NULL;

    Node *child = node->firstChild;
    while (child && child->label != label)
        child = child->nextSibling;
    return child;
}

/**
 * @fn ReturnInfo printReturnInfo(ReturnInfo info)
 * @brief Print the return info.
 *
 * @param info The info to print.
 * @return ReturnInfo The return info.
 */
ReturnInfo printReturnInfo(ReturnInfo info)
{
    return SUCCESS;
}
// ReturnInfo printReturnInfo(ReturnInfo info)
// {
//     switch (info)
//     {
//     case NULL_ARGUMENT:
//         fprintf(stderr, "A null argument was provided.\n");
//         return NULL_ARGUMENT;
//     case ALLOC_ERROR:
//         fprintf(stderr, "An allocation error has occured.\n");
//         return ALLOC_ERROR;
//     case ID_IN_TABLE:
//         fprintf(stderr, "This ID is already in the table.\n");
//         return ID_IN_TABLE;
//     case ID_NOT_IN_TABLE:
//         fprintf(stderr, "This ID is not in the table.\n");
//         return ID_NOT_IN_TABLE;
//     case ID_UNAUTHORIZED:
//         fprintf(stderr, "This ID is not authorized.\n");
//         return ID_UNAUTHORIZED;
//     case SUCCESS:
//         fprintf(stdout, "Your function successfully returned.\n");
//         return SUCCESS;
//     case FAILURE:
//         fprintf(stderr, "Your function failed.\n");
//         return FAILURE;
//     case TOO_LONG_ID:
//         fprintf(stderr, "The ID provided is too long.\n");
//         return TOO_LONG_ID;
//     case NOT_A_FUNCTION:
//         fprintf(stderr, "The ID provided is not refering to a kown function.\n");
//         return NOT_A_FUNCTION;
//     case NOT_A_LIST:
//         fprintf(stderr, "The ID provided is not refering to a kown list.\n");
//         return NOT_A_LIST;
//     case NOT_A_TYPE:
//         fprintf(stderr, "The ID provided is not refering to a kown type.\n");
//         return NOT_A_TYPE;
//     case NOT_A_BODY:
//         fprintf(stderr, "Function body expected.\n");
//         return NOT_A_BODY;
//     case COULD_NOT_OPEN_FILE:
//         fprintf(stderr, "Could not open the file. Probably non-existing.\n");
//         return COULD_NOT_OPEN_FILE;
//     case UNKOWN_BOOLEAN_OPERATION:
//         fprintf(stderr, "Unkown boolean operation.\n");
//         return UNKOWN_BOOLEAN_OPERATION;
//     case ARG_WRONG_TYPE:
//         fprintf(stderr, "The type of the argument is not the same as the function's.\n");
//         return ARG_WRONG_TYPE;
//     case TOO_MANY_ARGUMENT:
//         fprintf(stderr, "Trying to call a function with too many arguments.\n");
//         return TOO_MANY_ARGUMENT;
//     case TOO_FEW_ARGUMENT:
//         fprintf(stderr, "Trying to call a function with too few arguments.\n");
//         return TOO_FEW_ARGUMENT;
//     case VOID_OPERATION:
//         fprintf(stderr, "Trying to perform a void-like operation (comparaison, assignation, etc).\n");
//         return VOID_OPERATION;
//     case MISSING_RETURN_VALUE:
//         fprintf(stderr, "Missing return value in a non-void-typed function.\n");
//         return MISSING_RETURN_VALUE;
//     case INVALID_ARGUMENT_TYPE:
//         fprintf(stderr, "Invalid argument type.\n");
//         return INVALID_ARGUMENT_TYPE;
//     case NO_MAIN_FUNCTION:
//         fprintf(stderr, "No main function found in the program.\n");
//         return NO_MAIN_FUNCTION;
//     case ARRAY_UNEXPECTED:
//         fprintf(stderr, "Array unexpected.\n");
//         return ARRAY_UNEXPECTED;
//     }
//     return SUCCESS;
// }

/**
 * @fn int getErrorCode(ReturnInfo info)
 * @brief Get the error code of the return info.
 *
 * @param info The info to get the error code from.
 * @return int The error code.
 */
int getErrorCode(ReturnInfo info)
{
    switch (info)
    {
    case ID_UNAUTHORIZED:
    case TOO_LONG_ID:
    case UNKOWN_BOOLEAN_OPERATION:
    case ARG_WRONG_TYPE:
    case TOO_MANY_ARGUMENT:
    case TOO_FEW_ARGUMENT:
    case VOID_ADDSUB:     // - 1
    case VOID_ASSIGNMENT: // - 2
    case VOID_COMPARATION:
    case VOID_RETURN_ILLEGAL:
    case VOID_ARGUMENT_PASSED:
    case VOID_INDEX:
    case VOID_DIVSTA:
    case ARRAY_EXPECTED:
    case MISSING_RETURN_VALUE:
    case INVALID_ARGUMENT_TYPE:
    case NO_MAIN_FUNCTION:
    case ARRAY_UNEXPECTED:
    case ID_IN_TABLE: // - 2
    case ID_NOT_IN_TABLE:

    case NULL_ARGUMENT:
        return 2;
    case ALLOC_ERROR:
    case COULD_NOT_OPEN_FILE:
    case NOT_A_LIST:
    case NOT_A_TYPE:
    case NOT_A_BODY:
        return 3;
    default:
        return 0;
    }
}

/**
 * @fn ReturnInfo getFunId(Node *declFun, char *funId)
 * @brief Get the function id.
 *
 * @param declFun The declaration's node.
 * @param funId The function id.
 * @return ReturnInfo The return info.
 */
ReturnInfo getFunId(Node *declFun, char *funId)
{
    if (!declFun || !funId)
        return NULL_ARGUMENT;
    if (declFun->label != DeclFonct)
        return NOT_A_FUNCTION;

    if (declFun->firstChild->label == Void)
        strcpy(funId, declFun->firstChild->nextSibling->u.ident);
    else
        strcpy(funId, declFun->firstChild->firstChild->u.ident);
    return SUCCESS;
}

/**
 * @fn int charToAsciiCode(const char *character)
 * @brief Convert a character to its ASCII code.
 *
 * @param character The character to convert.
 * @return int The ASCII code of the character.
 */
int charToAsciiCode(const char *character)
{
    if (character == NULL)
    {
        fprintf(stderr, "Invalid input: NULL pointer provided.\n");
        return -1; // Return -1 to indicate error
    }

    // Handling special escape sequences
    if (strcmp(character, "\\n") == 0)
        return 10; // ASCII for newline
    else if (strcmp(character, "\\t") == 0)
        return 9; // ASCII for horizontal tab
    else if (strcmp(character, "\\r") == 0)
        return 13; // ASCII for carriage return
    else if (strcmp(character, "\\b") == 0)
        return 8; // ASCII for backspace
    else if (strcmp(character, "\\f") == 0)
        return 12; // ASCII for form feed
    else if (strcmp(character, "\\a") == 0)
        return 7; // ASCII for alert (bell)
    else if (strcmp(character, "\\v") == 0)
        return 11; // ASCII for vertical tab
    else if (strcmp(character, "\\0") == 0)
        return 0; // ASCII for null character
    else if (strlen(character) == 1)
        return (int)character[0]; // Direct ASCII conversion for regular characters
    else
    {
        fprintf(stderr, "Invalid input: expected a single character or a recognized escape sequence.\n");
        return -1; // Return -1 to indicate error
    }
}

/**
 * @fn int min(int a, int b)
 * @brief Get the minimum between a and b.
 *
 * @param a The first number.
 * @param b The second number.
 * @return int The minimum between a and b.
 */
int min(int a, int b)
{
    return a < b ? a : b;
}

/**
 * @fn int max(int a, int b)
 * @brief Get the maximum between a and b.
 *
 * @param a The first number.
 * @param b The second number.
 * @return int The maximum between a and b.
 */
int max(int a, int b)
{
    return a > b ? a : b;
}

/**
 * @fn char *sizeToAsm(int size)
 * @brief Convert the size to its assembly equivalent.
 * 
 * @param size The size to convert.
 * @return char* The assembly equivalent.
 */
char *sizeToAsm(int size)
{
    if (!size)
        return NULL;
    switch (size)
    {
    case 1:
        return "resb";
    case 2:
        return "resw";
    case 4:
        return "resd";
    default:
        return "resq";
    }
}

/**
 * @fn label_t getExpressionType(Node *expr, ProgTable pt, FunctionInfo funTable)
 * @brief Get the type of the expression.
 * 
 * @param expr The expression to get the type from.
 * @param pt The program table.
 * @param funTable The function table we are in.
 * @return label_t The type of the expression.
 */
label_t getExpressionType(Node *expr, ProgTable pt, FunctionInfo funTable)
{
    int index;
    switch (expr->label)
    {
    case Array:
        if (!expr->firstChild)
            return Address;
    case Ident:
        if (isInTable(funTable.locals, expr->u.ident, &index) == ID_IN_TABLE)
            return (funTable.locals.symbols[index].isArray && !expr->firstChild) ? Address : (funTable.locals.symbols[index].type == INT ? Num : Character);
        else if (isInTable(funTable.args, expr->u.ident, &index) == ID_IN_TABLE)
            return (funTable.args.symbols[index].isArray && !expr->firstChild) ? Address : (funTable.args.symbols[index].type == INT ? Num : Character);
        else if (isInTable(pt.glob, expr->u.ident, &index) == ID_IN_TABLE)
            return (pt.glob.symbols[index].isArray && !expr->firstChild) ? Address : (pt.glob.symbols[index].type == INT ? Num : Character);
        else
        {
            FunctionInfo call = getFunctionsTable(pt, expr->u.ident);
            if (!strcmp(call.id, "err"))
                return Void;
            return call.type == INT ? Num : (call.type == CHAR ? Character : Void);
        }
    case Instr:
        return getExpressionType(expr->firstChild, pt, funTable);
    case Or:
    case And:
    case Eq:
    case Order:
    case Addsub:
    case Divstar:
    case Num:
        return Num;
    case Character:
        return Character;
    case ListTypVar:
    case ListExp:
    case SuiteInstr:
    case Exp:
    case TB:
    case FB:
    case M:
    case E:
    case T:
    case F:
    case LValue:
    case ExclamationPoint:
        printf("J'étais pas sur de son type donc vérifie ce que c'est.\n");
        printTree(expr);
    default:
        return Void;
    }
}
