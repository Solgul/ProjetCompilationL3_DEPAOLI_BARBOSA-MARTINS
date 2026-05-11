#include "asm.h"
#include <string.h>
#include <stdlib.h>

static int while_count = 0;
static int if_count = 0;

static const char *amd64_regs[] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};

static void parcours_reste(Node * root, FILE * file, SymbolTable * globalSt, SymbolTable * localSt) {
    if (root == NULL) return;

    switch (root->label) {
        case Num:
            fprintf(file, "    push %d\n", root->num);
            break;

        case Char:
            if (strcmp(root->ident, "'\\n'") == 0) fprintf(file, "    push 10\n");
            else if (strcmp(root->ident, "'\\t'") == 0) fprintf(file, "    push 9\n");
            else fprintf(file, "    push %s\n", root->ident);
            break;

        case Ident: {
            Symbol *s = lookupSymbol(globalSt, localSt, root->ident);
            if (s && (s->use == SYM_VAR || s->use == SYM_PARAM) && localSt && findSymbol(localSt, root->ident) != -1) {
                fprintf(file, "    mov rax, [rbp - %d]\n", s->offset);
            } else {
                fprintf(file, "    mov rax, [%s]\n", root->ident);
            }
            fprintf(file, "    push rax\n");
            break;
        }

        case Affect: {
            parcours_reste(root->firstChild->nextSibling, file, globalSt, localSt);
            fprintf(file, "    pop rax\n");
            Symbol *s = lookupSymbol(globalSt, localSt, root->ident);
            if (s && (s->use == SYM_VAR || s->use == SYM_PARAM) && localSt && findSymbol(localSt, root->ident) != -1) {
                fprintf(file, "    mov [rbp - %d], rax\n", s->offset);
            } else {
                fprintf(file, "    mov [%s], rax\n", root->ident);
            }
            break;
        }

        case Addsub:
        case Divstar:
            parcours_reste(root->firstChild, file, globalSt, localSt);
            parcours_reste(root->firstChild->nextSibling, file, globalSt, localSt);
            fprintf(file, "    pop rbx\n");
            fprintf(file, "    pop rax\n");
            if (strcmp(root->ident, "+") == 0) fprintf(file, "    add rax, rbx\n");
            else if (strcmp(root->ident, "-") == 0) fprintf(file, "    sub rax, rbx\n");
            else if (strcmp(root->ident, "*") == 0) fprintf(file, "    imul rbx\n");
            else if (strcmp(root->ident, "/") == 0) { fprintf(file, "    cqo\n    idiv rbx\n"); }
            fprintf(file, "    push rax\n");
            break;

        case While: {
            int current_while = while_count++;
            fprintf(file, ".L_while_start_%d:\n", current_while);
            parcours_reste(root->firstChild, file, globalSt, localSt); /* Condition */
            fprintf(file, "    pop rax\n    cmp rax, 0\n");
            fprintf(file, "    je .L_while_end_%d\n", current_while);
            if (root->firstChild->nextSibling) 
                parcours_reste(root->firstChild->nextSibling, file, globalSt, localSt); /* Corps */
            fprintf(file, "    jmp .L_while_start_%d\n", current_while);
            fprintf(file, ".L_while_end_%d:\n", current_while);
            break;
        }
        case If: {
            int current_if = if_count++;
            Node *condNode = NULL, *doNode = NULL, *elseNode = NULL;
            for (Node *child = root->firstChild; child != NULL; child = child->nextSibling) {
                if (child->label == Cond) condNode = child;
                else if (child->label == Do) doNode = child;
                else if (child->label == Else) elseNode = child;
            }
            if (condNode && condNode->firstChild) {
                parcours_reste(condNode->firstChild, file, globalSt, localSt);
            }
            fprintf(file, "    pop rax\n");
            fprintf(file, "    cmp rax, 0\n");
            fprintf(file, "    je .L_if_false_%d\n", current_if);
            if (doNode && doNode->firstChild) {
                parcours_reste(doNode->firstChild, file, globalSt, localSt);
            }
            fprintf(file, "    jmp .L_if_end_%d\n", current_if);
            fprintf(file, ".L_if_false_%d:\n", current_if);
            if (elseNode && elseNode->firstChild) {
                parcours_reste(elseNode->firstChild, file, globalSt, localSt);
            }
            fprintf(file, ".L_if_end_%d:\n", current_if);
            break;
        }
        case Funct: {
            int arg_count = 0;
            for (Node *arg = root->firstChild; arg != NULL; arg = arg->nextSibling) {
                parcours_reste(arg, file, globalSt, localSt);
                arg_count++;
            }
            for (int i = arg_count - 1; i >= 0; i--) {
                if (i < 6) fprintf(file, "    pop %s\n", amd64_regs[i]);
            }
            fprintf(file, "    call %s\n", root->ident);
            fprintf(file, "    push rax\n");
            break;
        }
        default:
            for (Node *child = root->firstChild; child != NULL; child = child->nextSibling)
                parcours_reste(child, file, globalSt, localSt);
            break;
    }
}

void parcours_depart(Node * root, FILE * file, SymbolTable * globalSt) {
    if (root == NULL) return;

    if (root->label == Prog) {
        fprintf(file, "section .bss\n");
        for (int i = 0; i < globalSt->nbSymbolInList; i++) {
            if (globalSt->symbolList[i].use == SYM_VAR)
                fprintf(file, "    %s: resq 1\n", globalSt->symbolList[i].ident);
        }
        fprintf(file, "\nsection .text\nextern getint, putchar, putint\n\n");
    }
    if (root->label == DeclFonct) {
        Node *enTete = root->firstChild;
        Node *ident = enTete->firstChild->nextSibling;
        int funcIdx = findSymbol(globalSt, ident->ident);
        SymbolTable *localSt = globalSt->symbolList[funcIdx].child;
        if (strcmp(ident->ident, "main") == 0) {
            fprintf(file, "global _start\n_start:\n");
            parcours_reste(enTete->nextSibling, file, globalSt, localSt);
            fprintf(file, "    pop rdi\n    mov rax, 60\n    syscall\n");
        } else {
            fprintf(file, "%s:\n    push rbp\n    mov rbp, rsp\n", ident->ident);
            fprintf(file, "    sub rsp, %d\n", localSt->nbSymbolInList * 8);
            int p_idx = 0;
            for (int i = 0; i < localSt->nbSymbolInList; i++) {
                if (localSt->symbolList[i].use == SYM_PARAM && p_idx < 6) {
                    fprintf(file, "    mov [rbp - %d], %s\n", localSt->symbolList[i].offset, amd64_regs[p_idx++]);
                }
            }
            parcours_reste(enTete->nextSibling, file, globalSt, localSt);
            fprintf(file, "    mov rsp, rbp\n    pop rbp\n    ret\n\n");
        }
    } else {
        for (Node *child = root->firstChild; child != NULL; child = child->nextSibling)
            parcours_depart(child, file, globalSt);
    }
}