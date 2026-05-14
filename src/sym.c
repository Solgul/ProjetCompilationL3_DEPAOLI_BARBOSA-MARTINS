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
    Symbol *sym = &st->symbolList[st->nbSymbolInList];
    sym->ident = strdup(ident);
    sym->type = type;
    sym->use = use;
    sym->lineno = lineno;
    sym->isStatic = 0;
    sym->child = NULL;
    sym->offset = (st->nbSymbolInList + 1) * 8; 
    st->nbSymbolInList++;
    return 1;
}


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


SymbolTable* buildSymbolTable(Node *root) {
    SymbolTable *globalSt = createSymbolTable();
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
        case Equal:
        case Order:
        case And:
        case Or:
            return TYPE_INT; 
            
        case Dot: {
            Node *base = exp->firstChild;
            Node *champ = base->nextSibling;
            Symbol *sBase = lookupSymbol(globalSt, localSt, base->ident);
            
            if (sBase && sBase->nameStruct) {
                for (int i = 0; i < sBase->nameStruct->nbChamp; i++) {
                    if (strcmp(sBase->nameStruct->champs[i].ident, champ->ident) == 0) {
                        return sBase->nameStruct->champs[i].type;
                    }
                }
            }
            return TYPE_WRONG;
        }
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
    if (instrNode->label == Return) {
        SymbolDataType expectedType = TYPE_VOID; 
        for (int i = 0; i < globalSt->nbSymbolInList; i++) {
            if (globalSt->symbolList[i].child == localSt) {
                expectedType = globalSt->symbolList[i].type;
                break;
            }
        }
        SymbolDataType actualType = TYPE_VOID;
        if (instrNode->firstChild) {
            actualType = getExpType(instrNode->firstChild, globalSt, localSt);
        }
        if (expectedType != actualType && actualType != TYPE_WRONG) {
            fprintf(stderr, "Erreur sémantique ligne %d : Le type de retour ne correspond pas au type de la fonction.\n", instrNode->lineno);
            (*semErrors)++;
        }
    }
    if (instrNode->label == Affect) {
        Node *lvalue = instrNode->firstChild;
        Node *expr = lvalue->nextSibling;
        SymbolDataType tLhs = TYPE_WRONG;
        if (lvalue->label == Ident) {
            Symbol *s = lookupSymbol(globalSt, localSt, lvalue->ident);
            if (!s) {
                fprintf(stderr, "Erreur sémantique ligne %d : Variable '%s' non déclarée.\n", instrNode->lineno, lvalue->ident);
                (*semErrors)++;
            } else {
                tLhs = s->type;
            }
        } else if (lvalue->label == Dot) { // base.champ
            Node *base = lvalue->firstChild;
            Node *champ = base->nextSibling;
            Symbol *sBase = lookupSymbol(globalSt, localSt, base->ident);
            if (!sBase) {
                fprintf(stderr, "Erreur sémantique ligne %d : Structure '%s' non déclarée.\n", instrNode->lineno, base->ident);
                (*semErrors)++;
            } else {
                tLhs = getExpType(lvalue, globalSt, localSt);
                if (tLhs == TYPE_WRONG) {
                    fprintf(stderr, "Erreur sémantique ligne %d : Le champ '%s' n'existe pas dans la structure '%s'.\n", 
                            instrNode->lineno, champ->ident, sBase.);
                    (*semErrors)++;
                }
            }
        }
        SymbolDataType tRhs = getExpType(expr, globalSt, localSt);
        if (tLhs == TYPE_CHAR && tRhs == TYPE_INT) {
            fprintf(stderr, "Avertissement ligne %d : Affectation d'une valeur 'int' à une variable 'char' (risque de perte de données).\n", instrNode->lineno);
        }
    }
    if (instrNode->label == Funct) {
        Symbol *s = lookupSymbol(globalSt, localSt, instrNode->ident);
        if (!s) {
            fprintf(stderr, "Erreur sémantique ligne %d : La fonction '%s' n'est pas déclarée.\n", instrNode->lineno, instrNode->ident);
            (*semErrors)++;
        } else if (s->use == SYM_FUNC) {
            int argCount = 0;
            for (Node *arg = instrNode->firstChild; arg != NULL; arg = arg->nextSibling) {
                argCount++;
            }
            int paramCount = 0;
            if (s->child) {
                for (int i = 0; i < s->child->nbSymbolInList; i++) {
                    if (s->child->symbolList[i].use == SYM_PARAM) paramCount++;
                }
            }
            if (argCount != paramCount) {
                fprintf(stderr, "Erreur sémantique ligne %d : L'appel à la fonction '%s' fournit %d argument(s), mais %d sont attendus.\n", 
                        instrNode->lineno, instrNode->ident, argCount, paramCount);
                (*semErrors)++;
            }
        }
    }
    for (Node *child = instrNode->firstChild; child != NULL; child = child->nextSibling) {
        checkInstructions(child, globalSt, localSt, semErrors);
    }
}

int checkSemantics(Node *root, SymbolTable *globalSt) {
    int semErrors = 0;
    if (!root) return 0;
    int hasMain = findSymbol(globalSt, "main");
    if (hasMain == -1) {
        fprintf(stderr, "Erreur sémantique : La fonction principale 'main' est introuvable.\n");
        semErrors++;
    }
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

void computeOffsets(SymbolTable *st) {
    if (!st) return;
    int current_offset = 0;
    
    for (int i = 0; i < st->nbSymbolInList; i++) {
        if (st->symbolList[i].use == SYM_VAR || st->symbolList[i].use == SYM_PARAM) {
            if (st->symbolList[i].type == TYPE_STRUCT && st->symbolList[i].nameStruct) {
                current_offset += 8 * st->symbolList[i].nameStruct->nbChamp;
            } else {
                current_offset += 8;
            }
            st->symbolList[i].offset = current_offset; 
        }
    }
}

void addPredefinedFunctions(SymbolTable *globalSt) {
    if (!globalSt) return;
    addSymbol(globalSt, "putchar", TYPE_VOID, SYM_PREDEF, 0);
    addSymbol(globalSt, "putint", TYPE_VOID, SYM_PREDEF, 0);
    addSymbol(globalSt, "getchar", TYPE_CHAR, SYM_PREDEF, 0);
    addSymbol(globalSt, "getint", TYPE_INT, SYM_PREDEF, 0);
}