/* tree.h */

typedef enum {
    Prog, 
    DeclVars, 
    DeclFonct, 
    EnTeteFonct, 
    Instr, 
    Type, 
    Ident, 
    Void, 
    Var, 
    Funct, 
    Char, 
    Num, 
    Return, 
    Affect, 
    UnOperator, 
    BiOperator, 
    Negate, 
    Order, 
    Equal, 
    Or, 
    And, 
    While, 
    If,
    Else, 
    Cond, 
    Do,
    Struct, 
    DeclStruct, 
    Dot
} label_t;

typedef struct Node {
  label_t label;
  struct Node *firstChild, *nextSibling;
  int lineno;
  char * ident;
  int num;
  char tank;
} Node;

Node *makeNode(label_t label);
void addSibling(Node *node, Node *sibling);
void addChild(Node *parent, Node *child);
void deleteTree(Node*node);
void printTree(Node *node);

#define FIRSTCHILD(node) node->firstChild
#define SECONDCHILD(node) node->firstChild->nextSibling
#define THIRDCHILD(node) node->firstChild->nextSibling->nextSibling
