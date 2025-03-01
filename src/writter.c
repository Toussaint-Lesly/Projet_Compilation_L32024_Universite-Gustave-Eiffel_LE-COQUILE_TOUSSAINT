/**
 * @file writter.c
 * @author Marc LE COQUIL - Lesly Jumelle TOUSSAINT
 * @brief Writter and translater from tpc to nasm.
 * @date 2024-02-10
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "functionTable.h"
#include "writter.h"
#include "defaultFunctionWritter.h"

// Node used to compare the value of a variable with 0.
Node COMPARAISON_NODE = {Ident, NULL, NULL};
Node ZERO = {Num, NULL, &COMPARAISON_NODE, {.num = 0}};
Node IMPLCITE_IF_NODE = {Eq, &ZERO, NULL, {.ident = "!="}};
Node IMPLCITE_SUB_NODE = {Addsub, &ZERO, NULL, {.ident = "-"}};

// Registers used to pass the arguments of a function.
char *ARG_REGISTERS[6] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};

ReturnInfo writeInstr(Node *instr, FunctionInfo funTable);
ReturnInfo writeCall(Node *call, FunctionInfo funTable, FunctionInfo funCalled);
void writeAlignStackBeforeFunCall(FILE *f);
void writeAlignStackAfterFunCall(FILE *f);
ReturnInfo handleLocalGetIdent(Node *maybeLocal, FunctionInfo funTable);
ReturnInfo handleLocalPushIdent(Node *maybeLocal, FunctionInfo funTable);
ReturnInfo handleFunctionCall(Node *maybeCall, FunctionInfo funTable);
ReturnInfo writeElse(Node *elseInstr, FunctionInfo funTable, int curIfCount);

int ifCount = 0;
int whileCount = 0;
int conditionCount = 0;
int assignementCount = 0;
FILE *f;
ProgTable pt;

/**
 * @fn ReturnInfo quickVerif(Node *root, char *fileName)
 * @brief Quick verification of the program.
 *
 * @param root Node* Root of the tree.
 * @param fileName char* Name of the file.
 * @return ReturnInfo Eventual error code.
 */
ReturnInfo quickVerif(Node *root, char *fileName)
{
    if (!root || !fileName || pt.size <= 0)
        return FAILURE;

    FunctionInfo maybeMain = getFunctionsTable(pt, "main");
    if (strcmp(maybeMain.id, "main"))
    {
        fprintf(stderr, "No main function found in the program.\n");
        return NO_MAIN_FUNCTION;
    }

    return SUCCESS;
}

/* ----------------- Writing of basics value (int and char) ----------------- */
/**
 * @fn ReturnInfo writeNum(Node *num, FunctionInfo funTable)
 * @brief Write the translation of pushing a number on the stack.
 *
 * @param num Node* Number to push.
 * @param funTable FunctionInfo Function table we are in.
 * @return ReturnInfo Eventual error code.
 */
ReturnInfo writeNum(Node *num, FunctionInfo funTable)
{
    fprintf(f, "\tmov rax, %d\n", num->u.num);
    fprintf(f, "\tpush rax\n\n");
    return SUCCESS;
}

/**
 * @fn ReturnInfo writeCharacter(Node *chr, FunctionInfo funTable)
 * @brief Write the translation of pushing a character on the stack.
 *
 * @param chr Node* Character to push.
 * @param funTable FunctionInfo Function table we are in.
 * @return ReturnInfo Eventual error code.
 */
ReturnInfo writeCharacter(Node *chr, FunctionInfo funTable)
{
    fprintf(f, "\txor rax, rax\n");
    fprintf(f, "\tmov al, %d\n", charToAsciiCode(chr->u.character));
    fprintf(f, "\tpush rax\n\n");
    return SUCCESS;
}

/**
 * @fn ReturnInfo writeEventualIndex(Node *indexNode, FunctionInfo funTable, Symbol var)
 * @brief Write the translation of an eventual index.
 *
 * @param indexNode Node* Index to write.
 * @param funTable FunctionInfo Function table we are in.
 * @param var Symbol Variable wich the index refers to.
 * @return ReturnInfo Eventual error code.
 */
ReturnInfo writeEventualIndex(Node *indexNode, FunctionInfo funTable, Symbol var)
{
    if (!indexNode && !var.isArray)
    {
        fprintf(f, "\txor rax, rax\n");
        fprintf(f, "\tpush rax\n\n");
        return SUCCESS;
    }
    if (!indexNode && var.isArray)
    {
        fprintf(f, "\tmov rax, -1\n");
        fprintf(f, "\tpush rax\n\n");
        return SUCCESS;
    }
    if (getExpressionType(indexNode, pt, funTable) == Void)
    {
        fprintf(stderr, "Void-like expression used to describe an index at line %d.\n", indexNode->lineno);
        return VOID_INDEX;
    }
    return writeInstr(indexNode, funTable);
}

/**
 * @fn ReturnInfo writeGlobalVariablePushValue(int globalValueIndex)
 * @brief Write the translation of pushing a global variable on the stack.
 *
 * @param globalValueIndex int Index of the global variable in the program table.
 * @return ReturnInfo Eventual error code.
 */
ReturnInfo writeGlobalVariablePushValue(int globalValueIndex)
{
    if (pt.glob.symbols[globalValueIndex].isArray)
    {
        fprintf(f, "\tpop rbx\n");
        fprintf(f, "\tcmp rbx, -1\n");
        fprintf(f, "\tjne .not_address%d\n", assignementCount);
        fprintf(f, "\tlea rax, [%s]\n", pt.glob.symbols[globalValueIndex].id);
        fprintf(f, "\tjmp .end_assignement%d\n", assignementCount);
        fprintf(f, "\t\t.not_address%d:\n", assignementCount);
        fprintf(f, "\timul rbx, %d\n", pt.glob.symbols[globalValueIndex].type);
        fprintf(f, "\txor rax, rax\n");
        fprintf(f, "\tmovsx rax, %s [%s + rbx]\n",
                pt.glob.symbols[globalValueIndex].type == INT ? "dword" : "byte",
                pt.glob.symbols[globalValueIndex].id);
        fprintf(f, "\t\t.end_assignement%d:\n", assignementCount);
        assignementCount++;
    }
    else
    {
        fprintf(f, "\txor rax, rax\n");
        fprintf(f, "\tmovsx rax, %s [%s]\n",
                pt.glob.symbols[globalValueIndex].type == INT ? "dword" : "byte",
                pt.glob.symbols[globalValueIndex].id);
    }
    fprintf(f, "\tpush rax\n\n");

    return SUCCESS;
}

/**
 * @fn ReturnInfo writeGlobalVariableGetValue(int globalValueIndex)
 * @brief Write the translation of getting a global variable of the stack.
 *
 * @param globalValueIndex int Index of the global variable in the program table.
 * @return ReturnInfo Eventual error code.
 */
ReturnInfo writeGlobalVariableGetValue(int globalValueIndex)
{
    if (pt.glob.symbols[globalValueIndex].isArray)
    {
        fprintf(f, "\tpop rbx\n");
        fprintf(f, "\timul rbx, %d\n", pt.glob.symbols[globalValueIndex].type);
        fprintf(f, "\tpop rax\n");
        fprintf(f, "\tmov [%s + rbx], %s\n",
                pt.glob.symbols[globalValueIndex].id,
                pt.glob.symbols[globalValueIndex].type == INT ? "eax" : "al");
    }
    else
    {
        fprintf(f, "\tpop rax\n");
        fprintf(f, "\tmov %s [%s], %s\n",
                pt.glob.symbols[globalValueIndex].type == INT ? "dword" : "byte",
                pt.glob.symbols[globalValueIndex].id,
                pt.glob.symbols[globalValueIndex].type == INT ? "eax" : "al");
    }
    return SUCCESS;
}

/**
 * @fn ReturnInfo writeLocalVariablePushValue(FunctionInfo funTable, int localValueIndex)
 * @brief Write the translation of pushing a local variable on the stack.
 *
 * @param funTable FunctionInfo Function table we are in.
 * @param localValueIndex int Index of the local variable in the function table.
 * @return ReturnInfo Eventual error code.
 */
ReturnInfo writeLocalVariablePushValue(FunctionInfo funTable, int localValueIndex)
{
    if (funTable.locals.symbols[localValueIndex].isArray)
    {
        fprintf(f, "\tpop rbx\n");
        fprintf(f, "\tcmp rbx, -1\n");
        fprintf(f, "\tjne .not_address%d\n", assignementCount);
        fprintf(f, "\tlea rax, [rbp - %d]\n", funTable.locals.symbols[localValueIndex].address + funTable.locals.symbols[localValueIndex].type * funTable.locals.symbols[localValueIndex].numberOfValues);
        fprintf(f, "\tjmp .end_assignement%d\n", assignementCount);
        fprintf(f, "\t\t.not_address%d:\n", assignementCount);
        fprintf(f, "\timul rbx, %d\n", funTable.locals.symbols[localValueIndex].type);
        fprintf(f, "\txor rax, rax\n");
        fprintf(f, "\tmovsx rax, %s [rbp - %d + rbx]\n",
                funTable.locals.symbols[localValueIndex].type == INT ? "dword" : "byte",
                funTable.locals.symbols[localValueIndex].address + funTable.locals.symbols[localValueIndex].type * funTable.locals.symbols[localValueIndex].numberOfValues);
        fprintf(f, "\t\t.end_assignement%d:\n", assignementCount);
        assignementCount++;
    }
    else
    {
        fprintf(f, "\txor rax, rax\n");
        fprintf(f, "\tmovsx rax, %s [rbp - %d]\n",
                funTable.locals.symbols[localValueIndex].type == INT ? "dword" : "byte",
                funTable.locals.symbols[localValueIndex].address + funTable.locals.symbols[localValueIndex].type);
    }
    fprintf(f, "\tpush rax\n\n");
    return SUCCESS;
}

/**
 * @fn ReturnInfo writeLocalVariableGetValue(FunctionInfo funTable, int localValueIndex)
 * @brief Write the translation of getting a local variable of the stack.
 *
 * @param funTable FunctionInfo Function table we are in.
 * @param localValueIndex int Index of the local variable in the function table.
 * @return ReturnInfo Eventual error code.
 */
ReturnInfo writeLocalVariableGetValue(FunctionInfo funTable, int localValueIndex)
{
    if (funTable.locals.symbols[localValueIndex].isArray)
    {
        fprintf(f, "\tpop rbx\n");
        fprintf(f, "\timul rbx, %d\n", funTable.locals.symbols[localValueIndex].type);
        fprintf(f, "\tpop rax\n");
        fprintf(f, "\tmov [rbp - %d + rbx], %s\n",
                funTable.locals.symbols[localValueIndex].address + funTable.locals.symbols[localValueIndex].type * funTable.locals.symbols[localValueIndex].numberOfValues,
                funTable.locals.symbols[localValueIndex].type == INT ? "eax" : "al");
    }
    else
    {
        fprintf(f, "\tpop rax\n");
        fprintf(f, "\tmov [rbp - %d], %s\n",
                funTable.locals.symbols[localValueIndex].address + funTable.locals.symbols[localValueIndex].type,
                funTable.locals.symbols[localValueIndex].type == INT ? "eax" : "al");
    }

    return SUCCESS;
}

/**
 * @fn ReturnInfo writeArgVariablePushValue(FunctionInfo funTable, int argValueIndex)
 * @brief Write the translation of pushing a argument variable on the stack.
 *
 * @param funTable FunctionInfo Function table we are in.
 * @param argValueIndex int Index of the argument variable in the function table.
 * @return ReturnInfo Eventual error code.
 */
ReturnInfo writeArgVariablePushValue(FunctionInfo funTable, int argValueIndex)
{
    // TODO
    if (argValueIndex < 6)
    {
        if (funTable.args.symbols[argValueIndex].isAddress)
        {
            fprintf(f, "\tpop rbx\n");
            fprintf(f, "\tcmp rbx, -1\n");
            fprintf(f, "\tjne .not_address%d\n", assignementCount);
            fprintf(f, "\tmov rax, %s\n", ARG_REGISTERS[argValueIndex]);
            fprintf(f, "\tjmp .end_assignement%d\n", assignementCount);
            fprintf(f, "\t\t.not_address%d:\n", assignementCount);
            fprintf(f, "\timul rbx, %d\n", funTable.args.symbols[argValueIndex].type);
            fprintf(f, "\txor rax, rax\n");
            fprintf(f, "\tmovsx rax, %s [%s + rbx]\n",
                    funTable.args.symbols[argValueIndex].type == INT ? "dword" : "byte", ARG_REGISTERS[argValueIndex]);
            fprintf(f, "\t\t.end_assignement%d:\n", assignementCount);
            fprintf(f, "\tpush rax\n\n");
            assignementCount++;
        }
        else
            fprintf(f, "\tpush %s\n", ARG_REGISTERS[argValueIndex]);
    }
    else
        fprintf(f, "\tpush %s\n", ARG_REGISTERS[argValueIndex - 6]);
    return SUCCESS;
}

/**
 * @fn ReturnInfo writeArgVariableGetValue(FunctionInfo funTable, int argValueIndex)
 * @brief Write the translation of getting a argument variable of the stack.
 *
 * @param funTable FunctionInfo Function table we are in.
 * @param argValueIndex int Index of the argument variable in the function table.
 * @return ReturnInfo Eventual error code.
 */
ReturnInfo writeArgVariableGetValue(FunctionInfo funTable, int argValueIndex)
{
    // TODO
    if (argValueIndex < 6)
    {
        if (funTable.args.symbols[argValueIndex].isAddress)
        {
            fprintf(f, "\tpop rbx\n");
            fprintf(f, "\timul rbx, %d\n", funTable.args.symbols[argValueIndex].type);
            fprintf(f, "\tmov rax, %s\n", ARG_REGISTERS[argValueIndex]);
            fprintf(f, "\tadd rax, rbx\n");
            fprintf(f, "\tpop rbx\n");
            fprintf(f, "\tmov [rax], %s\n", funTable.args.symbols[argValueIndex].type == INT ? "ebx" : "bl");
        }
        else
        {
            fprintf(f, "\tpop rax\n");
            fprintf(f, "\tmov %s, rax\n", ARG_REGISTERS[argValueIndex]);
        }
    }
    else
        fprintf(f, "\tmov %s, rax\n", ARG_REGISTERS[argValueIndex - 6]);

    return SUCCESS;
}

/**
 * @fn ReturnInfo processInstructionBlock(Node *block, FunctionInfo funTable)
 * @brief Write the translation of every instruction in a block.
 *
 * @param block Node* Block of instructions to write.
 * @param funTable FunctionInfo Function table we are in.
 * @return ReturnInfo Eventual error code.
 */
ReturnInfo processInstructionBlock(Node *block, FunctionInfo funTable)
{
    ReturnInfo info;
    if (!block)
        return SUCCESS;
    do
    {
        info = writeInstr(block, funTable);
        if (info != SUCCESS)
            return info;

    } while ((block = block->nextSibling) != NULL);
    return SUCCESS;
}

/* ------- Identifier handlers -------- */
/**
 * @fn ReturnInfo writeGetIdent(Node *ident, FunctionInfo funTable)
 * @brief Write the translation of putting the value of an identifier from the stack.
 *
 * @param ident Node* Identifier to get.
 * @param funTable FunctionInfo Function table we are in.
 * @return ReturnInfo Eventual error code.
 */
ReturnInfo writeGetIdent(Node *ident, FunctionInfo funTable)
{
    ReturnInfo info = handleLocalGetIdent(ident, funTable);
    if (info != SUCCESS)
        return info;
    fprintf(f, "\n");
    return SUCCESS;
}

/**
 * @fn ReturnInfo writePushIdent(Node *ident, FunctionInfo funTable)
 * @brief Write the translation of pushing the value of an identifier on the stack.
 *
 * @param ident Node* Identifier to push.
 * @param funTable FunctionInfo Function table we are in.
 * @return ReturnInfo Eventual error code.
 */
ReturnInfo writePushIdent(Node *ident, FunctionInfo funTable)
{
    ReturnInfo info = (ident && ident->firstChild && ident->firstChild->label == Arguments) ? handleFunctionCall(ident, funTable) : handleLocalPushIdent(ident, funTable);
    if (info != SUCCESS)
        return info;
    fprintf(f, "\n");
    return SUCCESS;
}

/* ---------------------- Writing of basics operation ----------------------- */

/**
 * @fn ReturnInfo writeAddsub(Node *addsub, FunctionInfo funTable)
 * @brief Write the translation of an addition or subtraction.
 *
 * @param addsub Node* Addition or subtraction to write.
 * @param funTable FunctionInfo Function table we are in.
 * @return ReturnInfo Eventual error code.
 */
ReturnInfo writeAddsub(Node *addsub, FunctionInfo funTable)
{
    if (!addsub->firstChild->nextSibling && !strcmp(addsub->u.ident, "-"))
    {
        ZERO.nextSibling = addsub->firstChild;
        addsub = &IMPLCITE_SUB_NODE;
    }

    if (getExpressionType(addsub->firstChild, pt, funTable) == Void || getExpressionType(addsub->firstChild->nextSibling, pt, funTable) == Void)
    {

        fprintf(stderr, "Error: Addition or subtraction of a void-like expression at line %d.\n", addsub->lineno);
        return VOID_ADDSUB;
    }

    ReturnInfo info = writeInstr(addsub->firstChild, funTable);
    if (info != SUCCESS)
        return info;

    info = writeInstr(addsub->firstChild->nextSibling, funTable);
    if (info != SUCCESS)
        return info;

    fprintf(f, "\tpop rcx\n");
    fprintf(f, "\tpop rax\n");
    fprintf(f, "\t%s rax, rcx\n", !strcmp(addsub->u.ident, "+") ? "add" : "sub");
    fprintf(f, "\tpush rax\n\n");
    return SUCCESS;
}

/**
 * @fn ReturnInfo writeDivstar(Node *divsta, FunctionInfo funTable)
 * @brief Write the translation of a division or multiplication.
 *
 * @param divsta Node* Division or multiplication to write.
 * @param funTable FunctionInfo Function table we are in.
 * @return ReturnInfo Eventual error code.
 */
ReturnInfo writeDivstar(Node *divsta, FunctionInfo funTable)
{
    if (getExpressionType(divsta->firstChild, pt, funTable) == Void || getExpressionType(divsta->firstChild->nextSibling, pt, funTable) == Void)
    {
        fprintf(stderr, "Error: Division or multiplication of a void-like expression at line %d.\n", divsta->lineno);
        return VOID_DIVSTA;
    }

    ReturnInfo info = writeInstr(divsta->firstChild, funTable);
    if (info != SUCCESS)
        return info;

    info = writeInstr(divsta->firstChild->nextSibling, funTable);
    if (info != SUCCESS)
        return info;

    fprintf(f, "\tpop rcx\n");
    fprintf(f, "\tpop rax\n");
    if (!strcmp(divsta->u.ident, "/"))
    {
        fprintf(f, "\txor edx, edx;\n");
        fprintf(f, "\tidiv rcx\n");
    }
    else
        fprintf(f, "\timul rax, rcx\n");
    fprintf(f, "\tpush rax\n\n");
    return SUCCESS;
}

/**
 * @fn ReturnInfo writeEgual(Node *eg, FunctionInfo funTable)
 * @brief Write the translation of an assignment.
 *
 * @param eg Node* Assignment to write.
 * @param funTable FunctionInfo Function table we are in.
 * @return ReturnInfo Eventual error code.
 */
ReturnInfo writeEgual(Node *eg, FunctionInfo funTable)
{
    if (getExpressionType(eg->firstChild, pt, funTable) == Void || getExpressionType(eg->firstChild->nextSibling, pt, funTable) == Void)
    {
        fprintf(stderr, "Error: Assignation of a void-like expression at line %d.\n", eg->lineno);
        return VOID_ASSIGNMENT;
    }

    ReturnInfo info = writeInstr(eg->firstChild->nextSibling, funTable);
    if (info != SUCCESS)
        return info;

    info = writeGetIdent(eg->firstChild, funTable);
    if (info != SUCCESS)
        return info;

    if (getExpressionType(eg->firstChild->nextSibling, pt, funTable) == Num && getExpressionType(eg->firstChild, pt, funTable) == Character)
        fprintf(stderr, "Warning: Int passed as a character at line %d. May cause a problem if below 0 or above 256.\n", eg->lineno);

    return SUCCESS;
}

/**
 * @fn ReturnInfo writeReturn(Node *retInstr, FunctionInfo funTable)
 * @brief Write the translation of a return.
 *
 * @param retInstr Node* Return to write.
 * @param funTable FunctionInfo Function table we are in.
 * @return ReturnInfo Eventual error code.
 */
ReturnInfo writeReturn(Node *retInstr, FunctionInfo funTable)
{
    if (funTable.type == VOID_TYPE && retInstr->firstChild)
    {
        fprintf(stderr, "Error: No value should be returned in void typed function at line %d.\n", retInstr->lineno);
        return VOID_RETURN_ILLEGAL;
    }
    else if (funTable.type != VOID_TYPE && (!retInstr->firstChild || getExpressionType(retInstr->firstChild, pt, funTable) == Void))
    {
        fprintf(stderr, "Error: No value or void-like value returned at line %d while a %s is expected.\n", retInstr->lineno, funTable.type == INT ? "int" : "char");
        return MISSING_RETURN_VALUE;
    }
    else if (funTable.type == CHAR && getExpressionType(retInstr->firstChild, pt, funTable) == Num)
        fprintf(stderr, "Warning: Int returned while a character is expected at line %d. May cause a problem if below 0 or above 256.\n", retInstr->lineno);

    else if (funTable.type == VOID_TYPE && !retInstr->firstChild)
    {
        fprintf(f, "\tmov rax, 0\n");
        return SUCCESS;
    }
    ReturnInfo info = writeInstr(retInstr->firstChild, funTable);
    if (info != SUCCESS)
        return info;
    if (strcmp(funTable.id, "main"))
    {
        fprintf(f, "\tpop rax\n");
        fprintf(f, "\tmov rsp, rbp\n");
        fprintf(f, "\tpop rbp\n");
        fprintf(f, "\tret\n\n");
    }
    else
    {
        fprintf(f, "\tmov rdi, rax\n");
        fprintf(f, "\tmov rax, 60\n");
        fprintf(f, "\tsyscall\n\n");
    }
    return SUCCESS;
}

/* ------- Writing of boolean operation -------- */

/**
 * @fn ReturnInfo writeRightComp(Node *comp, FunctionInfo funTable)
 * @brief Write the translation of a comparison.
 *
 * @param comp Node* Comparison to write.
 * @param funTable FunctionInfo Function table we are in.
 * @return ReturnInfo Eventual error code.
 */
ReturnInfo writeRightComp(Node *comp, FunctionInfo funTable)
{
    switch (comp->label)
    {
    case Eq:
        fprintf(f, "\tcmp rax, rcx\n");
        fprintf(f, "\tj%s .true%d\n", strcmp(comp->u.ident, "==") ? "ne" : "e", conditionCount);
        fprintf(f, "\txor rax, rax\n");
        fprintf(f, "\tjmp .false%d\n", conditionCount);
        fprintf(f, "\t\t.true%d:\n", conditionCount);
        fprintf(f, "\tmov rax, 1\n");
        fprintf(f, "\t\t.false%d:\n", conditionCount);
        break;
        break;
    case Order:
        fprintf(f, "\tcmp rax, rcx\n");
        if (comp->u.ident[0] == '>')
            fprintf(f, "\tjg%c .true%d\n", comp->u.ident[1] == '=' ? 'e' : ' ', conditionCount);
        else
            fprintf(f, "\tjl%c .true%d\n", comp->u.ident[1] == '=' ? 'e' : ' ', conditionCount);

        fprintf(f, "\txor rax, rax\n");
        fprintf(f, "\tjmp .false%d\n", conditionCount);
        fprintf(f, "\t\t.true%d:\n", conditionCount);
        fprintf(f, "\tmov rax, 1\n");
        fprintf(f, "\t\t.false%d:\n", conditionCount);
        break;
    case And:
    case Or:
        fprintf(f, "\t%s rax, rcx\n", comp->label == And ? "and" : "or");
        fprintf(f, "\ttest rax, rax\n");
        fprintf(f, "\tsetnz al\n");
        break;
    default:
        fprintf(stderr, "Unkown boolean operation: %s at line: %d\n", StringFromLabel[comp->label], comp->lineno);
        return UNKOWN_BOOLEAN_OPERATION;
    }
    conditionCount++;
    return SUCCESS;
}

/**
 * @fn ReturnInfo writeBooleanComp(Node *comp, FunctionInfo funTable)
 * @brief Write the two members of the comparison before the boolean operation.
 *
 * @param comp Node* Comparison to write.
 * @param funTable FunctionInfo Function table we are in.
 * @return ReturnInfo Eventual error code.
 */
ReturnInfo writeBooleanComp(Node *comp, FunctionInfo funTable)
{
    label_t gauche = getExpressionType(comp->firstChild, pt, funTable);
    label_t droite = getExpressionType(comp->firstChild->nextSibling, pt, funTable);
    if (gauche == Void || droite == Void)
    {
        fprintf(stderr, "Error: Comparison of a void-like expression at line %d.\n", comp->lineno);
        return VOID_COMPARATION;
    }
    if (gauche == Address || droite == Address)
    {
        fprintf(stderr, "Error: Comparison with an address at line %d.\n", comp->lineno);
        return INVALID_ARGUMENT_TYPE;
    }

    ReturnInfo info = writeInstr(comp->firstChild, funTable);
    if (info != SUCCESS)
        return info;

    info = writeInstr(comp->firstChild->nextSibling, funTable);
    if (info != SUCCESS)
        return info;

    fprintf(f, "\tpop rcx\n");
    fprintf(f, "\tpop rax\n");
    info = writeRightComp(comp, funTable);
    if (info != SUCCESS)
        return info;
    fprintf(f, "\tpush rax\n\n");

    return SUCCESS;
}

/* ------- Writing of conditionnal structures -------- */

/**
 * @fn ReturnInfo evaluateCondition(Node *cond, FunctionInfo funTable)
 * @brief Write the translation of the evaluation of a condition.
 *
 * @param cond Node* Condition to write.
 * @param funTable FunctionInfo Function table we are in.
 * @return ReturnInfo Eventual error code.
 */
ReturnInfo evaluateCondition(Node *cond, FunctionInfo funTable)
{
    ReturnInfo info = writeInstr(cond, funTable);
    if (info != SUCCESS)
        return info;

    fprintf(f, "\tpop rax\n");
    fprintf(f, "\tcmp rax, 0\n");
    return SUCCESS;
}

/**
 * @fn void handleIfBranching(Node *maybeElse, int curIfCount)
 * @brief Write the translation of the branching of an if.
 *
 * @param maybeElse Node* Possible else of the if.
 * @param curIfCount int Current if count.
 */
void handleIfBranching(Node *maybeElse, int curIfCount)
{
    fprintf(f, "\tje .%s%d\n", maybeElse ? "else" : "endif", curIfCount);
    fprintf(f, "\n");
}

/**
 * @fn ReturnInfo processIfBody(Node *body, int curIfCount, FunctionInfo funTable)
 * @brief Write the translation of the body of an if.
 *
 * @param body Node* Body of the if.
 * @param curIfCount int Current if count.
 * @param funTable FunctionInfo Function table we are in.
 * @return ReturnInfo Eventual error code.
 */
ReturnInfo processIfBody(Node *body, int curIfCount, FunctionInfo funTable)
{
    ReturnInfo info;
    if (!body)
        return SUCCESS;
    do
    {
        if (body->label == Else)
        {
            fprintf(f, "\tjmp .endif%d\n\n", curIfCount);
            info = writeElse(body, funTable, curIfCount);
        }
        else
            info = writeInstr(body, funTable);
        if (info != SUCCESS)
            return info;

    } while ((body = body->nextSibling) != NULL);
    return SUCCESS;
}

/**
 * @fn ReturnInfo writeIf(Node *ifInstr, FunctionInfo funTable)
 * @brief Write the translation of an if.
 *
 * @param ifInstr Node* If to write.
 * @param funTable FunctionInfo Function table we are in.
 * @return ReturnInfo Eventual error code.
 */
ReturnInfo writeIf(Node *ifInstr, FunctionInfo funTable)
{
    ReturnInfo info;
    Node *cond = ifInstr->firstChild;
    Node *body = cond->nextSibling;

    Node *maybeElse = getChildLabeled(ifInstr, Else);
    int curIfCount = ifCount;
    ifCount++;

    info = evaluateCondition(cond, funTable);
    if (info != SUCCESS)
        return info;

    handleIfBranching(maybeElse, curIfCount);

    info = processIfBody(body, curIfCount, funTable);
    if (info != SUCCESS)
        return info;

    fprintf(f, "\t.endif%d:\n\n", curIfCount);
    return SUCCESS;
}

/**
 * @fn ReturnInfo writeElse(Node *elseInstr, FunctionInfo funTable, int curIfCount)
 * @brief Write the translation of an else.
 *
 * @param elseInstr Node* Else to write.
 * @param funTable FunctionInfo Function table we are in.
 * @param curIfCount int Current if count.
 * @return ReturnInfo Eventual error code.
 */
ReturnInfo writeElse(Node *elseInstr, FunctionInfo funTable, int curIfCount)
{
    Node *body = elseInstr->firstChild;
    fprintf(f, "\t.else%d:\n", curIfCount);

    return processInstructionBlock(body, funTable);
}

/**
 * @fn ReturnInfo writeWhile(Node *whileInstr, FunctionInfo funTable)
 * @brief Write the translation of a while loop.
 *
 * @param whileInstr Node* While to write.
 * @param funTable FunctionInfo Function table we are in.
 * @return ReturnInfo Eventual error code.
 */
ReturnInfo writeWhile(Node *whileInstr, FunctionInfo funTable)
{
    ReturnInfo info;
    Node *cond = whileInstr->firstChild;
    Node *body = cond->nextSibling;
    if (cond->label == Ident)
    {
        sprintf(COMPARAISON_NODE.u.ident, "%s", cond->u.ident);
        cond = &IMPLCITE_IF_NODE;
    }
    int curWhileCount = whileCount;
    whileCount++;

    fprintf(f, "\t.loop%d:\n", curWhileCount);

    info = evaluateCondition(cond, funTable);
    if (info != SUCCESS)
        return info;
    fprintf(f, "\tje .endloop%d\n\n", curWhileCount);

    info = processInstructionBlock(body, funTable);
    if (info != SUCCESS)
        return info;

    fprintf(f, "\tjmp .loop%d\n", curWhileCount);
    fprintf(f, "\t.endloop%d:\n\n", curWhileCount);
    return SUCCESS;
}

/* ------- Function calls handler -------- */

/**
 * @fn void pushArgs(FunctionInfo funTable)
 * @brief Write the translation of the push of the arguments of a function. Used to save them before the call of another function
 *
 * @param funTable FunctionInfo Function table we are in.
 */
void pushArgs(FunctionInfo funTable)
{
    for (int i = funTable.args.len - 1; i >= 0; i--)
        fprintf(f, "\tpush %s\n", ARG_REGISTERS[i]);
    fprintf(f, "\n");
}

/**
 * @fn void popArgs(FunctionInfo funTable)
 * @brief Write the translation of the pop of the arguments of a function. Used to restore the stack.
 *
 * @param funTable FunctionInfo Function table we are in.
 */
void popArgs(FunctionInfo funTable)
{
    for (int i = 0; i < funTable.args.len; i++)
        fprintf(f, "\tpop %s\n", ARG_REGISTERS[i]);
    fprintf(f, "\n");
}

/**
 * @fn void writeAlignStackBeforeFunCall(FILE *f)
 * @brief Write the translation of the alignment of the stack before a function call.
 *
 * @param f FILE* File we are writing to.
 */
void writeAlignStackBeforeFunCall(FILE *f)
{
    fprintf(f, "\tpush r15\n");
    fprintf(f, "\tmov r15, rsp\n");
    fprintf(f, "\tand rsp, -16\n");
    fprintf(f, "\tsub rsp, 8\n\n");
}

/**
 * @fn void writeAlignStackAfterFunCall(FILE *f)
 * @brief Write the translation of the alignment of the stack after a function call.
 *
 * @param f FILE* File we are writing to.
 */
void writeAlignStackAfterFunCall(FILE *f)
{
    fprintf(f, "\tmov rsp, r15\n");
    fprintf(f, "\tpop r15\n\n");
}

/**
 * @fn ReturnInfo checkArgummentType(Node *arg, FunctionInfo callingFun, int argIndex, FunctionInfo funCalled)
 * @brief Check if the type of the argument is correct.
 *
 * @param arg Node* Argument to check.
 * @param callingFun FunctionInfo Function we are calling.
 * @param argIndex int Index of the argument.
 * @param funCalled FunctionInfo Function we are calling.
 * @return ReturnInfo Eventual error code.
 */
ReturnInfo checkArgummentType(Node *arg, FunctionInfo callingFun, int argIndex, FunctionInfo funCalled)
{
    int callingFunctionsVariablesIndex;
    Symbol funCalledArg = funCalled.args.symbols[argIndex];
    if (isInTable(callingFun.args, arg->u.ident, &callingFunctionsVariablesIndex) == ID_IN_TABLE)
    {
        Symbol callingArg = callingFun.args.symbols[callingFunctionsVariablesIndex];
        if (callingArg.isAddress && funCalledArg.isAddress)
        {
            if (callingArg.type != funCalled.args.symbols[argIndex].type)
            {
                fprintf(stderr, "ERROR: %s[] expected, got %s[] at line %d\n", funCalledArg.type == INT ? "int" : "char", callingArg.type == INT ? "int" : "char", arg->lineno);
                return INVALID_ARGUMENT_TYPE;
            }
        }
        else if (callingArg.isAddress && !arg->firstChild && !funCalledArg.isAddress)
        {
            fprintf(stderr, "ERROR: %s[] expected, got %s at line %d\n", funCalledArg.type == INT ? "int" : "char", callingArg.type == INT ? "int" : "char", arg->lineno);
            return INVALID_ARGUMENT_TYPE;
        }
        else if (!callingArg.isAddress && funCalledArg.isAddress)
        {
            fprintf(stderr, "ERROR: %s expected, got %s[] at line %d\n", funCalledArg.type == INT ? "int" : "char", callingArg.type == INT ? "int" : "char", arg->lineno);
            return INVALID_ARGUMENT_TYPE;
        }
    }
    else
    {
        Symbol callingArg;
        if (isInTable(callingFun.locals, arg->u.ident, &callingFunctionsVariablesIndex) == ID_IN_TABLE)
            callingArg = callingFun.locals.symbols[callingFunctionsVariablesIndex];
        else if (isInTable(pt.glob, arg->u.ident, &callingFunctionsVariablesIndex) == ID_IN_TABLE)
            callingArg = pt.glob.symbols[callingFunctionsVariablesIndex];
        else
            return SUCCESS;

        if (callingArg.isArray && !arg->firstChild && !funCalledArg.isAddress)
        {
            fprintf(stderr, "ERROR: %s expected, got %s[] at line %d\n", funCalledArg.type == INT ? "int" : "char", callingArg.type == INT ? "int" : "char", arg->lineno);
            return INVALID_ARGUMENT_TYPE;
        }
        else if (callingArg.isArray && arg->firstChild && funCalledArg.isAddress)
        {
            fprintf(stderr, "ERROR: %s[] expected, got %s at line %d\n", funCalledArg.type == INT ? "int" : "char", callingArg.type == INT ? "int" : "char", arg->lineno);
            return INVALID_ARGUMENT_TYPE;
        }
        else if (callingArg.isArray && !arg->firstChild && funCalledArg.isAddress && callingArg.type != funCalledArg.type)
        {
            fprintf(stderr, "ERROR: %s[] expected, got %s[] at line %d\n", funCalledArg.type == INT ? "int" : "char", callingArg.type == INT ? "int" : "char", arg->lineno);
            return INVALID_ARGUMENT_TYPE;
        }
        else if (!callingArg.isArray && funCalledArg.isAddress)
        {
            fprintf(stderr, "ERROR: %s[] expected, got %s at line %d\n", funCalledArg.type == INT ? "int" : "char", callingArg.type == INT ? "int" : "char", arg->lineno);
            return INVALID_ARGUMENT_TYPE;
        }
    }
    return SUCCESS;
}

/**
 * @fn ReturnInfo writeArg(Node *arg, FunctionInfo funTable, int argIndex, FunctionInfo funCalled)
 * @brief Translate the placement of the argument in the right registers.
 *
 * @param arg Node* Argument list to write.
 * @param funTable FunctionInfo Function table we are in.
 * @param argIndex int Index of the argument.
 * @param funCalled FunctionInfo Function we are calling.
 * @return ReturnInfo Eventual error code.
 */
ReturnInfo writeArg(Node *arg, FunctionInfo funTable, int argIndex, FunctionInfo funCalled)
{
    if (!arg || arg->label == Void)
        return SUCCESS;

    if (getExpressionType(arg, pt, funTable) == Void)
    {
        fprintf(stderr, "Error: Void-like argument passed to a function at line %d.\n", arg->lineno);
        return VOID_ARGUMENT_PASSED;
    }

    ReturnInfo info = writeArg(arg->nextSibling, funTable, argIndex + 1, funCalled);
    if (info != SUCCESS)
        return info;

    if (arg->label == Ident || arg->label == Array || arg->label == Address)
    {
        info = checkArgummentType(arg, funTable, argIndex, funCalled);
        if (info != SUCCESS)
            return info;
    }
    else if (funCalled.args.symbols[argIndex].isAddress)
    {
        fprintf(stderr, "Error: Passing a %s as a %s[] at line %d.\n", funCalled.args.symbols[argIndex].type == INT ? "int" : "char", funCalled.args.symbols[argIndex].type == INT ? "int" : "char", arg->lineno);
        return ARRAY_EXPECTED;
    }

    if (getExpressionType(arg, pt, funTable) == Num && funCalled.args.symbols[argIndex].type == CHAR)
        fprintf(stderr, "Warning: Int passed as a character at line %d. May cause a problem if below 0 or above 256.\n", arg->lineno);

    info = writeInstr(arg, funTable);
    if (info != SUCCESS)
        return info;

    if (argIndex < 6)
    {
        fprintf(f, "\tpop rax\n");
        fprintf(f, "\tmov %s, rax\n\n", ARG_REGISTERS[argIndex]);
    }

    return SUCCESS;
}

/**
 * @fn ReturnInfo writeCall(Node *call, FunctionInfo funTable, FunctionInfo funCalled)
 * @brief Translate the placement of the call of a function.
 *
 * @param call Node* Call to translate.
 * @param funTable FunctionInfo Function table we are in.
 * @param funCalled FunctionInfo Function we are calling.
 * @return ReturnInfo Eventual error code.
 */
ReturnInfo writeCall(Node *call, FunctionInfo funTable, FunctionInfo funCalled)
{
    int nbArg = 1;
    Node *args = call->firstChild;
    Node *arg = args->firstChild;
    while ((arg = arg->nextSibling) != NULL)
        nbArg++;

    arg = args->firstChild;

    if (arg->label == Void)
        nbArg = 0;

    if (funCalled.args.len == 0 && arg->label != Void)
    {
        fprintf(stderr, "Trying to call a function with arguments but the function does not take any. Line %d\n", call->lineno);
        return TOO_MANY_ARGUMENT;
    }
    else if (nbArg != funCalled.args.len)
    {
        fprintf(stderr, "Trying to call a function with too %s arguments. Expected %d, got %d. Line %d\n", nbArg > funCalled.args.len ? "many" : "few", funCalled.args.len, nbArg, call->lineno);
        return nbArg > funCalled.args.len ? TOO_MANY_ARGUMENT : TOO_FEW_ARGUMENT;
    }

    pushArgs(funTable);
    ReturnInfo info = writeArg(arg, funTable, 0, funCalled);
    if (info != SUCCESS)
        return info;
    writeAlignStackBeforeFunCall(f);
    fprintf(f, "\tcall %s\n\n", call->u.ident);
    writeAlignStackAfterFunCall(f);
    popArgs(funTable);
    return SUCCESS;
}

/**
 * @fn ReturnInfo handleFunctionCall(Node *maybeCall, FunctionInfo funTable)
 * @brief Handle the call of a function (check it's existence etc)
 *
 * @param maybeCall Node* Call to handle.
 * @param funTable FunctionInfo Function table we are in.
 * @return ReturnInfo Eventual error code.
 */
ReturnInfo handleFunctionCall(Node *maybeCall, FunctionInfo funTable)
{
    FunctionInfo call = getFunctionsTable(pt, maybeCall->u.ident);

    if (!strcmp(call.id, "err"))
    {
        fprintf(stderr, "Unidentified function indetififer : %s. Line %d\n", maybeCall->u.ident, maybeCall->lineno);
        return NOT_A_FUNCTION;
    }
    ReturnInfo info = writeCall(maybeCall, funTable, call);
    if (info != SUCCESS)
        return info;

    if (call.type != VOID_TYPE)
        fprintf(f, "\tpush rax\n\n");
    return SUCCESS;
}

/**
 * @fn ReturnInfo handleGlobalGetIdent(Node *maybeGlobal, FunctionInfo funTable, int index)
 * @brief Handle the get the global variable's value from the stack.
 *
 * @param maybeGlobal Node* Global variable to handle.
 * @param funTable FunctionInfo Function table we are in.
 * @param index int Index of the global variable in the table.
 * @return ReturnInfo Eventual error code.
 */
ReturnInfo handleGlobalGetIdent(Node *maybeGlobal, FunctionInfo funTable, int index)
{
    ReturnInfo info = isInTable(pt.glob, maybeGlobal->u.ident, &index);
    if (info != ID_IN_TABLE)
    {
        fprintf(stderr, "Unidentified indetififer : %s. Line %d\n", maybeGlobal->u.ident, maybeGlobal->lineno);
        return info;
    }

    if (pt.glob.symbols[index].isArray)
    {
        info = writeEventualIndex(maybeGlobal->firstChild, funTable, pt.glob.symbols[index]);
        if (info != SUCCESS)
            return info;
    }
    else if (maybeGlobal->firstChild && pt.glob.symbols[index].isArray)
    {
        fprintf(stderr, "Array unexpected at line %d\n", maybeGlobal->lineno);
        return ARRAY_UNEXPECTED;
    }

    return writeGlobalVariableGetValue(index);
}

/**
 * @fn ReturnInfo handlerArgGetIdent(Node *maybeArg, FunctionInfo funTable)
 * @brief Handle the get the argument's value from the stack.
 *
 * @param maybeArg Node* Argument to handle.
 * @param funTable FunctionInfo Function table we are in.
 * @return ReturnInfo Eventual error code.
 */
ReturnInfo handlerArgGetIdent(Node *maybeArg, FunctionInfo funTable)
{
    int index;
    ReturnInfo info = isInTable(funTable.args, maybeArg->u.ident, &index);
    if (info != ID_IN_TABLE)
        return handleGlobalGetIdent(maybeArg, funTable, index);

    if (funTable.args.symbols[index].isAddress)
    {
        info = writeEventualIndex(maybeArg->firstChild, funTable, funTable.args.symbols[index]);
        if (info != SUCCESS)
            return info;
    }
    else if (maybeArg->firstChild)
    {
        fprintf(stderr, "Array unexpected at line %d\n", maybeArg->lineno);
        return ARRAY_UNEXPECTED;
    }
    return writeArgVariableGetValue(funTable, index);
}

/**
 * @fn ReturnInfo handleLocalGetIdent(Node *maybeLocal, FunctionInfo funTable)
 * @brief Handle the get the local variable's value from the stack.
 *
 * @param maybeLocal Node* Local variable to handle.
 * @param funTable FunctionInfo Function table we are in.
 * @return ReturnInfo Eventual error code.
 */
ReturnInfo handleLocalGetIdent(Node *maybeLocal, FunctionInfo funTable)
{
    int index;
    ReturnInfo info = isInTable(funTable.locals, maybeLocal->u.ident, &index);
    if (info != ID_IN_TABLE)
        return handlerArgGetIdent(maybeLocal, funTable);

    if (funTable.locals.symbols[index].isArray && maybeLocal->firstChild)
    {
        info = writeEventualIndex(maybeLocal->firstChild, funTable, funTable.locals.symbols[index]);
        if (info != SUCCESS)
            return info;
    }
    else if (maybeLocal->firstChild)
    {
        fprintf(stderr, "Array unexpected at line %d\n", maybeLocal->lineno);
        return ARRAY_UNEXPECTED;
    }
    return writeLocalVariableGetValue(funTable, index);
}

/**
 * @fn ReturnInfo handleGlobalPushIdent(Node *maybeGlobal, FunctionInfo funTable, int index)
 * @brief Handle the push of a global variable.
 *
 * @param maybeGlobal Node* Global variable to handle.
 * @param funTable FunctionInfo Function table we are in.
 * @param index int Index of the global variable in the table.
 * @return ReturnInfo Eventual error code.
 */
ReturnInfo handleGlobalPushIdent(Node *maybeGlobal, FunctionInfo funTable, int index)
{
    ReturnInfo info = isInTable(pt.glob, maybeGlobal->u.ident, &index);

    if (info != ID_IN_TABLE)
    {
        fprintf(stderr, "Unidentified indetififer : %s. Line %d\n", maybeGlobal->u.ident, maybeGlobal->lineno);
        return info;
    }

    if (pt.glob.symbols[index].isArray && maybeGlobal->firstChild)
    {
        info = writeEventualIndex(maybeGlobal->firstChild, funTable, pt.glob.symbols[index]);
        if (info != SUCCESS)
            return info;
    }
    else if (maybeGlobal->firstChild)
    {
        fprintf(stderr, "Array unexpected at line %d\n", maybeGlobal->lineno);
        return ARRAY_UNEXPECTED;
    }
    return writeGlobalVariablePushValue(index);
}

/**
 * @fn ReturnInfo handlerArgPushIdent(Node *maybeArg, FunctionInfo funTable)
 * @brief Handle the push of an argument.
 *
 * @param maybeArg Node* Argument to handle.
 * @param funTable FunctionInfo Function table we are in.
 * @return ReturnInfo Eventual error code.
 */
ReturnInfo handlerArgPushIdent(Node *maybeArg, FunctionInfo funTable)
{
    int index;
    ReturnInfo info = isInTable(funTable.args, maybeArg->u.ident, &index);

    if (info != ID_IN_TABLE)
        return handleGlobalPushIdent(maybeArg, funTable, index);

    if (funTable.args.symbols[index].isAddress && maybeArg->firstChild)
    {
        info = writeEventualIndex(maybeArg->firstChild, funTable, funTable.args.symbols[index]);
        if (info != SUCCESS)
            return info;
    }
    else if (maybeArg->firstChild)
    {
        fprintf(stderr, "Array unexpected at line %d\n", maybeArg->lineno);
        return ARRAY_UNEXPECTED;
    }
    return writeArgVariablePushValue(funTable, index);
}

/**
 * @fn ReturnInfo handleLocalPushIdent(Node *maybeLocal, FunctionInfo funTable)
 * @brief Handle the push of a local variable.
 *
 * @param maybeLocal Node* Local variable to handle.
 * @param funTable FunctionInfo Function table we are in.
 * @return ReturnInfo Eventual error code.
 */
ReturnInfo handleLocalPushIdent(Node *maybeLocal, FunctionInfo funTable)
{
    int index;
    ReturnInfo info = isInTable(funTable.locals, maybeLocal->u.ident, &index);

    if (info != ID_IN_TABLE)
        return handlerArgPushIdent(maybeLocal, funTable);

    if (funTable.locals.symbols[index].isArray && maybeLocal->firstChild)
    {
        info = writeEventualIndex(maybeLocal->firstChild, funTable, funTable.locals.symbols[index]);
        if (info != SUCCESS)
            return info;
    }
    else if (maybeLocal->firstChild)
    {
        fprintf(stderr, "Array unexpected at line %d\n", maybeLocal->lineno);
        return ARRAY_UNEXPECTED;
    }
    return writeLocalVariablePushValue(funTable, index);
}

/**
 * @fn ReturnInfo writeDeclVarsLocale(Node *decl, FunctionInfo funTable)
 * @brief Write the translation of the declaration of the local variables.
 *
 * @param decl Node* Declaration to write.
 * @param funTable FunctionInfo Function table we are in.
 * @return ReturnInfo Eventual error code.
 */
ReturnInfo writeDeclVarsLocale(Node *decl, FunctionInfo funTable)
{
    fprintf(f, "\tsub rsp, %d\n", funTable.locals.size);
    fprintf(f, "\n");

    return SUCCESS;
}

/**
 * @fn ReturnInfo writeInstr(Node *instr, FunctionInfo funTable)
 * @brief Write the translation of any instruction (switch to the right function).
 *
 * @param instr Node* Instruction to write.
 * @param funTable FunctionInfo Function table we are in.
 * @return ReturnInfo Eventual error code.
 */
ReturnInfo writeInstr(Node *instr, FunctionInfo funTable)
{
    switch (instr->label)
    {
    case DeclVarsLocale:
        return writeDeclVarsLocale(instr, funTable);
        break;
    case If:
        return writeIf(instr, funTable);
        break;
    case Array:
    case Ident:
        return writePushIdent(instr, funTable);
        break;
    case Else:
        return writeElse(instr, funTable, ifCount);
        break;
    case While:
        return writeWhile(instr, funTable);
        break;
    case Return:
        return writeReturn(instr, funTable);
        break;
    case Or:
    case And:
    case Eq:
    case Order:
        return writeBooleanComp(instr, funTable);
        break;
    case Addsub:
        return writeAddsub(instr, funTable);
        break;
    case Divstar:
        return writeDivstar(instr, funTable);
        break;
    case Num:
        return writeNum(instr, funTable);
        break;
    case Character:
        return writeCharacter(instr, funTable);
        break;
    case Egual:
        return writeEgual(instr, funTable);
        break;
    default:
        fprintf(stderr, "huh ?\t%s\t%s\n\n", StringFromLabel[instr->label], instr->u.ident);
        break;
    }

    return FAILURE;
}

/**
 * @fn ReturnInfo writeBody(Node *body, FunctionInfo funTable)
 * @brief Write the translation of the body of a function.
 *
 * @param body Node* Body to write.
 * @param funTable FunctionInfo Function table we are in.
 * @return ReturnInfo Eventual error code.
 */
ReturnInfo writeBody(Node *body, FunctionInfo funTable)
{
    if (!body)
        return SUCCESS;
    return processInstructionBlock(body->firstChild, funTable);
}

/**
 * @fn ReturnInfo writeMain(Node *mainFun, FunctionInfo funTable)
 * @brief Write the translation of the main function.
 *
 * @param mainFun Node* Main function to write.
 * @param funTable FunctionInfo Function table we are in.
 * @return ReturnInfo Eventual error code.
 */
ReturnInfo writeMain(Node *mainFun, FunctionInfo funTable)
{
    fprintf(f, "_start:\n\tmov rbp, rsp\n");
    ReturnInfo info = writeBody(getChildLabeled(mainFun, Body), funTable);
    if (info != SUCCESS)
        return info;

    if (!getChildLabeled(getChildLabeled(mainFun, Body), Return))
    {
        fprintf(f, "\tmov rax, 60\n");
        fprintf(f, "\tmov rdi, 0\n");
        fprintf(f, "\tsyscall\n\n");
    }
    return SUCCESS;
}

/**
 * @fn ReturnInfo writeFunction(Node *fun, FunctionInfo funTable)
 * @brief Write the translation of a function.
 *
 * @param fun Node* Function to write.
 * @param funTable FunctionInfo Function table we are in.
 * @return ReturnInfo Eventual error code.
 */
ReturnInfo writeFunction(Node *fun, FunctionInfo funTable)
{
    ReturnInfo info;
    fprintf(f, "%s:\n", funTable.id);
    fprintf(f, "\tpush rbp\n");
    fprintf(f, "\tmov rbp, rsp\n\n");

    info = writeBody(getChildLabeled(fun, Body), funTable);
    if (info != SUCCESS)
        return info;

    if (!getChildLabeled(getChildLabeled(fun, Body), Return))
    {
        fprintf(f, "\tmov rsp, rbp\n");
        fprintf(f, "\tpop rbp\n");
        fprintf(f, "\tret\n\n");
    }

    return SUCCESS;
}

/**
 * @fn ReturnInfo writeGlobals()
 * @brief Write the translation of the global variables.
 *
 * @return ReturnInfo Eventual error code.
 */
ReturnInfo writeGlobals()
{
    fprintf(f, "section .bss\n");
    for (int i = 0; i < pt.glob.len; i++)
        fprintf(f, "\t%s: %s %d\n", pt.glob.symbols[i].id, sizeToAsm(pt.glob.symbols[i].type), pt.glob.symbols[i].numberOfValues);
    fprintf(f, "\n");

    return SUCCESS;
}

/**
 * @fn ReturnInfo writeProg(Node *prog)
 * @brief Write the translation of the whole program.
 *
 * @param prog Node* Program to write.
 * @return ReturnInfo Eventual error code.
 */
ReturnInfo writeProg(Node *prog)
{
    ReturnInfo info = writeGlobals();
    if (info != SUCCESS)
        return info;

    fprintf(f, "\n");

    fprintf(f, "global _start\nsection .text\n\n");
    writeDefaultFunctions(f);

    Node *fun = prog->firstChild;
    do
    {
        if (fun->label == DeclVarsGlobale)
            continue;

        char id[SIZE_ID];
        getFunId(fun, id);

        FunctionInfo funTable = getFunctionsTable(pt, id);
        if (!strcmp(id, "main"))
            info = writeMain(fun, funTable);
        else if (strcmp(id, "err"))
            info = writeFunction(fun, funTable);
        else
            printTree(fun);

        if (info != SUCCESS)
            return info;

        fprintf(f, "\n");
    } while ((fun = fun->nextSibling) != NULL);
    return SUCCESS;
}

/**
 * @fn ReturnInfo writeAll(Node *root, ProgTable progt, char *fileName)
 * @brief Write the translation of the whole program after checking quick verifications.
 *
 * @param root Node* Root of the program to write.
 * @param progt ProgTable Program table we are in.
 * @param fileName char* Name of the file to write.
 * @return ReturnInfo Eventual error code.
 */
ReturnInfo writeAll(Node *root, ProgTable progt, char *fileName)
{
    pt = progt;
    ReturnInfo verif = quickVerif(root, fileName);
    if (verif != SUCCESS)
        return verif;

    f = fopen(fileName, "w");

    if (!f)
        return COULD_NOT_OPEN_FILE;

    verif = writeProg(root);
    if (verif != SUCCESS)
        return verif;

    fclose(f);
    return SUCCESS;
}

/*

;-----------------------------------------
    push r11
    push rax
    push rdi

    call show_registers

    pop rdi
    pop rax
    pop r11
;-----------------------------------------

*/

/*

TODO :
1. Gérer les listes d'argument de plus de 6.
2. Gérer les return de la main

TODO (but in the end)
1. Gérer les valeurs de retour qui doivent être obligatoirement présente pour les non-void fonctions (optionnel)

*/