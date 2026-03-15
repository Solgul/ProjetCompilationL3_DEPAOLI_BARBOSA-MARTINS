/* tree.c */
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include "tree.h"
extern int lineno;

static const char *StringFromLabel[] = {
    "Prog", 
    "DeclVars", 
    "DeclFonct", 
    "EnTeteFonct", 
    "Instr", 
    "Type", 
    "Ident", 
    "Void", 
    "Var", 
    "Funct", 
    "Char", 
    "Num", 
    "Return", 
    "Affect", 
    "UnOperator", 
    "BiOperator", 
    "Negate", 
    "Order", 
    "Equal", 
    "Or", 
    "And", 
    "While", 
    "If", 
    "Else", 
    "Cond", 
    "Do",
    "Struct", 
    "DeclStruct", 
    "Dot"
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
    static bool rightmost[128];
    static int depth = 0;
    for (int i = 1; i < depth; i++) {
        printf(rightmost[i] ? "    " : "\u2502   ");
    }
    if (depth > 0) {
        printf(rightmost[depth] ? "\u2514\u2500\u2500 " : "\u251c\u2500\u2500 ");
    }
    
    printf("%s", StringFromLabel[node->label]);
    if (node->label == Ident || node->label == Type || node->label == Affect || 
        node->label == BiOperator || node->label == Funct || node->label == Equal || 
        node->label == Order || node->label == Struct || node->label == DeclStruct) {
        if (node->ident != NULL) printf(": %s", node->ident);
    } else if (node->label == Num) {
        printf(": %d", node->num);
    }
    printf("\n");
    depth++;
    for (Node *child = node->firstChild; child != NULL; child = child->nextSibling) {
        rightmost[depth] = (child->nextSibling) ? false : true;
        printTree(child);
    }
    depth--;
}
