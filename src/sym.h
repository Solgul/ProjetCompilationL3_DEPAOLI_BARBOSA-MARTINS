#ifndef SYM
#define SYM

#define TAB_SIZE 256

// Type d'un symbole 
typedef enum {
    TYPE_INT, //un int
    TYPE_CHAR, //un char
    TYPE_VOID, //void
    TYPE_STRUCT, //une struct
    TYPE_WRONG //type non valide
} SymbolDataType;

// symbolisme d'un symbole (variable, fonction, paramètre)
typedef enum {
    SYM_VAR,    // Variable
    SYM_FUNC,   // Fonction
    SYM_PARAM,   // Paramètre
    SYM_PREDEF  // Fonction prédéfinie (non redéfinissable)
} SymbolType;

typedef struct Struct {
    char * name;
    struct Symbol * champs;
    int nbChamp; //le nombre de champs de la structure
} Struct;

typedef struct Symbol {
    char* ident;
    SymbolDataType type;
    SymbolType use;
    int lineno;
    int isStatic; //1 if static, 0 if not
    char* identFunc; //identifient de la fonction d'origine
    Struct * nameStruct;
    struct SymbolTable * child; //????
} Symbol;

typedef struct SymbolTable {
    Symbol symbolList [TAB_SIZE];
    int maxSymbolInList;
    int nbSymbolInList;
} SymbolTable;

#endif