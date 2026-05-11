#ifndef TREE_H
#define TREE_H

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
    Addsub, 
    Divstar,
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

/**
 * @brief Alloue et initialise un nouveau noeud pour l'arbre.
 * 
 * @param label L'étiquette (de type label_t) définissant le type du noeud (Prog, DeclVars, Ident, etc.).
 * @return Node* Un pointeur vers le noeud nouvellement créé.
 */
Node *makeNode(label_t label);

/**
 * @brief Ajoute un noeud frère à la fin de la liste des frères du noeud courant.
 * 
 * @param node Le noeud de départ.
 * @param sibling Le noeud frère à ajouter.
 */
void addSibling(Node *node, Node *sibling);

/**
 * @brief Ajoute un noeud enfant à un noeud parent.
 * (Si le parent a déjà des enfants, le nouvel enfant est ajouté comme frère du dernier enfant.)
 * 
 * @param parent Le noeud parent auquel ajouter l'enfant.
 * @param child Le noeud enfant à ajouter.
 */
void addChild(Node *parent, Node *child);

/**
 * @brief Libère récursivement la mémoire allouée pour un arbre ou un sous-arbre.
 * @param node Le noeud racine de l'arbre à supprimer.
 */
void deleteTree(Node*node);

/**
 * @brief Affiche l'arbre syntaxique abstrait sur la sortie standard.
 * 
 * @param node Le noeud racine de l'arbre à afficher.
 */
void printTree(Node *node);

#define FIRSTCHILD(node) node->firstChild
#define SECONDCHILD(node) node->firstChild->nextSibling
#define THIRDCHILD(node) node->firstChild->nextSibling->nextSibling
extern const char *StringFromLabel[];

#endif /* TREE_H */