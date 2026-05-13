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
            Node *lvalue = root->firstChild;
            Node *expr = lvalue->nextSibling;
            parcours_reste(expr, file, globalSt, localSt);
            fprintf(file, "    pop rax\n"); 
            
            if (lvalue->label == Ident) {
                Symbol *s = lookupSymbol(globalSt, localSt, lvalue->ident);
                if (s) {
                    if ((s->use == SYM_VAR || s->use == SYM_PARAM) && localSt && findSymbol(localSt, lvalue->ident) != -1) {
                        fprintf(file, "    mov [rbp - %d], rax\n", s->offset);
                    } else {
                        fprintf(file, "    mov [%s], rax\n", lvalue->ident);
                    }
                }
            } 
            else if (lvalue->label == Dot) {
                Node *base = lvalue->firstChild;
                Node *champ = base->nextSibling;
                Symbol *sBase = lookupSymbol(globalSt, localSt, base->ident);
                if (sBase) {
                    int offset_champ = 0;
                    if (sBase->nameStruct) {
                        for (int i = 0; i < sBase->nameStruct->nbChamp; i++) {
                            if (strcmp(sBase->nameStruct->champs[i].ident, champ->ident) == 0) {
                                offset_champ = i * 8;
                                break;
                            }
                        }
                    }
                    if ((sBase->use == SYM_VAR || sBase->use == SYM_PARAM) && localSt && findSymbol(localSt, base->ident) != -1) {
                        fprintf(file, "    mov [rbp - %d], rax\n", sBase->offset - offset_champ);
                    } else {
                        fprintf(file, "    mov [%s + %d], rax\n", base->ident, offset_champ);
                    }
                }
            }
            fprintf(file, "    push rax\n"); 
            break;
        }
        case Addsub: {
            parcours_reste(root->firstChild, file, globalSt, localSt); 
            parcours_reste(root->firstChild->nextSibling, file, globalSt, localSt);
            fprintf(file, "    pop rbx\n"); 
            fprintf(file, "    pop rax\n");
            if (root->ident[0] == '+') {
                fprintf(file, "    add rax, rbx\n");
            } else {
                fprintf(file, "    sub rax, rbx\n");
            }
            fprintf(file, "    push rax\n");
            break;
        }
        case Divstar: {
            parcours_reste(root->firstChild, file, globalSt, localSt);
            parcours_reste(root->firstChild->nextSibling, file, globalSt, localSt);
            fprintf(file, "    pop rbx\n"); 
            fprintf(file, "    pop rax\n");
            if (root->ident[0] == '*') {
                fprintf(file, "    imul rax, rbx\n");
            } else if (root->ident[0] == '/') {
                fprintf(file, "    cqo\n"); 
                fprintf(file, "    idiv rbx\n");
            } else if (root->ident[0] == '%') {
                fprintf(file, "    cqo\n");
                fprintf(file, "    idiv rbx\n");
                fprintf(file, "    mov rax, rdx\n"); 
            }
            fprintf(file, "    push rax\n");
            break;
        }
        case Equal: {
            parcours_reste(root->firstChild, file, globalSt, localSt);
            parcours_reste(root->firstChild->nextSibling, file, globalSt, localSt);
            fprintf(file, "    pop rbx\n");
            fprintf(file, "    pop rax\n");
            fprintf(file, "    cmp rax, rbx\n");
            if (strcmp(root->ident, "==") == 0) {
                fprintf(file, "    sete al\n");
            } else { 
                fprintf(file, "    setne al\n");
            }
            fprintf(file, "    movzx rax, al\n");
            fprintf(file, "    push rax\n");
            break;
        }
        case Order: {
            parcours_reste(root->firstChild, file, globalSt, localSt);
            parcours_reste(root->firstChild->nextSibling, file, globalSt, localSt);
            fprintf(file, "    pop rbx\n");
            fprintf(file, "    pop rax\n");
            fprintf(file, "    cmp rax, rbx\n");
            if (strcmp(root->ident, "<") == 0) fprintf(file, "    setl al\n");
            else if (strcmp(root->ident, "<=") == 0) fprintf(file, "    setle al\n");
            else if (strcmp(root->ident, ">") == 0) fprintf(file, "    setg al\n");
            else if (strcmp(root->ident, ">=") == 0) fprintf(file, "    setge al\n");
            fprintf(file, "    movzx rax, al\n");
            fprintf(file, "    push rax\n");
            break;
        }
        case While: {
            int w = while_count++;
            fprintf(file, ".L_while_start_%d:\n", w);
            parcours_reste(root->firstChild, file, globalSt, localSt); 
            fprintf(file, "    pop rax\n");
            fprintf(file, "    cmp rax, 0\n");
            fprintf(file, "    je .L_while_end_%d\n", w);
            parcours_reste(root->firstChild->nextSibling, file, globalSt, localSt);
            fprintf(file, "    jmp .L_while_start_%d\n", w);
            fprintf(file, ".L_while_end_%d:\n", w);
            break;
        }
        case If: {
            int i = if_count++;
            parcours_reste(root->firstChild, file, globalSt, localSt);
            fprintf(file, "    pop rax\n");
            fprintf(file, "    cmp rax, 0\n");
            Node *elseNode = root->firstChild->nextSibling->nextSibling;
            if (elseNode) {
                fprintf(file, "    je .L_if_false_%d\n", i);
                parcours_reste(root->firstChild->nextSibling, file, globalSt, localSt);
                fprintf(file, "    jmp .L_if_end_%d\n", i);
                fprintf(file, ".L_if_false_%d:\n", i);
                parcours_reste(elseNode, file, globalSt, localSt);
            } else { 
                fprintf(file, "    je .L_if_end_%d\n", i);
                parcours_reste(root->firstChild->nextSibling, file, globalSt, localSt);
            }
            fprintf(file, ".L_if_end_%d:\n", i);
            break;
        }
        case And: {
            int c = if_count++;
            parcours_reste(root->firstChild, file, globalSt, localSt);
            fprintf(file, "    pop rax\n");
            fprintf(file, "    cmp rax, 0\n");
            fprintf(file, "    je .L_and_false_%d\n", c);
            parcours_reste(root->firstChild->nextSibling, file, globalSt, localSt);
            fprintf(file, "    pop rax\n");
            fprintf(file, ".L_and_false_%d:\n", c);
            fprintf(file, "    push rax\n");
            break;
        }
        case Or: {
            int c = if_count++;
            parcours_reste(root->firstChild, file, globalSt, localSt);
            fprintf(file, "    pop rax\n");
            fprintf(file, "    cmp rax, 0\n");
            fprintf(file, "    jne .L_or_true_%d\n", c);
            parcours_reste(root->firstChild->nextSibling, file, globalSt, localSt);
            fprintf(file, "    pop rax\n");
            fprintf(file, ".L_or_true_%d:\n", c);
            fprintf(file, "    push rax\n");
            break;
        }
        case Funct: { 
            int arg_count = 0;
            Node *arg = root->firstChild;
            while (arg) {
                parcours_reste(arg, file, globalSt, localSt);
                arg_count++;
                arg = arg->nextSibling;
            }
            for (int j = arg_count - 1; j >= 0; j--) {
                if (j < 6) {
                    fprintf(file, "    pop %s\n", amd64_regs[j]);
                }
            }
            fprintf(file, "    mov rbx, rsp\n");
            fprintf(file, "    and rsp, -16\n");
            fprintf(file, "    call %s\n", root->ident);
            fprintf(file, "    mov rsp, rbx\n");
            if (arg_count > 6) {
                fprintf(file, "    add rsp, %d\n", (arg_count - 6) * 8);
            }
            fprintf(file, "    push rax\n");
            break;
        }
        case Return: {
            if (root->firstChild) {
                parcours_reste(root->firstChild, file, globalSt, localSt);
                fprintf(file, "    pop rax\n");
            }
            fprintf(file, "    mov rsp, rbp\n");
            fprintf(file, "    pop rbp\n");
            fprintf(file, "    ret\n");
            break;
        }
        case Dot: {
            Node *base = root->firstChild;
            Node *champ = base->nextSibling;
            Symbol *sBase = lookupSymbol(globalSt, localSt, base->ident);
            if (!sBase) break;
            int offset_champ = 0;
            if (sBase->nameStruct) {
                for (int i = 0; i < sBase->nameStruct->nbChamp; i++) {
                    if (strcmp(sBase->nameStruct->champs[i].ident, champ->ident) == 0) {
                        offset_champ = i * 8;
                        break;
                    }
                }
            }
            if ((sBase->use == SYM_VAR || sBase->use == SYM_PARAM) && localSt && findSymbol(localSt, base->ident) != -1) {
                fprintf(file, "    mov rax, [rbp - %d]\n", sBase->offset - offset_champ);
            } else {
                fprintf(file, "    mov rax, [%s + %d]\n", base->ident, offset_champ);
            }
            fprintf(file, "    push rax\n");
            break;
        }
        case UnOperator: {
            parcours_reste(root->firstChild, file, globalSt, localSt);
            fprintf(file, "    pop rax\n");
            if (root->ident[0] == '-') {
                fprintf(file, "    neg rax\n");
            }
            fprintf(file, "    push rax\n");
            break;
        }
        case Negate: {
            parcours_reste(root->firstChild, file, globalSt, localSt);
            fprintf(file, "    pop rax\n");
            fprintf(file, "    cmp rax, 0\n");
            fprintf(file, "    sete al\n");
            fprintf(file, "    movzx rax, al\n");
            fprintf(file, "    push rax\n");
            break;
        }
        default:
            for (Node *child = root->firstChild; child != NULL; child = child->nextSibling) {
                parcours_reste(child, file, globalSt, localSt);
                if (child->label == Affect || child->label == Funct) {
                    fprintf(file, "    add rsp, 8\n");
                }
            }
            break;
    }
}

void parcours_depart(Node * root, FILE * file, SymbolTable * globalSt) {
    if (root == NULL) return;
    if (root->label == Prog) {
        fprintf(file, "section .bss\n");
        for (int i = 0; i < globalSt->nbSymbolInList; i++) {
            if (globalSt->symbolList[i].use == SYM_VAR) {
                int size = 1;
                if (globalSt->symbolList[i].type == TYPE_STRUCT && globalSt->symbolList[i].nameStruct) {
                    size = globalSt->symbolList[i].nameStruct->nbChamp;
                }
                fprintf(file, "    %s: resq %d\n", globalSt->symbolList[i].ident, size);
            }
        }
        fprintf(file, "\nsection .text\nextern getint, putchar, putint, getchar\n\n");
    }
    if (root->label == DeclFonct) {
        Node *enTete = root->firstChild;
        Node *ident = enTete->firstChild->nextSibling;
        int funcIdx = findSymbol(globalSt, ident->ident);
        SymbolTable *localSt = globalSt->symbolList[funcIdx].child;
        if (strcmp(ident->ident, "main") == 0) {
            fprintf(file, "global _start\n_start:\n");
            Node *instr = enTete->nextSibling;
            while (instr != NULL) {
                parcours_reste(instr, file, globalSt, localSt);
                if (instr->label == Affect || instr->label == Funct) {
                    fprintf(file, "    add rsp, 8\n"); 
                }
                instr = instr->nextSibling;
            }    
            fprintf(file, "    pop rdi\n    mov rax, 60\n    syscall\n");       
        } else {
            fprintf(file, "%s:\n    push rbp\n    mov rbp, rsp\n", ident->ident);
            int stack_size = 0;
            for (int i = 0; i < localSt->nbSymbolInList; i++) {
                if (localSt->symbolList[i].offset > stack_size) {
                    stack_size = localSt->symbolList[i].offset;
                }
            }
            if (stack_size % 16 != 0) {
                stack_size += 16 - (stack_size % 16);
            }
            if (stack_size == 0) stack_size = 16;
            fprintf(file, "    sub rsp, %d\n", stack_size);
            int p_idx = 0;
            for (int i = 0; i < localSt->nbSymbolInList; i++) {
                if (localSt->symbolList[i].use == SYM_PARAM && p_idx < 6) {
                    fprintf(file, "    mov [rbp - %d], %s\n", localSt->symbolList[i].offset, amd64_regs[p_idx++]);
                }
            }
            Node *instr = enTete->nextSibling;
            while (instr != NULL) {
                parcours_reste(instr, file, globalSt, localSt);
                if (instr->label == Affect || instr->label == Funct) {
                    fprintf(file, "    add rsp, 8\n"); 
                }
                instr = instr->nextSibling;
            }
            fprintf(file, "    ; --- Epilogue de securite implicite ---\n");
            fprintf(file, "    mov rsp, rbp\n");
            fprintf(file, "    pop rbp\n");
            fprintf(file, "    ret\n\n");
        }
    } else {
        for (Node *child = root->firstChild; child != NULL; child = child->nextSibling)
            parcours_depart(child, file, globalSt);
    }
}