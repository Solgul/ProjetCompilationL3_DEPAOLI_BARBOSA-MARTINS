#ifndef ASM
#define ASM

#include "tree.h"
#include "sym.h"
#include <stdio.h>

/**
 * @brief 
 * 
 * @param root 
 * @param file
 */
void parcours_depart(Node * root, FILE * file, SymbolTable * globalSt);

#endif