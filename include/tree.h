/* 	Author : LE COQUIL - TOUSSAINT */
/* 	tree.h */

#ifndef __TREE__
#define __TREE__

#define SIZE_ID 64

typedef enum
{
    Prog,
    DeclVarsGlobale,
    DeclVarsLocale,
    Declarateurs,
    InitVarsLocale,
    DeclFoncts,
    DeclFonct,
    FuncHead,
    ParamList,
    ListTypVar,
    Body,
    SuiteInstr,
    Instr,
    Exp,
    TB,
    FB,
    M,
    E,
    T,
    F,
    LValue,
    Arguments,
    ListExp,
    Type,
    Ident,
    Array,
    Address,
    Void,
    If,
    Else,
    While,
    Return,
    Or,
    And,
    Eq,
    Order,
    Addsub,
    Divstar,
    Num,
    Character,
    Egual,
    ExclamationPoint
    /* list all other node labels, if any */
    /* The list must coincide with the string array in tree.c */
    /* To avoid listing them twice, see https://stackoverflow.com/a/10966395 */
} label_t;

typedef struct Node
{
    label_t label;
    struct Node *firstChild, *nextSibling;
    union
    {
        char byte;
        int num;
        char ident[SIZE_ID];
        char comp[3];
        char character[3];
    } u;
    int lineno;
} Node;

extern const char *StringFromLabel[];
extern Node *root;
extern int lineno;

Node *makeNode(label_t label);
void addSibling(Node *node, Node *sibling);
void addChild(Node *parent, Node *child);
void deleteTree(Node *node);
void printTree(Node *node);

#define FIRSTCHILD(node) node->firstChild
#define SECONDCHILD(node) node->firstChild->nextSibling
#define THIRDCHILD(node) node->firstChild->nextSibling->nextSibling

#endif
