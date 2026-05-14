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
    SYM_STRUCT,
    SYM_PREDEF   // Fonction prédéfinie (putchar, getint...)
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
    int offset;
} Symbol;

typedef struct SymbolTable {
    Symbol symbolList [TAB_SIZE];
    int maxSymbolInList;
    int nbSymbolInList;
} SymbolTable;

/**
 * @brief Alloue et initialise une nouvelle table des symboles vide.
 * 
 * @return SymbolTable* Un pointeur vers la nouvelle table des symboles.
 */
SymbolTable* createSymbolTable();

/**
 * @brief Cherche un identificateur spécifique dans une table des symboles donnée.
 * 
 * @param st Un pointeur vers la table des symboles dans laquelle chercher.
 * @param ident La chaîne de caractères représentant l'identificateur recherché.
 * @return int L'index du symbole dans la table s'il est trouvé, -1 sinon.
 */
int findSymbol(SymbolTable *st, char *ident);

/**
 * @brief Ajoute un nouveau symbole dans la table des symboles s'il n'existe pas déjà.
 * 
 * @param st Un pointeur vers la table des symboles.
 * @param ident Le nom de l'identificateur.
 * @param type Le type de données du symbole (int, char, void, struct).
 * @param use Le rôle du symbole (variable, fonction, paramètre).
 * @param lineno Le numéro de ligne où le symbole a été déclaré.
 * @return int 1 si l'ajout a réussi, 0 en cas d'échec (ex: symbole déjà existant).
 */
int addSymbol(SymbolTable *st, char *ident, SymbolDataType type, SymbolType use, int lineno);

/**
 * @brief Parcourt l'arbre syntaxique abstrait pour construire la table des symboles globale et les tables locales.
 * 
 * @param root Le noeud racine de l'arbre (généralement le noeud Prog).
 * @return SymbolTable* Un pointeur vers la table des symboles globale construite.
 */
SymbolTable* buildSymbolTable(Node *root);

/**
 * @brief Affiche le contenu d'une table des symboles de manière lisible.
 * 
 * @param st Un pointeur vers la table des symboles à afficher.
 * @param indent Le niveau d'indentation pour l'affichage.
 */
void printSymbolTable(SymbolTable *st, int indent);

/**
 * @brief Recherche un symbole en regardant d'abord dans la table locale, puis dans la table globale.
 * 
 * @param globalSt La table des symboles globale.
 * @param localSt La table des symboles locale.
 * @param ident Le nom de l'identificateur recherché.
 * @return Symbol* Un pointeur vers le symbole s'il est trouvé, NULL sinon.
 */
Symbol* lookupSymbol(SymbolTable *globalSt, SymbolTable *localSt, char *ident);

/**
 * @brief Détermine et renvoie le type de retour d'une expression donnée.
 * 
 * @param exp Le noeud de l'expression à évaluer.
 * @param globalSt La table des symboles globale.
 * @param localSt La table des symboles locale.
 * @return SymbolDataType Le type de l'expression calculée.
 */
SymbolDataType getExpType(Node *exp, SymbolTable *globalSt, SymbolTable *localSt);

/**
 * @brief Parcourt un bloc d'instructions pour vérifier les erreurs sémantiques (déclarations manquantes, types incompatibles).
 * 
 * @param instrNode Le noeud contenant l'instruction à vérifier.
 * @param globalSt La table des symboles globale.
 * @param localSt La table des symboles locale.
 * @param semErrors Un pointeur vers le compteur d'erreurs sémantiques (incrémenté si une erreur est trouvée).
 */
void checkInstructions(Node *instrNode, SymbolTable *globalSt, SymbolTable *localSt, int *semErrors);

/**
 * @brief Point d'entrée de l'analyse sémantique. Parcourt l'arbre pour lancer les vérifications sur les fonctions et instructions.
 * 
 * @param root Le noeud racine de l'arbre.
 * @param globalSt La table des symboles globale complète.
 * @return int Le nombre total d'erreurs sémantiques rencontrées (0 si le code est sémantiquement correct).
 */
int checkSemantics(Node *root, SymbolTable *globalSt);

/**
 * @brief met a jour les offset pour la table des symbole 
 * 
 * @param st 
 */
void computeOffsets(SymbolTable *st);

/**
 * @brief Ajoute les fonction predéfinis dans la table des symbole.
 * 
 * @param globalSt La table des symboles globale complète.
 */
void addPredefinedFunctions(SymbolTable *globalSt);

#endif