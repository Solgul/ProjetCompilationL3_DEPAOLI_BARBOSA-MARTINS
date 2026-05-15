#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sym.h"
#include "tree.h"

static StructInfo structs[TAB_SIZE];
static int nbStructs = 0;
static int semantic_errors_build = 0;

SymbolTable* createSymbolTable() {
    SymbolTable* st = malloc(sizeof(SymbolTable));
    if (!st) { fprintf(stderr, "Erreur : mémoire insuffisante.\n"); exit(3); }
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

static int type_size(SymbolDataType t, StructInfo *si) {
    if (t == TYPE_STRUCT && si) return si->nbChamp * 8;
    return 8;
}

static StructInfo *findStructInfo(const char *name) {
    for (int i = 0; i < nbStructs; i++) {
        if (strcmp(structs[i].name, name) == 0) return &structs[i];
    }
    return NULL;
}

int findSymbol(SymbolTable *st, char *ident) {
    if (!st || !ident) return -1;
    for (int i = 0; i < st->nbSymbolInList; i++) {
        if (strcmp(st->symbolList[i].ident, ident) == 0) return i;
    }
    return -1;
}

int addSymbol(SymbolTable *st, char *ident, SymbolDataType type, SymbolType use, int lineno) {
    if (!st || !ident) return 0;
    if (findSymbol(st, ident) != -1) {
        fprintf(stderr, "Erreur sémantique ligne %d : l'identificateur '%s' est déjà déclaré dans cette portée.\n", lineno, ident);
        semantic_errors_build++;
        return 0;
    }
    if (st->nbSymbolInList >= st->maxSymbolInList) {
        fprintf(stderr, "Erreur : la table des symboles est pleine.\n");
        exit(3);
    }
    Symbol *sym = &st->symbolList[st->nbSymbolInList++];
    sym->ident = strdup(ident);
    sym->type = type;
    sym->use = use;
    sym->lineno = lineno;
    sym->isStatic = 0;
    sym->identFunc = NULL;
    sym->nameStruct = NULL;
    sym->child = NULL;
    sym->offset = 0;
    return 1;
}

static StructInfo *registerStruct(Node *declStruct) {
    if (!declStruct || declStruct->label != DeclStruct) return NULL;
    StructInfo *old = findStructInfo(declStruct->ident);
    if (old) {
        fprintf(stderr, "Erreur sémantique ligne %d : le type struct '%s' est déjà déclaré.\n", declStruct->lineno, declStruct->ident);
        semantic_errors_build++;
        return old;
    }
    if (nbStructs >= TAB_SIZE) { fprintf(stderr, "Erreur : trop de structures.\n"); exit(3); }
    StructInfo *si = &structs[nbStructs++];
    si->name = strdup(declStruct->ident);
    si->champs = calloc(TAB_SIZE, sizeof(Symbol));
    si->nbChamp = 0;

    Node *decls = declStruct->firstChild;
    for (Node *typeNode = decls ? decls->firstChild : NULL; typeNode; typeNode = typeNode->nextSibling) {
        if (typeNode->label != Type && typeNode->label != Struct) continue;
        SymbolDataType t = getDataTypeFromString(typeNode->ident);
        StructInfo *fieldStruct = (typeNode->label == Struct) ? findStructInfo(typeNode->ident) : NULL;
        for (Node *id = typeNode->firstChild; id; id = id->nextSibling) {
            int duplicate = 0;
            for (int i = 0; i < si->nbChamp; i++) if (strcmp(si->champs[i].ident, id->ident) == 0) duplicate = 1;
            if (duplicate) {
                fprintf(stderr, "Erreur sémantique ligne %d : champ '%s' déjà déclaré dans struct '%s'.\n", id->lineno, id->ident, si->name);
                semantic_errors_build++;
                continue;
            }
            Symbol *f = &si->champs[si->nbChamp++];
            f->ident = strdup(id->ident);
            f->type = t;
            f->use = SYM_VAR;
            f->lineno = id->lineno;
            f->nameStruct = fieldStruct;
            f->offset = (si->nbChamp - 1) * 8;
            f->child = NULL;
        }
    }
    return si;
}

static void processOneTypedDecl(Node *typeNode, SymbolTable *st, SymbolType use) {
    if (!typeNode || (typeNode->label != Type && typeNode->label != Struct)) return;
    SymbolDataType dataType = getDataTypeFromString(typeNode->ident);
    StructInfo *si = NULL;
    if (typeNode->label == Struct) {
        si = findStructInfo(typeNode->ident);
        if (!si) {
            fprintf(stderr, "Erreur sémantique ligne %d : le type struct '%s' n'est pas déclaré.\n", typeNode->lineno, typeNode->ident);
            semantic_errors_build++;
        }
    }
    if (dataType == TYPE_VOID && use == SYM_VAR) {
        fprintf(stderr, "Erreur sémantique ligne %d : impossible de déclarer une variable de type void.\n", typeNode->lineno);
        semantic_errors_build++;
        return;
    }
    for (Node *idNode = typeNode->firstChild; idNode; idNode = idNode->nextSibling) {
        if (idNode->label == Ident && addSymbol(st, idNode->ident, dataType, use, idNode->lineno)) {
            st->symbolList[st->nbSymbolInList - 1].nameStruct = si;
        }
    }
}

void processDeclVars(Node *declVarsNode, SymbolTable *st, SymbolType use) {
    if (!declVarsNode) return;
    if (declVarsNode->label == Type || declVarsNode->label == Struct) {
        processOneTypedDecl(declVarsNode, st, use);
        return;
    }
    for (Node *typeNode = declVarsNode->firstChild; typeNode; typeNode = typeNode->nextSibling) {
        processOneTypedDecl(typeNode, st, use);
    }
}

static int countParams(SymbolTable *st) {
    int n = 0;
    if (!st) return 0;
    for (int i = 0; i < st->nbSymbolInList; i++) if (st->symbolList[i].use == SYM_PARAM) n++;
    return n;
}

void computeOffsets(SymbolTable *st) {
    if (!st) return;
    int current = 0;
    for (int i = 0; i < st->nbSymbolInList; i++) {
        if (st->symbolList[i].use == SYM_VAR || st->symbolList[i].use == SYM_PARAM) {
            current += type_size(st->symbolList[i].type, st->symbolList[i].nameStruct);
            st->symbolList[i].offset = current;
        }
    }
}

void processFunction(Node *declFonctNode, SymbolTable *globalSt) {
    Node *enTete = declFonctNode->firstChild;
    Node *typeNode = enTete->firstChild;
    Node *funcIdNode = typeNode->nextSibling;
    Node *paramsNode = funcIdNode->nextSibling;
    SymbolDataType retType = (typeNode->label == Void) ? TYPE_VOID : getDataTypeFromString(typeNode->ident);
    StructInfo *retStruct = (typeNode->label == Struct) ? findStructInfo(typeNode->ident) : NULL;

    if (!addSymbol(globalSt, funcIdNode->ident, retType, SYM_FUNC, funcIdNode->lineno)) return;
    int funcIndex = findSymbol(globalSt, funcIdNode->ident);
    globalSt->symbolList[funcIndex].nameStruct = retStruct;
    SymbolTable *localSt = createSymbolTable();
    globalSt->symbolList[funcIndex].child = localSt;

    if (paramsNode && paramsNode->label == Var) {
        for (Node *pTypeNode = paramsNode->firstChild; pTypeNode; pTypeNode = pTypeNode->nextSibling) {
            if (pTypeNode->label == Type || pTypeNode->label == Struct) {
                SymbolDataType pType = getDataTypeFromString(pTypeNode->ident);
                StructInfo *pStruct = (pTypeNode->label == Struct) ? findStructInfo(pTypeNode->ident) : NULL;
                Node *pIdNode = pTypeNode->firstChild;
                if (pIdNode && pIdNode->label == Ident && addSymbol(localSt, pIdNode->ident, pType, SYM_PARAM, pIdNode->lineno)) {
                    localSt->symbolList[localSt->nbSymbolInList - 1].nameStruct = pStruct;
                }
            }
        }
    }
    Node *corpsNode = enTete->nextSibling;
    if (corpsNode && corpsNode->label == DeclVars) processDeclVars(corpsNode, localSt, SYM_VAR);
    computeOffsets(localSt);
}

void addPredefinedFunctions(SymbolTable *globalSt) {
    if (!globalSt) return;
    if (findSymbol(globalSt, "putchar") == -1) addSymbol(globalSt, "putchar", TYPE_VOID, SYM_PREDEF, 0);
    if (findSymbol(globalSt, "putint") == -1) addSymbol(globalSt, "putint", TYPE_VOID, SYM_PREDEF, 0);
    if (findSymbol(globalSt, "getchar") == -1) addSymbol(globalSt, "getchar", TYPE_CHAR, SYM_PREDEF, 0);
    if (findSymbol(globalSt, "getint") == -1) addSymbol(globalSt, "getint", TYPE_INT, SYM_PREDEF, 0);
}

SymbolTable* buildSymbolTable(Node *root) {
    semantic_errors_build = 0;
    nbStructs = 0;
    SymbolTable *globalSt = createSymbolTable();
    addPredefinedFunctions(globalSt);
    if (!root) return globalSt;

    for (Node *child = root->firstChild; child; child = child->nextSibling) {
        if (child->label == DeclStruct) registerStruct(child);
    }
    for (Node *child = root->firstChild; child; child = child->nextSibling) {
        if (child->label == Type || child->label == Struct) processDeclVars(child, globalSt, SYM_VAR);
        else if (child->label == DeclFonct) processFunction(child, globalSt);
    }
    computeOffsets(globalSt);
    return globalSt;
}

static const char *StringFromSymbolDataType[] = {"int", "char", "void", "struct", "erreur"};
static const char *StringFromSymbolType[] = {"Var", "Func", "Param", "Predef"};

void printSymbolTable(SymbolTable *st, int indent) {
    if (!st) return;
    for (int i = 0; i < st->nbSymbolInList; i++) {
        Symbol *s = &st->symbolList[i];
        for (int j = 0; j < indent; j++) printf("  ");
        printf("- %s (Type: %s", s->ident, StringFromSymbolDataType[s->type]);
        if (s->nameStruct) printf(" %s", s->nameStruct->name);
        printf(", Usage: %s, Ligne: %d, Offset: %d)\n", StringFromSymbolType[s->use], s->lineno, s->offset);
        if (s->use == SYM_FUNC && s->child) {
            for (int j = 0; j < indent + 1; j++) printf("  ");
            printf("[Table locale de %s] :\n", s->ident);
            printSymbolTable(s->child, indent + 2);
        }
    }
}

Symbol* lookupSymbol(SymbolTable *globalSt, SymbolTable *localSt, char *ident) {
    if (localSt) {
        int idx = findSymbol(localSt, ident);
        if (idx != -1) return &localSt->symbolList[idx];
    }
    if (globalSt) {
        int idx = findSymbol(globalSt, ident);
        if (idx != -1) return &globalSt->symbolList[idx];
    }
    return NULL;
}

static StructInfo *getExpStruct(Node *exp, SymbolTable *globalSt, SymbolTable *localSt);
static void check_expr(Node *exp, SymbolTable *globalSt, SymbolTable *localSt, int *semErrors, int asValue);
static void check_stmt_list(Node *n, SymbolTable *globalSt, SymbolTable *localSt, int *semErrors, Symbol *currentFunc);

SymbolDataType getExpType(Node *exp, SymbolTable *globalSt, SymbolTable *localSt) {
    if (!exp) return TYPE_VOID;
    switch (exp->label) {
        case Num: return TYPE_INT;
        case Char: return TYPE_CHAR;
        case Ident: {
            Symbol *s = lookupSymbol(globalSt, localSt, exp->ident);
            return s ? s->type : TYPE_WRONG;
        }
        case Funct: {
            Symbol *s = lookupSymbol(globalSt, localSt, exp->ident);
            return (s && (s->use == SYM_FUNC || s->use == SYM_PREDEF)) ? s->type : TYPE_WRONG;
        }
        case Dot: {
            Node *base = exp->firstChild;
            Node *field = base ? base->nextSibling : NULL;
            StructInfo *si = getExpStruct(base, globalSt, localSt);
            if (si && field) {
                for (int i = 0; i < si->nbChamp; i++) {
                    if (strcmp(si->champs[i].ident, field->ident) == 0) return si->champs[i].type;
                }
            }
            return TYPE_WRONG;
        }
        case Addsub: case Divstar: case Equal: case Order: case And: case Or: case UnOperator: case Negate:
            return TYPE_INT;
        case Cond: case Do: case Else: case Instr:
            return getExpType(exp->firstChild, globalSt, localSt);
        default: return TYPE_WRONG;
    }
}

static StructInfo *getExpStruct(Node *exp, SymbolTable *globalSt, SymbolTable *localSt) {
    if (!exp) return NULL;
    switch (exp->label) {
        case Ident: {
            Symbol *s = lookupSymbol(globalSt, localSt, exp->ident);
            return s ? s->nameStruct : NULL;
        }
        case Funct: {
            Symbol *s = lookupSymbol(globalSt, localSt, exp->ident);
            return s ? s->nameStruct : NULL;
        }
        case Dot: {
            Node *base = exp->firstChild;
            Node *field = base ? base->nextSibling : NULL;
            StructInfo *si = getExpStruct(base, globalSt, localSt);
            if (si && field) {
                for (int i = 0; i < si->nbChamp; i++) {
                    if (strcmp(si->champs[i].ident, field->ident) == 0) return si->champs[i].nameStruct;
                }
            }
            return NULL;
        }
        default:
            return NULL;
    }
}

static int same_struct(StructInfo *a, StructInfo *b) {
    if (a == NULL || b == NULL) return a == b;
    return strcmp(a->name, b->name) == 0;
}

static void check_assignment_compat_ext(int line, SymbolDataType dst, StructInfo *dstStruct,
                                        SymbolDataType src, StructInfo *srcStruct,
                                        int *semErrors, const char *ctx) {
    if (dst == TYPE_WRONG || src == TYPE_WRONG) return;
    if (dst == TYPE_VOID || src == TYPE_VOID) {
        fprintf(stderr, "Erreur sémantique ligne %d : type void interdit dans %s.\n", line, ctx);
        (*semErrors)++;
        return;
    }
    if (dst == TYPE_STRUCT || src == TYPE_STRUCT) {
        if (dst != TYPE_STRUCT || src != TYPE_STRUCT || !same_struct(dstStruct, srcStruct)) {
            fprintf(stderr, "Erreur sémantique ligne %d : types incompatibles dans %s.\n", line, ctx);
            (*semErrors)++;
        }
        return;
    }
    if (dst == TYPE_CHAR && src == TYPE_INT) {
        fprintf(stderr, "Avertissement ligne %d : conversion int vers char dans %s.\n", line, ctx);
    }
}

static void check_assignment_compat(int line, SymbolDataType dst, SymbolDataType src, int *semErrors, const char *ctx) {
    check_assignment_compat_ext(line, dst, NULL, src, NULL, semErrors, ctx);
}

static Symbol *check_lvalue(Node *lhs, SymbolTable *globalSt, SymbolTable *localSt, int *semErrors) {
    if (!lhs) {
        fprintf(stderr, "Erreur sémantique : LValue absente.\n");
        (*semErrors)++;
        return NULL;
    }
    if (lhs->label == Ident) {
        Symbol *s = lookupSymbol(globalSt, localSt, lhs->ident);
        if (!s || !(s->use == SYM_VAR || s->use == SYM_PARAM)) {
            fprintf(stderr, "Erreur sémantique ligne %d : '%s' n'est pas une variable déclarée.\n", lhs->lineno, lhs->ident);
            (*semErrors)++;
            return NULL;
        }
        return s;
    }
    if (lhs->label == Dot) {
        Node *base = lhs->firstChild;
        Node *field = base ? base->nextSibling : NULL;
        Symbol *baseSym = NULL;
        if (!base || base->label != Ident || !field || field->label != Ident) {
            fprintf(stderr, "Erreur sémantique ligne %d : accès à un champ invalide.\n test : %d\n ident : %d\n dot : %d\n", lhs->lineno, base->label, Ident, Dot);
            (*semErrors)++;
            return NULL;
        }
        baseSym = lookupSymbol(globalSt, localSt, base->ident);
        if (!baseSym || !(baseSym->use == SYM_VAR || baseSym->use == SYM_PARAM)) {
            fprintf(stderr, "Erreur sémantique ligne %d : '%s' n'est pas une structure déclarée.\n", base->lineno, base->ident);
            (*semErrors)++;
            return NULL;
        }
        if (baseSym->type != TYPE_STRUCT || !baseSym->nameStruct) {
            fprintf(stderr, "Erreur sémantique ligne %d : l'opérateur . s'applique uniquement aux structures.\n", lhs->lineno);
            (*semErrors)++;
            return NULL;
        }
        for (int i = 0; i < baseSym->nameStruct->nbChamp; i++) {
            if (strcmp(baseSym->nameStruct->champs[i].ident, field->ident) == 0) return &baseSym->nameStruct->champs[i];
        }
        fprintf(stderr, "Erreur sémantique ligne %d : le champ '%s' n'existe pas dans struct %s.\n", field->lineno, field->ident, baseSym->nameStruct->name);
        (*semErrors)++;
        return NULL;
    }
    fprintf(stderr, "Erreur sémantique ligne %d : partie gauche d'affectation invalide.\n", lhs->lineno);
    (*semErrors)++;
    return NULL;
}

static void check_call(Node *call, SymbolTable *globalSt, SymbolTable *localSt, int *semErrors, int asValue) {
    Symbol *s = lookupSymbol(globalSt, localSt, call->ident);
    int argc = 0;
    for (Node *arg = call->firstChild; arg; arg = arg->nextSibling) argc++;

    if (!s) {
        fprintf(stderr, "Erreur sémantique ligne %d : la fonction '%s' n'est pas déclarée.\n", call->lineno, call->ident);
        (*semErrors)++;
    } else if (!(s->use == SYM_FUNC || s->use == SYM_PREDEF)) {
        fprintf(stderr, "Erreur sémantique ligne %d : '%s' n'est pas une fonction.\n", call->lineno, call->ident);
        (*semErrors)++;
    } else {
        if (asValue && s->type == TYPE_VOID) {
            fprintf(stderr, "Erreur sémantique ligne %d : une fonction void ne peut pas être utilisée comme expression.\n", call->lineno);
            (*semErrors)++;
        }
        if (s->use == SYM_PREDEF) {
            int expected = (!strcmp(s->ident, "putchar") || !strcmp(s->ident, "putint")) ? 1 : 0;
            if (argc != expected) {
                fprintf(stderr, "Erreur sémantique ligne %d : la fonction '%s' attend %d argument(s).\n", call->lineno, s->ident, expected);
                (*semErrors)++;
            } else if (argc == 1) {
                Node *arg = call->firstChild;
                SymbolDataType at = getExpType(arg, globalSt, localSt);
                StructInfo *as = getExpStruct(arg, globalSt, localSt);
                SymbolDataType expectedType = !strcmp(s->ident, "putchar") ? TYPE_CHAR : TYPE_INT;
                check_assignment_compat_ext(call->lineno, expectedType, NULL, at, as, semErrors, "appel de fonction");
            }
        } else {
            int expected = countParams(s->child);
            if (argc != expected) {
                fprintf(stderr, "Erreur sémantique ligne %d : la fonction '%s' attend %d argument(s), %d fourni(s).\n", call->lineno, call->ident, expected, argc);
                (*semErrors)++;
            }
            int pos = 0;
            for (Node *arg = call->firstChild; arg; arg = arg->nextSibling, pos++) {
                int k = 0;
                for (int i = 0; s->child && i < s->child->nbSymbolInList; i++) {
                    if (s->child->symbolList[i].use == SYM_PARAM) {
                        if (k == pos) {
                            check_assignment_compat_ext(call->lineno,
                                s->child->symbolList[i].type, s->child->symbolList[i].nameStruct,
                                getExpType(arg, globalSt, localSt), getExpStruct(arg, globalSt, localSt),
                                semErrors, "appel de fonction");
                            break;
                        }
                        k++;
                    }
                }
            }
        }
    }
    for (Node *arg = call->firstChild; arg; arg = arg->nextSibling) check_expr(arg, globalSt, localSt, semErrors, 1);
}

static void check_expr(Node *exp, SymbolTable *globalSt, SymbolTable *localSt, int *semErrors, int asValue) {
    if (!exp) return;
    switch (exp->label) {
        case Num:
        case Char:
            return;
        case Ident: {
            Symbol *s = lookupSymbol(globalSt, localSt, exp->ident);
            if (!s || !(s->use == SYM_VAR || s->use == SYM_PARAM)) {
                fprintf(stderr, "Erreur sémantique ligne %d : l'identificateur '%s' n'est pas une variable déclarée.\n", exp->lineno, exp->ident);
                (*semErrors)++;
            }
            return;
        }
        case Dot:
            (void)check_lvalue(exp, globalSt, localSt, semErrors);
            return;
        case Funct:
            check_call(exp, globalSt, localSt, semErrors, asValue);
            return;
        case Addsub: case Divstar: case Equal: case Order: case And: case Or: {
            Node *a = exp->firstChild;
            Node *b = a ? a->nextSibling : NULL;
            check_expr(a, globalSt, localSt, semErrors, 1);
            check_expr(b, globalSt, localSt, semErrors, 1);
            SymbolDataType ta = getExpType(a, globalSt, localSt);
            SymbolDataType tb = getExpType(b, globalSt, localSt);
            if (ta == TYPE_STRUCT || ta == TYPE_VOID || tb == TYPE_STRUCT || tb == TYPE_VOID) {
                fprintf(stderr, "Erreur sémantique ligne %d : opération invalide sur ce type.\n", exp->lineno);
                (*semErrors)++;
            }
            return;
        }
        case UnOperator: case Negate: {
            Node *a = exp->firstChild;
            check_expr(a, globalSt, localSt, semErrors, 1);
            SymbolDataType ta = getExpType(a, globalSt, localSt);
            if (ta == TYPE_STRUCT || ta == TYPE_VOID) {
                fprintf(stderr, "Erreur sémantique ligne %d : opérateur unaire invalide sur ce type.\n", exp->lineno);
                (*semErrors)++;
            }
            return;
        }
        case Cond: case Do: case Else: case Instr:
            check_expr(exp->firstChild, globalSt, localSt, semErrors, asValue);
            return;
        default:
            for (Node *child = exp->firstChild; child; child = child->nextSibling) check_expr(child, globalSt, localSt, semErrors, asValue);
            return;
    }
}

static void check_stmt(Node *n, SymbolTable *globalSt, SymbolTable *localSt, int *semErrors, Symbol *currentFunc) {
    if (!n) return;
    switch (n->label) {
        case DeclVars: case DeclStruct: case Type: case Struct: case EnTeteFonct: case Var: case Void:
            return;
        case Instr: case Do: case Else:
            check_stmt_list(n->firstChild, globalSt, localSt, semErrors, currentFunc);
            return;
        case Affect: {
            Node *lhs = n->firstChild;
            Node *rhs = lhs ? lhs->nextSibling : NULL;
            Symbol *ls = check_lvalue(lhs, globalSt, localSt, semErrors);
            check_expr(rhs, globalSt, localSt, semErrors, 1);
            SymbolDataType lt = ls ? ls->type : TYPE_WRONG;
            StructInfo *lsn = ls ? ls->nameStruct : NULL;
            check_assignment_compat_ext(n->lineno, lt, lsn, getExpType(rhs, globalSt, localSt), getExpStruct(rhs, globalSt, localSt), semErrors, "affectation");
            return;
        }
        case Return: {
            SymbolDataType expected = currentFunc ? currentFunc->type : TYPE_VOID;
            StructInfo *expectedStruct = currentFunc ? currentFunc->nameStruct : NULL;
            if (n->firstChild) {
                check_expr(n->firstChild, globalSt, localSt, semErrors, 1);
                check_assignment_compat_ext(n->lineno, expected, expectedStruct,
                    getExpType(n->firstChild, globalSt, localSt), getExpStruct(n->firstChild, globalSt, localSt),
                    semErrors, "return");
                if (expected == TYPE_VOID) {
                    fprintf(stderr, "Erreur sémantique ligne %d : return avec valeur dans une fonction void.\n", n->lineno);
                    (*semErrors)++;
                }
            } else if (expected != TYPE_VOID) {
                fprintf(stderr, "Erreur sémantique ligne %d : return sans valeur dans une fonction non void.\n", n->lineno);
                (*semErrors)++;
            }
            return;
        }
        case If: case While: {
            Node *cw = n->firstChild;
            Node *cond = cw ? cw->firstChild : NULL;
            check_expr(cond, globalSt, localSt, semErrors, 1);
            SymbolDataType ct = getExpType(cond, globalSt, localSt);
            if (ct == TYPE_VOID || ct == TYPE_STRUCT || ct == TYPE_WRONG) {
                fprintf(stderr, "Erreur sémantique ligne %d : condition invalide.\n", n->lineno);
                (*semErrors)++;
            }
            for (Node *child = cw ? cw->nextSibling : NULL; child; child = child->nextSibling) check_stmt(child, globalSt, localSt, semErrors, currentFunc);
            return;
        }
        case Funct:
            check_call(n, globalSt, localSt, semErrors, 0);
            return;
        default:
            check_expr(n, globalSt, localSt, semErrors, 1);
            return;
    }
}

static void check_stmt_list(Node *n, SymbolTable *globalSt, SymbolTable *localSt, int *semErrors, Symbol *currentFunc) {
    for (Node *cur = n; cur; cur = cur->nextSibling) check_stmt(cur, globalSt, localSt, semErrors, currentFunc);
}

void checkInstructions(Node *instrNode, SymbolTable *globalSt, SymbolTable *localSt, int *semErrors) {
    Symbol *currentFunc = NULL;
    if (globalSt) {
        for (int i = 0; i < globalSt->nbSymbolInList; i++) {
            if (globalSt->symbolList[i].child == localSt) { currentFunc = &globalSt->symbolList[i]; break; }
        }
    }
    check_stmt_list(instrNode, globalSt, localSt, semErrors, currentFunc);
}

int checkSemantics(Node *root, SymbolTable *globalSt) {
    int semErrors = semantic_errors_build;
    if (!root) return semErrors;
    int mainIdx = findSymbol(globalSt, "main");
    if (mainIdx == -1 || globalSt->symbolList[mainIdx].use != SYM_FUNC) {
        fprintf(stderr, "Erreur sémantique : la fonction main est introuvable.\n");
        semErrors++;
    } else if (globalSt->symbolList[mainIdx].type != TYPE_INT) {
        fprintf(stderr, "Erreur sémantique ligne %d : main doit renvoyer int.\n", globalSt->symbolList[mainIdx].lineno);
        semErrors++;
    }

    for (Node *child = root->firstChild; child; child = child->nextSibling) {
        if (child->label == DeclFonct) {
            Node *enTete = child->firstChild;
            if (!enTete || !enTete->firstChild || !enTete->firstChild->nextSibling) continue;
            Node *funcIdNode = enTete->firstChild->nextSibling;
            int idx = findSymbol(globalSt, funcIdNode->ident);
            Symbol *funcSym = idx >= 0 ? &globalSt->symbolList[idx] : NULL;
            SymbolTable *localSt = funcSym ? funcSym->child : NULL;
            Node *body = enTete->nextSibling;
            if (body) check_stmt_list(body, globalSt, localSt, &semErrors, funcSym);
        }
    }
    return semErrors;
}
