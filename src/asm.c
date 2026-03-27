#include "asm.h"
#include <string.h>

static void parcours_reste(Node * root, FILE * file) {
    if (root == NULL) return;
    if (root->label == Addsub) {
        parcours_reste(root->firstChild, file);
        parcours_reste(root->firstChild->nextSibling, file);
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
    else if (root->label == Num) {
        fprintf(file, "    push %d\n", root->num);
    } 
    else {
        for (Node *child = root->firstChild; child != NULL; child = child->nextSibling) {
            parcours_reste(child, file);
        }
    }
}

void parcours_depart(Node * root, FILE * file) {
    if(root == NULL) return;
    if (root->label == DeclFonct) {
        Node * entete = root->firstChild;
        if (entete != NULL && entete->label == EnTeteFonct) {
            Node * type = entete->firstChild;
            Node * ident = type->nextSibling;
            if(ident != NULL && ident->label == Ident && strcmp(ident->ident, "main") == 0) {
                fprintf(file, "global _start\n");
                fprintf(file, "section .text\n\n");
                fprintf(file, "_start:\n");
                for (Node *body = entete->nextSibling; body != NULL; body = body->nextSibling) {
                    parcours_reste(body, file);
                }
                fprintf(file, "    mov rax, 60\n");
                fprintf(file, "    mov rdi, 0\n");
                fprintf(file, "    syscall\n");
            }
        }
    } else {
        for (Node *child = root->firstChild; child != NULL; child = child->nextSibling) {
            parcours_depart(child, file);
        }
    }
}