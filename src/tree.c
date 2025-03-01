/**
 * @file tree.c
 * @author Marc LE COQUIL - Lesly Jumelle TOUSSAINT
 * @brief Functions dedicated to the tree.
 * @date 2024-02-10
 */

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include "tree.h"

Node *root;

const char *StringFromLabel[] = {
    "Prog",
    "DeclVarsGlobale",
    "DeclVarsLocale",
    "Declarateurs",
    "InitVarsLocale",
    "DeclFoncts",
    "DeclFonct",
    "FuncHead",
    "ParamList",
    "ListTypVar",
    "Corps",
    "SuiteInstr",
    "Instr",
    "Exp",
    "TB",
    "FB",
    "M",
    "E",
    "T",
    "F",
    "LValue",
    "Arguments",
    "ListExp",
    "Type",
    "Ident",
    "Array",
    "Address",
    "Void",
    "If",
    "Else",
    "While",
    "Return",
    "Or",
    "And",
    "Eq",
    "Order",
    "Addsub",
    "Divstar",
    "Num",
    "Character",
    "=",
    "!"
    /* list all other node labels, if any */
    /* The list must coincide with the label_t enum in tree.h */
    /* To avoid listing them twice, see https://stackoverflow.com/a/10966395 */
};

/**
 * @fn Node* makeNode(label_t label)
 * @brief Create a new node.
 * 
 * @param label label_t Label of the node.
 * @return Node* New node.
 */
Node *makeNode(label_t label)
{
    Node *node = malloc(sizeof(Node));
    if (!node)
    {
        fprintf(stderr, "Ran out of memory\n");
        exit(2);
    }
    node->label = label;
    node->firstChild = node->nextSibling = NULL;
    node->lineno = lineno;
    return node;
}

/**
 * @fn void addSibling(Node *node, Node *sibling)
 * @brief Add a sibling to a node.
 * 
 * @param node Node* Node to add the sibling on.
 * @param sibling Node* Sibling to add.
 */
void addSibling(Node *node, Node *sibling)
{
    Node *curr = node;
    while (curr->nextSibling != NULL)
        curr = curr->nextSibling;
    curr->nextSibling = sibling;
}

/**
 * @fn void addChild(Node *parent, Node *child)
 * @brief Add a child to a node.
 * 
 * @param parent Node* Parent to add the child on.
 * @param child Node* Child to add.
 */
void addChild(Node *parent, Node *child)
{
    if (parent->firstChild == NULL)
        parent->firstChild = child;
    else
        addSibling(parent->firstChild, child);
}

/**
 * @fn void deleteTree(Node *node)
 * @brief Delete a tree.
 * 
 * @param node Node* Node to delete.
 */
void deleteTree(Node *node)
{
    if (!node)
        return;
    if (node->firstChild)
        deleteTree(node->firstChild);
    if (node->nextSibling)
        deleteTree(node->nextSibling);
    free(node);
}

/**
 * @fn void printTree(Node *node)
 * @brief Print a tree.
 * 
 * @param node Node* Node to print.
 */
void printTree(Node *node)
{
    if (!node)
        return;
    static bool rightmost[128];     // tells if node is rightmost sibling
    static int depth = 0;           // depth of current node
    for (int i = 1; i < depth; i++) // 2502 = vertical line
        printf(rightmost[i] ? "    " : "\u2502   ");

    if (depth > 0) // 2514 = L form; 2500 = horizontal line; 251c = vertical line and right horiz
        printf(rightmost[depth] ? "\u2514\u2500\u2500 " : "\u251c\u2500\u2500 ");

    printf("%s", StringFromLabel[node->label]);
    switch (node->label)
    {
    case Type:
        printf("\033[31m(%s)\033[0m", node->u.ident);
        break;
    case Ident:
        printf("\033[31m(%s)\033[0m", node->u.ident);
        break;
    case Array: printf("\033[90m(%s)\033[0m", node->u.ident); /*pour differencier un tableau*/
        break;
    case Eq:
        printf("\033[32m(%s)\033[0m", node->u.comp);
        break;
    case Order:
        printf("\033[32m(%s)\033[0m", node->u.comp);
        break;
    case Addsub:
        printf("\033[34m(%c)\033[0m", node->u.byte);
        break;
    case Divstar:
        printf("\033[34m(%c)\033[0m", node->u.byte);
        break;
    case Num:
        printf("\033[95m(%d)\033[0m", node->u.num);
        break;
    case Character:
        printf("\033[96m(%s)\033[0m", node->u.character);
        break;
    default:
        break;
    }
    printf("\n");
    depth++;
    for (Node *child = node->firstChild; child != NULL; child = child->nextSibling)
    {
        rightmost[depth] = (child->nextSibling) ? false : true;
        printTree(child);
    }
    depth--;
}
