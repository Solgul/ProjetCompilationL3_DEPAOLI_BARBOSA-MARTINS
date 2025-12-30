/* tree.h */

typedef enum {
  Prog,
  DeclStruct,
  DeclStart,
  DeclStarts,
  Type,
  DeclVars,
  Declarateurs,
  DeclFoncts,
  DeclFonct,
  EnTeteFonct,
  Parametres,
  ListTypVar,
  Corps,
  SuiteInstr,
  IdentAffect,
  Instr,
  Exp,
  TB,
  FB,
  M,
  E,
  T,
  F,
  Arguments,
  ListExp,
  TR_IDENT,
  TR_STRUCT,
  TR_VOID,
  TR_TYPE,
  TR_EQ,
  TR_IF,
  TR_ELSE,
  TR_RETURN,
  TR_OR,
  TR_AND,
  TR_ORDER,
  TR_ADDSUB,
  TR_DIVSTAR,
  TR_NUM,
  TR_CHARACTER,
  TR_DOT,
  TR_WHILE,
  TR_NOT
  /* list all other node labels, if any */
  /* The list must coincide with the string array in tree.c */
  /* To avoid listing them twice, see https://stackoverflow.com/a/10966395 */
} label_t;

typedef struct Node {
  label_t label;
  struct Node *firstChild, *nextSibling;
  int lineno;
} Node;

Node *makeNode(label_t label);
void addSibling(Node *node, Node *sibling);
void addChild(Node *parent, Node *child);
void deleteTree(Node*node);
void printTree(Node *node);

#define FIRSTCHILD(node) node->firstChild
#define SECONDCHILD(node) node->firstChild->nextSibling
#define THIRDCHILD(node) node->firstChild->nextSibling->nextSibling
