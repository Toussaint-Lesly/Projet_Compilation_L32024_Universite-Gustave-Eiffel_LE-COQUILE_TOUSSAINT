/**
 * @file defaultFunctionWritter.c
 * @author Marc LE COQUIL - Lesly Jumelle TOUSSAINT
 * @brief Contain the functions to write the default input-output functions of every executable.
 * @date 2024-02-10
 *
 */

#include "utilitaries.h"
#include <stdio.h>

/**
 * @fn ReturnInfo writeGetCharAux(FILE *f)
 * @brief Write the function __getCharAux__ to the file.
 *
 * @param f The file to write to.
 * @return ReturnInfo The return info.
 */
ReturnInfo writeGetCharAux(FILE *f)
{
    fprintf(f, "__getCharAux__:\n"); // déclaration de la fonction
    fprintf(f, "\tpush rbp\n");
    fprintf(f, "\tmov rbp, rsp\n\n");

    fprintf(f, "\tsub rsp, 8\n");   // pour aligner la pile
    fprintf(f, "\tmov rax, 0\n");   // syscall pour lire
    fprintf(f, "\tmov rdi, 0\n");   // lire depuis stdin
    fprintf(f, "\tmov rsi, rsp\n"); // stocker le résultat dans la pile
    fprintf(f, "\tmov rdx, 1\n");   // lire un seul caractère
    fprintf(f, "\tsyscall\n");      // exécuter syscall
    fprintf(f, "\tmovzx rax, byte [rsp]\n\n");

    fprintf(f, "\tmov rsp, rbp\n");
    fprintf(f, "\tpop rbp\n");
    fprintf(f, "\tret\n\n");
    return SUCCESS;
}

/**
 * @fn ReturnInfo writeGetChar(FILE *f)
 * @brief Write the function getChar (that read a character and a '\\n') to the file.
 *
 * @param f The file to write to.
 * @return ReturnInfo The return info.
 */
ReturnInfo writeGetChar(FILE *f)
{
    writeGetCharAux(f);
    fprintf(f, "getChar:\n"); // déclaration de la fonction
    fprintf(f, "\tpush rbp\n");
    fprintf(f, "\tmov rbp, rsp\n\n");

    fprintf(f, "\tcall __getCharAux__\n");
    fprintf(f, "\tpush rax\n");
    fprintf(f, "\tcall __getCharAux__\n");
    fprintf(f, "\tpop rax\n");

    fprintf(f, "\tmov rsp, rbp\n");
    fprintf(f, "\tpop rbp\n");
    fprintf(f, "\tret\n\n");
    return SUCCESS;
}

/**
 * @fn ReturnInfo writeGetInt(FILE *f)
 * @brief Write the function getInt (that read an integer and a '\\n') to the file.
 *
 * @param f The file to write to.
 * @return ReturnInfo The return info.
 */
ReturnInfo writeGetInt(FILE *f)
{
    fprintf(f, "getInt:\n"); // déclaration de la fonction
    fprintf(f, "\tpush rbp\n");
    fprintf(f, "\tmov rbp, rsp\n\n");

    fprintf(f, "\txor r11, r11\n"); // Initialiser r11 à 0, servira pour la somme
    fprintf(f, "\txor rax, rax\n"); // Initialiser rax à 0
    fprintf(f, "\txor rdi, rdi\n"); // Initialiser rdi à 0 (signe)

    fprintf(f, "\tpush r11\n"); // On les push pour les récupérer après l'appel
    fprintf(f, "\tpush rdi\n\n");

    fprintf(f, "\tpush r15\n");
    fprintf(f, "\tmov r15, rsp\n");
    fprintf(f, "\tand rsp, -16\n");
    fprintf(f, "\tsub rsp, 8\n\n");
    fprintf(f, "\tcall __getCharAux__\n\n"); // Appeler getChar pour lire le premier caractère
    fprintf(f, "\tmov rsp, r15\n");
    fprintf(f, "\tpop r15\n\n");

    fprintf(f, "\tpop rdi\n"); // On les pop pour les récupérer après l'appel
    fprintf(f, "\tpop r11\n\n");

    fprintf(f, "\tcmp al, '-'\n");     // Vérifier si c'est un signe négatif
    fprintf(f, "\tje .is_negative\n"); // Sauter si négatif
    fprintf(f, "\tjmp .read_digit\n"); // Sinon, commencer à lire les chiffres
    fprintf(f, "\t\t.is_negative:\n");
    fprintf(f, "\tmov rdi, 1\n\n"); // Mettre rdi à 1 pour indiquer un nombre négatif

    fprintf(f, "\tpush r11\n"); // On les push pour les récupérer après l'appel
    fprintf(f, "\tpush rdi\n\n");

    fprintf(f, "\tpush r15\n");
    fprintf(f, "\tmov r15, rsp\n");
    fprintf(f, "\tand rsp, -16\n");
    fprintf(f, "\tsub rsp, 8\n\n");
    fprintf(f, "\tcall __getCharAux__\n\n"); // Appeler getChar pour lire le premier caractère
    fprintf(f, "\tmov rsp, r15\n");
    fprintf(f, "\tpop r15\n\n");

    fprintf(f, "\tpop rdi\n"); // On les pop pour les récupérer après l'appel
    fprintf(f, "\tpop r11\n\n");

    fprintf(f, "\t\t.read_digit:\n");
    fprintf(f, "\tcmp al, '0'\n");       // Vérifier si le caractère est un chiffre
    fprintf(f, "\tjl .done\n");          // Si moins que '0', finir
    fprintf(f, "\tcmp al, '9'\n");       // Vérifier si le caractère est un chiffre
    fprintf(f, "\tjg .done\n");          // Si plus que '9', finir
    fprintf(f, "\timul r11, r11, 10\n"); // Multiplier r11 par 10 (décaler à gauche)
    fprintf(f, "\tsub al, '0'\n");       // Convertir le caractère en chiffre
    fprintf(f, "\tadd r11, rax\n\n");    // Ajouter rax à la somme

    fprintf(f, "\tpush r11\n"); // On les push pour les récupérer après l'appel
    fprintf(f, "\tpush rdi\n\n");

    fprintf(f, "\tpush r15\n");
    fprintf(f, "\tmov r15, rsp\n");
    fprintf(f, "\tand rsp, -16\n");
    fprintf(f, "\tsub rsp, 8\n\n");
    fprintf(f, "\tcall __getCharAux__\n\n"); // Appeler getChar pour lire le premier caractère
    fprintf(f, "\tmov rsp, r15\n");
    fprintf(f, "\tpop r15\n\n");

    fprintf(f, "\tpop rdi\n"); // On les pop pour les récupérer après l'appel
    fprintf(f, "\tpop r11\n\n");

    fprintf(f, "\tjmp .read_digit\n"); // Boucler
    fprintf(f, "\t\t.done:\n");
    fprintf(f, "\tcmp rdi, 0\n"); // Vérifier si le nombre est négatif
    fprintf(f, "\tje .theEnd\n"); // Si zéro, sauter à la fin
    fprintf(f, "\tneg r11\n");    // Sinon, négatif le nombre dans r11
    fprintf(f, "\t\t.theEnd:\n");

    fprintf(f, "\tmov rax, r11\n\n"); // Mettre la somme dans rax pour future utilisation

    fprintf(f, "\tmov rsp, rbp\n");
    fprintf(f, "\tpop rbp\n");
    fprintf(f, "\tret\n\n");
    return SUCCESS;
}

/**
 * @fn ReturnInfo writePutChar(FILE *f)
 * @brief Write the function putChar (that write a character) to the file.
 *
 * @param f The file to write to.
 * @return ReturnInfo The return info.
 */
ReturnInfo writePutChar(FILE *f)
{
    fprintf(f, "putChar:\n"); // déclaration de la fonction
    fprintf(f, "\tpush rbp\n");
    fprintf(f, "\tmov rbp, rsp\n\n");

    fprintf(f, "\tpush rdi\n"); // on push le premier et seul argument

    fprintf(f, "\tmov rax, 1\n");   // syscall pour ecrire
    fprintf(f, "\tmov rdi, 1\n");   // ecrire dans stdout
    fprintf(f, "\tmov rsi, rsp\n"); // écrire le caractère supposé dans r11
    fprintf(f, "\tmov rdx, 1\n");   // ecrire un seul caractère
    fprintf(f, "\tsyscall\n\n");    // exécuter syscall

    fprintf(f, "\tmov rsp, rbp\n");
    fprintf(f, "\tpop rbp\n");
    fprintf(f, "\tret\n\n");
    return SUCCESS;
}

/**
 * @fn ReturnInfo writePutInt(FILE *f)
 * @brief Write the function putInt (that write an integer) to the file.
 *
 * @param f The file to write to.
 * @return ReturnInfo The return info.
 */
ReturnInfo writePutInt(FILE *f)
{
    fprintf(f, "putInt:\n"); // déclaration de la fonction
    fprintf(f, "\tpush rbp\n");
    fprintf(f, "\tmov rbp, rsp\n\n");

    fprintf(f, "\tcmp rdi, 0\n"); // On affiche un - et on inverse le nombre s'il est négatif
    fprintf(f, "\tjge .positive\n");
    fprintf(f, "\tpush rdi\n");
    fprintf(f, "\tmov rdi, '-'\n");
    fprintf(f, "\tcall putChar\n");
    fprintf(f, "\tpop rdi\n");
    fprintf(f, "\tneg rdi\n");
    fprintf(f, "\t\t.positive:\n\n");

    fprintf(f, "\tmov rax, rdi\n");  // On stocke notre entier de départ
    fprintf(f, "\txor r11, r11\n");  // On comptera nos chiffre avec r11
    fprintf(f, "\tmov r12, 10\n\n"); // On met notre diviseur à 10

    fprintf(f, "\t.trad_digit:\n");
    fprintf(f, "\tinc r11\n");
    fprintf(f, "\txor rdx, rdx\n");      // On met rdx à 0
    fprintf(f, "\tidiv r12\n");          // On divise rax par 10
    fprintf(f, "\tadd rdx, '0'\n");      // On ajoute le reste à '0' pour le convertir en char
    fprintf(f, "\tpush rdx\n");          // On push le reste dans la pile
    fprintf(f, "\tcmp rax, 0\n");        // On vérifie si rax est à 0
    fprintf(f, "\tjne .trad_digit\n\n"); // Si non, on recommence

    fprintf(f, "\tcmp r11, 0\n");           // On vérifie si r11 est à 0
    fprintf(f, "\tjle .end_write_digit\n"); // On affiche rien
    fprintf(f, "\t.write_digit:\n\n");      // On écris chaque chiffre dans le bon ordre
    fprintf(f, "\tpop rdi\n");
    fprintf(f, "\tpush r11\n");

    fprintf(f, "\tpush r15\n");
    fprintf(f, "\tmov r15, rsp\n");
    fprintf(f, "\tand rsp, -16\n");
    fprintf(f, "\tsub rsp, 8\n\n");
    fprintf(f, "\tcall putChar\n"); // On appelle putChar
    fprintf(f, "\tmov rsp, r15\n");
    fprintf(f, "\tpop r15\n\n");

    fprintf(f, "\tpop r11\n");
    fprintf(f, "\tdec r11\n");             // On décrémente r11
    fprintf(f, "\tcmp r11, 0\n");          // On vérifie si r11 est à 0
    fprintf(f, "\tjg .write_digit\n");     // On recommence
    fprintf(f, "\t.end_write_digit:\n\n"); // fin de boucle

    fprintf(f, "\tmov rsp, rbp\n");
    fprintf(f, "\tpop rbp\n");
    fprintf(f, "\tret\n\n");
    return SUCCESS;
}

/**
 * @fn ReturnInfo writeDefaultFunctions(FILE *f)
 * @brief Write the default input-output functions to the file.
 *
 * @param f The file to write to.
 * @return ReturnInfo The return info.
 */
ReturnInfo writeDefaultFunctions(FILE *f)
{
    writeGetChar(f);
    writeGetInt(f);
    writePutChar(f);
    writePutInt(f);
    return SUCCESS;
}
