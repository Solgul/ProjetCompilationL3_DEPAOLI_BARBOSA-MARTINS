#include "asm.h"
#include <string.h>

static void parcours_reste(Node * root, FILE * file) {

}

void parcours_depart(Node * root, FILE * file) {
    if(root == NULL) { //end
        return ;
    }

    if (root->label == DeclFonct) {
        Node * fonc = root->firstChild;
        Node * type = fonc->firstChild;
        Node * ident = type->nextSibling;
        if(!strcmp(ident->ident, "main")) {
            fprintf(file, "global _start\nsection .text\n_start :\n");
            parcours_reste(root, file);
            fprintf(file, "mov rax, 60\nmov rdi, 0\nsyscall");
        }
    } else {
        for (Node *child = root->firstChild; child != NULL; child = child->nextSibling) {
            parcours_depart(child, file);
        }
    }
}