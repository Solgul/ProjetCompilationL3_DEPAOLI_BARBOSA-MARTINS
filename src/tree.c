/* tree.c */
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include "tree.h"
extern int lineno;       /* from lexer */

static const char *StringFromLabel[] = {
  "Prog",
  "GlobalDecl",
  "GlobalDecls",
  "DeclStruct",
  "DeclStart",
  "DeclStarts",
  "Type",
  "DeclVars",
  "Declarateurs",
  "DeclFoncts",
  "DeclFonct",
  "EnTeteFonct",
  "Parametres",
  "ListTypVar",
  "Corps",
  "SuiteInstr",
  "IdentAffect",
  "Instr",
  "Exp",
  "TB",
  "FB",
  "M",
  "E",
  "T",
  "F",
  "Arguments",
  "ListExp",
  "TR_IDENT",
  "TR_STRUCT",
  "TR_VOID",
  "TR_TYPE",
  "TR_EQ",
  "TR_IF",
  "TR_ELSE",
  "TR_RETURN",
  "TR_OR",
  "TR_AND",
  "TR_ORDER",
  "TR_ADDSUB",
  "TR_DIVSTAR",
  "TR_NUM",
  "TR_CHARACTER",
  "TR_DOT",
  "TR_WHILE",
  "TR_NOT"
  /* list all other node labels, if any */
  /* The list must coincide with the label_t enum in tree.h */
  /* To avoid listing them twice, see https://stackoverflow.com/a/10966395 */
};

Node *makeNode(label_t label) {
  Node *node = malloc(sizeof(Node));
  if (!node) {
    printf("Run out of memory\n");
    exit(1);
  }
  node->label = label;
  node-> firstChild = node->nextSibling = NULL;
  node->lineno=lineno;
  node->ident = "\0";
  node->num = 0;
  node->tank = '\0'; 
  return node;
}

void addSibling(Node *node, Node *sibling) {
  Node *curr = node;
  while (curr->nextSibling != NULL) {
    curr = curr->nextSibling;
  }
  curr->nextSibling = sibling;
}

void addChild(Node *parent, Node *child) {
  if (parent == NULL || child == NULL) return;
  if (parent->firstChild == NULL) {
    parent->firstChild = child;
  }
  else {
    addSibling(parent->firstChild, child);
  }
}

void deleteTree(Node *node) {
  if (node->firstChild) {
    deleteTree(node->firstChild);
  }
  if (node->nextSibling) {
    deleteTree(node->nextSibling);
  }
  free(node);
}

void printTree(Node *node) {
  static bool rightmost[128]; // tells if node is rightmost sibling
  static int depth = 0;       // depth of current node
  for (int i = 1; i < depth; i++) { // 2502 = vertical line
    printf(rightmost[i] ? "    " : "\u2502   ");
  }
  if (depth > 0) { // 2514 = L form; 2500 = horizontal line; 251c = vertical line and right horiz 
    printf(rightmost[depth] ? "\u2514\u2500\u2500 " : "\u251c\u2500\u2500 ");
  }
  printf("%s", StringFromLabel[node->label]);
  if (node->ident[0] != '\0') {
      printf(": [%s]", node->ident); 
  }else if (node->num!= 0) {
      printf(": [%d]", node->num); 
  }else if (node->tank!= '\0') {
      printf(": [%c]", node->tank); 
  } else if (node->label == TR_IDENT || node->label == TR_TYPE || node->label == TR_STRUCT) {
      printf(": missing"); 
  }
  printf("\n");
  depth++;
  for (Node *child = node->firstChild; child != NULL; child = child->nextSibling) {
    rightmost[depth] = (child->nextSibling) ? false : true;
    printTree(child);
  }
  depth--;
}
