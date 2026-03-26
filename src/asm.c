#include "asm.h"
#include <stdio.h>

void parcours_pour_fonction(Node * root) {
    if(root == NULL) { //end
        return ;
    }

    printf("%s\n", root->ident);

    if (root->label == DeclFonct) {
        Node * fonc = root->firstChild;
        for (; fonc->label != Ident; fonc = fonc->nextSibling);
        printf("%s\n", fonc->ident);
    } else {
        while (root->nextSibling != NULL) {
            parcours_pour_fonction(root->nextSibling);
        }
    }
}