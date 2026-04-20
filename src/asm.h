#ifndef ASM
#define ASM

#include "tree.h"
#include "sym.h"
#include <stdio.h>

/**
 * @brief Point d'entrée pour la génération du code assembleur (NASM). 
 * Parcourt l'AST pour générer la section .bss, déclarer les fonctions externes, et lancer la traduction du main.
 * @param root Le nœud racine de l'arbre syntaxique abstrait.
 * @param file Le pointeur vers le fichier de sortie (.asm) ouvert en écriture.
 * @param globalSt La table des symboles globale (utilisée pour lister les variables globales).
 */
void parcours_depart(Node * root, FILE * file, SymbolTable * globalSt);

#endif