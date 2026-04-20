#include "asm.h"
#include <string.h>

static void parcours_reste(Node * root, FILE * file, SymbolTable * globalSt) {
    if (root == NULL) return;

    if (root->label == Addsub) {
        parcours_reste(root->firstChild, file, globalSt);
        parcours_reste(root->firstChild->nextSibling, file, globalSt);
        if (root->ident != NULL && strcmp(root->ident, "-") == 0) {
            fprintf(file, "    pop rbx\n");
            fprintf(file, "    pop rax\n");
            fprintf(file, "    sub rax, rbx\n");
            fprintf(file, "    push rax\n");
        } else if (root->ident != NULL && strcmp(root->ident, "+") == 0) {
            fprintf(file, "    pop rbx\n");
            fprintf(file, "    pop rax\n");
            fprintf(file, "    add rax, rbx\n");
            fprintf(file, "    push rax\n");
        }
    } 
    else if (root->label == Divstar) { 
        parcours_reste(root->firstChild, file, globalSt);
        parcours_reste(root->firstChild->nextSibling, file, globalSt);
        if (root->ident != NULL && strcmp(root->ident, "/") == 0) {
            fprintf(file, "    pop rbx\n");
            fprintf(file, "    pop rax\n");
            fprintf(file, "    cqo\n");
            fprintf(file, "    idiv rbx\n");
            fprintf(file, "    push rax\n");
        } else if (root->ident != NULL && strcmp(root->ident, "*") == 0) {
            fprintf(file, "    pop rbx\n");
            fprintf(file, "    pop rax\n");
            fprintf(file, "    imul rbx\n");
            fprintf(file, "    push rax\n");
        }
    }
    else if (root->label == Num) {
        fprintf(file, "    push %d\n", root->num);
    } 
    else if (root->label == Char) {
        if (strcmp(root->ident, "'\\n'") == 0) { 
            fprintf(file, "    push 10\n");
        } else if (strcmp(root->ident, "'\\t'") == 0) {
            fprintf(file, "    push 9\n");
        } else {
            fprintf(file, "    push %s\n", root->ident);
        }
    }
    else if (root->label == Ident) {
        fprintf(file, "    mov rax, [%s]\n", root->ident);
        fprintf(file, "    push rax\n");
    }
    else if (root->label == Affect) {
        parcours_reste(root->firstChild, file, globalSt);
        fprintf(file, "    pop rax\n");
        fprintf(file, "    mov [%s], rax\n", root->ident);
    }
    else if (root->label == Funct) {
        if (strcmp(root->ident, "getint") == 0) {
            fprintf(file, "    call getint\n");
            fprintf(file, "    push rax\n"); 
        } else if (strcmp(root->ident, "putchar") == 0) {
            parcours_reste(root->firstChild, file, globalSt); 
            fprintf(file, "    pop rdi\n"); 
            fprintf(file, "    call putchar\n");
        }
    }
    else if (root->label == If) {
        static int if_count = 0;
        int current_if = if_count++;
        Node *condNode = root->firstChild;
        Node *doNode = condNode->nextSibling;
        Node *elseNode = doNode ? doNode->nextSibling : NULL;
        parcours_reste(condNode, file, globalSt);
        fprintf(file, "    pop rax\n");
        fprintf(file, "    cmp rax, 0\n");
        if (elseNode) {
            fprintf(file, "    je .L_else_%d\n", current_if);
        } else {
            fprintf(file, "    je .L_end_if_%d\n", current_if);
        }
        parcours_reste(doNode, file, globalSt);
        if (elseNode) {
            fprintf(file, "    jmp .L_end_if_%d\n", current_if);
            fprintf(file, ".L_else_%d:\n", current_if);
            parcours_reste(elseNode, file, globalSt);
        }
        fprintf(file, ".L_end_if_%d:\n", current_if);
    }
    else if (root->label == Cond || root->label == Do || root->label == Else) {
        for (Node *child = root->firstChild; child != NULL; child = child->nextSibling) {
            parcours_reste(child, file, globalSt);
        }
    }
    else {
        for (Node *child = root->firstChild; child != NULL; child = child->nextSibling) {
            parcours_reste(child, file, globalSt);
        }
    }
}

void parcours_depart(Node * root, FILE * file, SymbolTable * globalSt) {
    if(root == NULL) return;
    if (root->label == Prog) {
        fprintf(file, "section .bss\n");
        for (int i = 0; i < globalSt->nbSymbolInList; i++) {
            if (globalSt->symbolList[i].use == SYM_VAR) {
                fprintf(file, "    %s: resq 1\n", globalSt->symbolList[i].ident);
            }
        }
        fprintf(file, "\n");
    }
    if (root->label == DeclFonct) {
        Node * entete = root->firstChild;
        if (entete != NULL && entete->label == EnTeteFonct) {
            Node * type = entete->firstChild;
            Node * ident = type->nextSibling;
            
            if(ident != NULL && ident->label == Ident && strcmp(ident->ident, "main") == 0) {
                fprintf(file, "extern getint\n");
                fprintf(file, "extern putchar\n\n");
                fprintf(file, "global _start\n");
                fprintf(file, "section .text\n\n");
                fprintf(file, "_start:\n");
                for (Node *body = entete->nextSibling; body != NULL; body = body->nextSibling) {
                    parcours_reste(body, file, globalSt);
                }
                fprintf(file, "    pop rdi\n");
                fprintf(file, "    mov rax, 60\n");
                fprintf(file, "    syscall\n");
            }
        }
    } else {
        for (Node *child = root->firstChild; child != NULL; child = child->nextSibling) {
            parcours_depart(child, file, globalSt);
        }
    }
}