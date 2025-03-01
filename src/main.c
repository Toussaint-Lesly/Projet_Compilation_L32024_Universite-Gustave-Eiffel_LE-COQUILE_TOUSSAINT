/**
 * @file main.c
 * @author Marc LE COQUIL - Lesly Jumelle TOUSSAINT
 * @brief Main file of the project.
 * @date 2024-02-10
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/syscall.h>
#include <unistd.h>

#include "writter.h"
#include "utilitaries.h"
#include "tpcas.tab.h"

extern Node *root;

/**
 * @fn void printOptions(ProgTable t, int showAllTables, int showAllFunctions, char *functionToShow, int showGlobals, int printTreeOption)
 * @brief Print the options chose by the user.
 * 
 * @param t ProgTable Table to print.
 * @param showAllTables int Show all the tables.
 * @param showAllFunctions int Show all the functions.
 * @param functionToShow char* Function to show.
 * @param showGlobals int Show the globals.
 * @param printTreeOption int Print the tree.
 */
void printOptions(ProgTable t, int showAllTables, int showAllFunctions,
                  char *functionToShow, int showGlobals, int printTreeOption)
{
  if (printTreeOption)
  {
    printTree(root);
    fprintf(stdout, "\n");
  }

  printProgTable(t, showAllTables, showAllFunctions, functionToShow,
                 showGlobals);
}

/**
 * @fn int main(int argc, char *argv[])
 * @brief Main function of the project.
 * 
 * @param argc int Number of arguments.
 * @param argv char*[] Arguments.
 * @return int 
 */
int main(int argc, char *argv[])
{
  int printTreeOption = 0;
  int showAllFunctions = 0;
  int showAllTables = 0;
  int showGlobals = 0;
  char functionToShow[SIZE_ID] = {};
  char outputName[SIZE_ID] = "_anonymous.asm";
  int chosenOption =
      optionHandler(argc, argv, &showAllTables, &showAllFunctions,
                    functionToShow, &showGlobals, &printTreeOption, outputName);

  if (chosenOption)
    return chosenOption;

  if (yyparse())
    return 1;

  ProgTable t = createNewProgTable();
  ReturnInfo errorCode = fillProgTable(&t, root);
  if (errorCode != SUCCESS)
  {
    fprintf(stderr, "An error has occured during function's symbol tables loading. ");
    return getErrorCode(errorCode);
  }

  printOptions(t, showAllTables, showAllFunctions, functionToShow, showGlobals,
               printTreeOption);

  errorCode = writeAll(root, t, outputName);
  if (errorCode == SUCCESS)
    system("make assemble");
  else
    return getErrorCode(errorCode);

  freeProgTable(&t);

  deleteTree(root);
  return 0;
}
