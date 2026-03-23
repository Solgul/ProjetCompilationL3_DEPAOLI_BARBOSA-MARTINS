#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sym.h"
#include "tree.h"

SymbolTable* createSymbolTable() {
    SymbolTable* st = malloc(sizeof(SymbolTable));
    st->nbSymbolInList = 0;
    st->maxSymbolInList = TAB_SIZE;
    return st;
}

SymbolDataType getDataTypeFromString(char *typeStr) {
    if (typeStr == NULL) return TYPE_WRONG;
    if (strcmp(typeStr, "int") == 0) return TYPE_INT;
    if (strcmp(typeStr, "char") == 0) return TYPE_CHAR;
    if (strcmp(typeStr, "void") == 0) return TYPE_VOID;
    return TYPE_STRUCT; 
}

/* ========================================================================= */
/* Vérifier et Ajouter un symbole                                            */
/* ========================================================================= */
int findSymbol(SymbolTable *st, char *ident) {
    for (int i = 0; i < st->nbSymbolInList; i++) {
        if (strcmp(st->symbolList[i].ident, ident) == 0) {
            return i;
        }
    }
    return -1;
}

int addSymbol(SymbolTable *st, char *ident, SymbolDataType type, SymbolType use, int lineno) {
    if (findSymbol(st, ident) != -1) {
        fprintf(stderr, "Erreur sémantique ligne %d : L'identificateur '%s' est déjà déclaré dans cette portée.\n", lineno, ident);
        return 0;
    }
    if (st->nbSymbolInList >= st->maxSymbolInList) {
        fprintf(stderr, "Erreur : La table des symboles est pleine.\n");
        exit(1);
    }
    Symbol *sym = &st->symbolList[st->nbSymbolInList++];
    sym->ident = strdup(ident);
    sym->type = type;
    sym->use = use;
    sym->lineno = lineno;
    sym->isStatic = 0;
    sym->child = NULL;
    return 1;
}

/* ========================================================================= */
/* TRAITEMENT DES DÉCLARATIONS DE VARIABLES (Globales ou Locales)            */
/* ========================================================================= */
void processDeclVars(Node *declVarsNode, SymbolTable *st, SymbolType use) {
    for (Node *typeNode = declVarsNode->firstChild; typeNode != NULL; typeNode = typeNode->nextSibling) {
        if (typeNode->label == Type || typeNode->label == Struct) {
            SymbolDataType dataType = getDataTypeFromString(typeNode->ident);
            for (Node *idNode = typeNode->firstChild; idNode != NULL; idNode = idNode->nextSibling) {
                if (idNode->label == Ident) {
                    addSymbol(st, idNode->ident, dataType, use, idNode->lineno);
                }
            }
        }
    }
}

/* ========================================================================= */
/* Création de la table locale pour une fonction                             */
/* ========================================================================= */
void processFunction(Node *declFonctNode, SymbolTable *globalSt) {
    Node *enTete = declFonctNode->firstChild;
    Node *typeNode = enTete->firstChild;
    Node *funcIdNode = typeNode->nextSibling;
    Node *paramsNode = funcIdNode->nextSibling;   
    SymbolDataType retType = (typeNode->label == Void) ? TYPE_VOID : getDataTypeFromString(typeNode->ident);
    addSymbol(globalSt, funcIdNode->ident, retType, SYM_FUNC, funcIdNode->lineno);
    SymbolTable *localSt = createSymbolTable();
    int funcIndex = findSymbol(globalSt, funcIdNode->ident);
    globalSt->symbolList[funcIndex].child = localSt; 
    if (paramsNode && paramsNode->label == Var) {
        for (Node *pTypeNode = paramsNode->firstChild; pTypeNode != NULL; pTypeNode = pTypeNode->nextSibling) {
            if (pTypeNode->label == Type || pTypeNode->label == Struct) {
                SymbolDataType pType = getDataTypeFromString(pTypeNode->ident);
                Node *pIdNode = pTypeNode->firstChild; 
                if (pIdNode && pIdNode->label == Ident) {
                    addSymbol(localSt, pIdNode->ident, pType, SYM_PARAM, pIdNode->lineno);
                }
            }
        }
    }
    Node *corpsNode = enTete->nextSibling;
    if (corpsNode && corpsNode->label == DeclVars) {
        processDeclVars(corpsNode, localSt, SYM_VAR);
    }
}

/* ========================================================================= */
/* Table des symboles Globale (Point d'entrée)                               */
/* ========================================================================= */
SymbolTable* buildSymbolTable(Node *root) {
    SymbolTable *globalSt = createSymbolTable();
    if (!root) return globalSt;
    
    for (Node *child = root->firstChild; child != NULL; child = child->nextSibling) {
        if (child->label == DeclVars) {
            processDeclVars(child, globalSt, SYM_VAR);
        } else if (child->label == DeclFonct) {
            processFunction(child, globalSt);
        }
    }
    return globalSt;
}

/* ========================================================================= */
/* AFFICHAGE DE LA TABLE                                             */
/* ========================================================================= */
static const char *StringFromSymbolDataType[] = {
    "int", 
    "char", 
    "void", 
    "struct", 
    "erreur"
};

static const char *StringFromSymbolType[] = {
    "Var", 
    "Func", 
    "Param", 
    "Predef"
};

void printSymbolTable(SymbolTable *st, int indent) {
    for (int i = 0; i < st->nbSymbolInList; i++) {
        Symbol *s = &st->symbolList[i];
        for(int j = 0; j < indent; j++) printf("  ");
        printf("- %s (Type: %s, Usage: %s, Ligne: %d)\n", 
               s->ident, 
               StringFromSymbolDataType[s->type], 
               StringFromSymbolType[s->use], 
               s->lineno);
        if (s->use == SYM_FUNC && s->child != NULL) {
            for(int j = 0; j < indent + 1; j++) printf("  ");
            printf("[Table locale de %s] :\n", s->ident);
            printSymbolTable(s->child, indent + 2);
        }
    }
}