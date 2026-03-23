#ifndef SYM
#define SYM

#include "tree.h"

#define TAB_SIZE 256

typedef enum {
    TYPE_INT,    // un int
    TYPE_CHAR,   // un char
    TYPE_VOID,   // void
    TYPE_STRUCT, // une struct
    TYPE_WRONG   // type non valide
} SymbolDataType;

typedef enum {
    SYM_VAR,     // Variable
    SYM_FUNC,    // Fonction
    SYM_PARAM,   // Paramètre
    SYM_PREDEF   // Fonction prédéfinie (non redéfinissable)
} SymbolType;

typedef struct StructInfo {
    char * name;
    struct Symbol * champs;
    int nbChamp; // le nombre de champs de la structure
} StructInfo;

typedef struct Symbol {
    char* ident;
    SymbolDataType type;
    SymbolType use;
    int lineno;
    int isStatic; // 1 if static, 0 if not
    char* identFunc; // identifiant de la fonction d'origine
    StructInfo * nameStruct; 
    struct SymbolTable * child; 
} Symbol;

typedef struct SymbolTable {
    Symbol symbolList [TAB_SIZE];
    int maxSymbolInList;
    int nbSymbolInList;
} SymbolTable;

/* ========================================================================= */
/* PROTOTYPES DES FONCTIONS DE sym.c                                         */
/* ========================================================================= */
SymbolTable* createSymbolTable();
int findSymbol(SymbolTable *st, char *ident);
int addSymbol(SymbolTable *st, char *ident, SymbolDataType type, SymbolType use, int lineno);
SymbolTable* buildSymbolTable(Node *root);
void printSymbolTable(SymbolTable *st, int indent);

#endif