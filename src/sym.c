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
    // addSymbol(globalSt, "putchar", TYPE_VOID, SYM_PREDEF, 0);
    // addSymbol(globalSt, "putint", TYPE_VOID, SYM_PREDEF, 0);
    // addSymbol(globalSt, "getchar", TYPE_CHAR, SYM_PREDEF, 0);
    // addSymbol(globalSt, "getint", TYPE_INT, SYM_PREDEF, 0);
    if (!root) return globalSt;
    for (Node *child = root->firstChild; child != NULL; child = child->nextSibling) {
        if (child->label == Type || child->label == Struct) {
            SymbolDataType dataType = getDataTypeFromString(child->ident);
            for (Node *idNode = child->firstChild; idNode != NULL; idNode = idNode->nextSibling) {
                if (idNode->label == Ident) {
                    addSymbol(globalSt, idNode->ident, dataType, SYM_VAR, idNode->lineno);
                }
            }
        } 
        else if (child->label == DeclFonct) {
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

Symbol* lookupSymbol(SymbolTable *globalSt, SymbolTable *localSt, char *ident) {
    if (localSt != NULL) {
        int idx = findSymbol(localSt, ident);
        if (idx != -1) return &localSt->symbolList[idx];
    }
    if (globalSt != NULL) {
        int idx = findSymbol(globalSt, ident);
        if (idx != -1) return &globalSt->symbolList[idx];
    }
    return NULL;
}

SymbolDataType getExpType(Node *exp, SymbolTable *globalSt, SymbolTable *localSt) {
    if (!exp) return TYPE_WRONG;
    
    switch (exp->label) {
        case Num:
            return TYPE_INT;
        case Char:
            return TYPE_CHAR;
        case Ident: {
            Symbol *s = lookupSymbol(globalSt, localSt, exp->ident);
            if (s) return s->type;
            return TYPE_WRONG;
        }
        case Funct: {
            Symbol *s = lookupSymbol(globalSt, localSt, exp->ident);
            if (s && (s->use == SYM_FUNC || s->use == SYM_PREDEF)) return s->type;
            return TYPE_WRONG;
        }
        case Addsub:
        case Divstar:
        case UnOperator:
        case Equal:
        case Order:
        case And:
        case Or:
            return TYPE_INT;
        default:
            return TYPE_WRONG;
    }
}

void checkInstructions(Node *instrNode, SymbolTable *globalSt, SymbolTable *localSt, int *semErrors) {
    if (!instrNode) return;
    if (instrNode->label == Ident) {
        Symbol *s = lookupSymbol(globalSt, localSt, instrNode->ident);
        if (!s) {
            fprintf(stderr, "Erreur sémantique ligne %d : L'identificateur '%s' n'est pas déclaré.\n", instrNode->lineno, instrNode->ident);
            (*semErrors)++;
        }
    }
    if (instrNode->label == Affect) {
        if(instrNode->firstChild->label == Dot) {//du type struct.nom_var = qqc
            Node * structVar = instrNode->firstChild;
            //jsp comment tester si la var est du bon type
        }
        char *varName = instrNode->ident;
        Symbol *sLhs = lookupSymbol(globalSt, localSt, varName);
        if (!sLhs) {
            fprintf(stderr, "Erreur sémantique ligne %d : La variable '%s' n'est pas déclarée.\n", instrNode->lineno, varName);
            (*semErrors)++;
        } else {
            Node *rhs = instrNode->firstChild;
            SymbolDataType tRhs = getExpType(rhs, globalSt, localSt);
            SymbolDataType tLhs = sLhs->type;
            if (tLhs == TYPE_CHAR && tRhs == TYPE_INT) {
                fprintf(stderr, "Avertissement ligne %d : affectation d'une valeur 'int' à une variable 'char' (%s).\n", instrNode->lineno, varName);
            }
        }
    }
    if (instrNode->label == Funct) {
        Symbol *s = lookupSymbol(globalSt, localSt, instrNode->ident);
        if (!s) {
            fprintf(stderr, "Erreur sémantique ligne %d : La fonction '%s' n'est pas déclarée.\n", instrNode->lineno, instrNode->ident);
            (*semErrors)++;
        }
    }
    for (Node *child = instrNode->firstChild; child != NULL; child = child->nextSibling) {
        checkInstructions(child, globalSt, localSt, semErrors);
    }
}

int checkSemantics(Node *root, SymbolTable *globalSt) {
    int semErrors = 0;
    if (!root) return 0;
    
    for (Node *child = root->firstChild; child != NULL; child = child->nextSibling) {
        if (child->label == DeclFonct) {
            Node *enTete = child->firstChild;
            Node *funcIdNode = enTete->firstChild->nextSibling; 
            int funcIdx = findSymbol(globalSt, funcIdNode->ident);
            SymbolTable *localSt = NULL;
            if (funcIdx != -1) {
                localSt = globalSt->symbolList[funcIdx].child;
            }
            for (Node *n = child->firstChild; n != NULL; n = n->nextSibling) {
                if (n->label == Instr) {
                    checkInstructions(n, globalSt, localSt, &semErrors);
                }
            }
        }
    }
    return semErrors;
}