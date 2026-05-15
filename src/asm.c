#include "asm.h"
#include <string.h>
#include <stdlib.h>

static int label_id = 0;
static const char *amd64_regs[] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};

static void asm_evaluate_expression(Node *n, FILE *out, SymbolTable *globalSt, SymbolTable *localSt);
static void asm_generate_statement(Node *n, FILE *out, SymbolTable *globalSt, SymbolTable *localSt);
static void asm_generate_block(Node *n, FILE *out, SymbolTable *globalSt, SymbolTable *localSt);

static int is_local(SymbolTable *localSt, const char *ident) {
    return localSt && findSymbol(localSt, (char *)ident) != -1;
}

static int count_args(Node *arg) {
    int n = 0;
    for (; arg; arg = arg->nextSibling) n++;
    return n;
}

static Node *arg_at(Node *arg, int index) {
    for (int i = 0; arg && i < index; i++) arg = arg->nextSibling;
    return arg;
}

static void asm_load_variable(Node *n, FILE *out, SymbolTable *globalSt, SymbolTable *localSt) {
    Symbol *s = lookupSymbol(globalSt, localSt, n->ident);
    if (s && (s->use == SYM_VAR || s->use == SYM_PARAM) && is_local(localSt, n->ident)) {
        fprintf(out, "    mov rax, qword [rbp - %d]\n", s->offset);
    } else {
        fprintf(out, "    mov rax, qword [%s]\n", n->ident);
    }
    fprintf(out, "    push rax\n");
}

static void asm_store_variable(Node *n, FILE *out, SymbolTable *globalSt, SymbolTable *localSt) {
    Symbol *s = lookupSymbol(globalSt, localSt, n->ident);
    if (s && (s->use == SYM_VAR || s->use == SYM_PARAM) && is_local(localSt, n->ident)) {
        fprintf(out, "    mov qword [rbp - %d], rax\n", s->offset);
    } else {
        fprintf(out, "    mov qword [%s], rax\n", n->ident);
    }
}

static int char_value(const char *s) {
    if (!s || s[0] != '\'') return 0;
    if (s[1] == '\\') {
        switch (s[2]) {
            case 'n': return 10;
            case 't': return 9;
            case '\\': return '\\';
            case '\'': return '\'';
            default: return (unsigned char)s[2];
        }
    }
    return (unsigned char)s[1];
}

static void asm_generate_function_call(Node *n, FILE *out, SymbolTable *globalSt, SymbolTable *localSt) {
    int argc = count_args(n->firstChild);
    int stack_args = argc > 6 ? argc - 6 : 0;
    int pad = stack_args % 2;

    if (pad) fprintf(out, "    sub rsp, 8\n");

    for (int i = argc - 1; i >= 6; i--) {
        asm_evaluate_expression(arg_at(n->firstChild, i), out, globalSt, localSt);
        fprintf(out, "    pop rax\n");
        fprintf(out, "    push rax\n");
    }

    int reg_args = argc < 6 ? argc : 6;
    for (int i = 0; i < reg_args; i++) {
        asm_evaluate_expression(arg_at(n->firstChild, i), out, globalSt, localSt);
    }
    for (int i = reg_args - 1; i >= 0; i--) {
        fprintf(out, "    pop %s\n", amd64_regs[i]);
    }

    fprintf(out, "    call %s\n", n->ident);
    if (stack_args || pad) fprintf(out, "    add rsp, %d\n", stack_args * 8 + pad * 8);
    fprintf(out, "    push rax\n");
}

static void asm_evaluate_expression(Node *n, FILE *out, SymbolTable *globalSt, SymbolTable *localSt) {
    if (!n) { fprintf(out, "    push 0\n"); return; }
    switch (n->label) {
        case Num:
            fprintf(out, "    push %d\n", n->num);
            break;
        case Char:
            fprintf(out, "    push %d\n", char_value(n->ident));
            break;
        case Ident:
            asm_load_variable(n, out, globalSt, localSt);
            break;
        case Dot: {
            Node *base = n->firstChild;
            Node *field = base ? base->nextSibling : NULL;
            Symbol *s = base ? lookupSymbol(globalSt, localSt, base->ident) : NULL;
            int off = 0;
            if (s && s->nameStruct && field) {
                for (int i = 0; i < s->nameStruct->nbChamp; i++) {
                    if (strcmp(s->nameStruct->champs[i].ident, field->ident) == 0) { off = i * 8; break; }
                }
            }
            if (s && is_local(localSt, base->ident)) fprintf(out, "    mov rax, qword [rbp - %d]\n", s->offset - off);
            else if (base) fprintf(out, "    mov rax, qword [%s + %d]\n", base->ident, off);
            else fprintf(out, "    xor rax, rax\n");
            fprintf(out, "    push rax\n");
            break;
        }
        case Addsub:
            asm_evaluate_expression(n->firstChild, out, globalSt, localSt);
            asm_evaluate_expression(n->firstChild->nextSibling, out, globalSt, localSt);
            fprintf(out, "    pop rbx\n    pop rax\n");
            fprintf(out, n->ident[0] == '+' ? "    add rax, rbx\n" : "    sub rax, rbx\n");
            fprintf(out, "    push rax\n");
            break;
        case Divstar:
            asm_evaluate_expression(n->firstChild, out, globalSt, localSt);
            asm_evaluate_expression(n->firstChild->nextSibling, out, globalSt, localSt);
            fprintf(out, "    pop rbx\n    pop rax\n");
            if (n->ident[0] == '*') fprintf(out, "    imul rax, rbx\n");
            else {
                fprintf(out, "    cqo\n    idiv rbx\n");
                if (n->ident[0] == '%') fprintf(out, "    mov rax, rdx\n");
            }
            fprintf(out, "    push rax\n");
            break;
        case Equal:
        case Order:
            asm_evaluate_expression(n->firstChild, out, globalSt, localSt);
            asm_evaluate_expression(n->firstChild->nextSibling, out, globalSt, localSt);
            fprintf(out, "    pop rbx\n    pop rax\n    cmp rax, rbx\n");
            if (n->label == Equal) fprintf(out, strcmp(n->ident, "==") == 0 ? "    sete al\n" : "    setne al\n");
            else if (strcmp(n->ident, "<") == 0) fprintf(out, "    setl al\n");
            else if (strcmp(n->ident, "<=") == 0) fprintf(out, "    setle al\n");
            else if (strcmp(n->ident, ">") == 0) fprintf(out, "    setg al\n");
            else fprintf(out, "    setge al\n");
            fprintf(out, "    movzx rax, al\n    push rax\n");
            break;
        case And: {
            int lfalse = label_id++, lend = label_id++;
            asm_evaluate_expression(n->firstChild, out, globalSt, localSt);
            fprintf(out, "    pop rax\n    cmp rax, 0\n    je .Lfalse_%d\n", lfalse);
            asm_evaluate_expression(n->firstChild->nextSibling, out, globalSt, localSt);
            fprintf(out, "    pop rax\n    cmp rax, 0\n    je .Lfalse_%d\n", lfalse);
            fprintf(out, "    mov rax, 1\n    jmp .Lend_%d\n.Lfalse_%d:\n    xor rax, rax\n.Lend_%d:\n    push rax\n", lend, lfalse, lend);
            break;
        }
        case Or: {
            int ltrue = label_id++, lend = label_id++;
            asm_evaluate_expression(n->firstChild, out, globalSt, localSt);
            fprintf(out, "    pop rax\n    cmp rax, 0\n    jne .Ltrue_%d\n", ltrue);
            asm_evaluate_expression(n->firstChild->nextSibling, out, globalSt, localSt);
            fprintf(out, "    pop rax\n    cmp rax, 0\n    jne .Ltrue_%d\n", ltrue);
            fprintf(out, "    xor rax, rax\n    jmp .Lend_%d\n.Ltrue_%d:\n    mov rax, 1\n.Lend_%d:\n    push rax\n", lend, ltrue, lend);
            break;
        }
        case UnOperator:
            asm_evaluate_expression(n->firstChild, out, globalSt, localSt);
            fprintf(out, "    pop rax\n");
            if (n->ident[0] == '-') fprintf(out, "    neg rax\n");
            fprintf(out, "    push rax\n");
            break;
        case Negate:
            asm_evaluate_expression(n->firstChild, out, globalSt, localSt);
            fprintf(out, "    pop rax\n    cmp rax, 0\n    sete al\n    movzx rax, al\n    push rax\n");
            break;
        case Affect:
            asm_generate_statement(n, out, globalSt, localSt);
            break;
        case Funct:
            asm_generate_function_call(n, out, globalSt, localSt);
            break;
        case Cond:
        case Do:
        case Else:
        case Instr:
            asm_evaluate_expression(n->firstChild, out, globalSt, localSt);
            break;
        default:
            fprintf(out, "    push 0\n");
            break;
    }
}

static void asm_generate_return(Node *n, FILE *out, SymbolTable *globalSt, SymbolTable *localSt) {
    if (n->firstChild) {
        asm_evaluate_expression(n->firstChild, out, globalSt, localSt);
        fprintf(out, "    pop rax\n");
    } else {
        fprintf(out, "    xor rax, rax\n");
    }
    fprintf(out, "    mov rsp, rbp\n    pop rbp\n    ret\n");
}

static void asm_generate_statement(Node *n, FILE *out, SymbolTable *globalSt, SymbolTable *localSt) {
    if (!n) return;
    switch (n->label) {
        case Instr:
        case Do:
        case Else:
            asm_generate_block(n->firstChild, out, globalSt, localSt);
            break;
        case Affect: {
            Node *lhs = n->firstChild;
            Node *rhs = lhs ? lhs->nextSibling : NULL;
            asm_evaluate_expression(rhs, out, globalSt, localSt);
            fprintf(out, "    pop rax\n");
            if (lhs && lhs->label == Ident) asm_store_variable(lhs, out, globalSt, localSt);
            else if (lhs && lhs->label == Dot) {
                Node *base = lhs->firstChild;
                Node *field = base ? base->nextSibling : NULL;
                Symbol *s = base ? lookupSymbol(globalSt, localSt, base->ident) : NULL;
                int off = 0;
                if (s && s->nameStruct && field) {
                    for (int i = 0; i < s->nameStruct->nbChamp; i++) {
                        if (strcmp(s->nameStruct->champs[i].ident, field->ident) == 0) { off = i * 8; break; }
                    }
                }
                if (s && is_local(localSt, base->ident)) fprintf(out, "    mov qword [rbp - %d], rax\n", s->offset - off);
                else if (base) fprintf(out, "    mov qword [%s + %d], rax\n", base->ident, off);
            }
            break;
        }
        case Funct:
            asm_generate_function_call(n, out, globalSt, localSt);
            fprintf(out, "    add rsp, 8\n");
            break;
        case Return:
            asm_generate_return(n, out, globalSt, localSt);
            break;
        case If: {
            int id = label_id++;
            Node *condWrap = n->firstChild;
            Node *thenWrap = condWrap ? condWrap->nextSibling : NULL;
            Node *elseWrap = thenWrap ? thenWrap->nextSibling : NULL;
            asm_evaluate_expression(condWrap ? condWrap->firstChild : NULL, out, globalSt, localSt);
            fprintf(out, "    pop rax\n    cmp rax, 0\n");
            if (elseWrap) {
                fprintf(out, "    je .Lelse_%d\n", id);
                asm_generate_statement(thenWrap, out, globalSt, localSt);
                fprintf(out, "    jmp .Lendif_%d\n.Lelse_%d:\n", id, id);
                asm_generate_statement(elseWrap, out, globalSt, localSt);
                fprintf(out, ".Lendif_%d:\n", id);
            } else {
                fprintf(out, "    je .Lendif_%d\n", id);
                asm_generate_statement(thenWrap, out, globalSt, localSt);
                fprintf(out, ".Lendif_%d:\n", id);
            }
            break;
        }
        case While: {
            int id = label_id++;
            Node *condWrap = n->firstChild;
            Node *bodyWrap = condWrap ? condWrap->nextSibling : NULL;
            fprintf(out, ".Lwhile_%d:\n", id);
            asm_evaluate_expression(condWrap ? condWrap->firstChild : NULL, out, globalSt, localSt);
            fprintf(out, "    pop rax\n    cmp rax, 0\n    je .Lendwhile_%d\n", id);
            asm_generate_statement(bodyWrap, out, globalSt, localSt);
            fprintf(out, "    jmp .Lwhile_%d\n.Lendwhile_%d:\n", id, id);
            break;
        }
        default:
            asm_generate_block(n->firstChild, out, globalSt, localSt);
            break;
    }
}

static void asm_generate_block(Node *n, FILE *out, SymbolTable *globalSt, SymbolTable *localSt) {
    for (Node *cur = n; cur; cur = cur->nextSibling) asm_generate_statement(cur, out, globalSt, localSt);
}

static int frame_size(SymbolTable *localSt) {
    int max = 0;
    if (!localSt) return 0;
    for (int i = 0; i < localSt->nbSymbolInList; i++) {
        if ((localSt->symbolList[i].use == SYM_VAR || localSt->symbolList[i].use == SYM_PARAM) && localSt->symbolList[i].offset > max)
            max = localSt->symbolList[i].offset;
    }
    if (max % 16) max += 16 - (max % 16);
    return max;
}

static void asm_inject_runtime_lib(FILE *out) {
    fprintf(out,
"\n; ---------------- Fonctions predefinies TPC ----------------\n"
"putchar:\n"
"    push rbp\n    mov rbp, rsp\n    sub rsp, 16\n"
"    mov byte [rbp-1], dil\n"
"    mov rax, 1\n    mov rdi, 1\n    lea rsi, [rbp-1]\n    mov rdx, 1\n    syscall\n"
"    xor rax, rax\n    mov rsp, rbp\n    pop rbp\n    ret\n\n"
"getchar:\n"
"    push rbp\n    mov rbp, rsp\n    sub rsp, 16\n"
"    xor rax, rax\n    xor rdi, rdi\n    lea rsi, [rbp-1]\n    mov rdx, 1\n    syscall\n"
"    cmp rax, 1\n    jne io_error\n"
"    movzx rax, byte [rbp-1]\n    mov rsp, rbp\n    pop rbp\n    ret\n\n"
"putint:\n"
"    push rbp\n    mov rbp, rsp\n    sub rsp, 64\n"
"    mov rax, rdi\n    lea rsi, [rbp-1]\n    mov byte [rsi], 10\n    mov rcx, 1\n"
"    cmp rax, 0\n    jge .putint_abs\n    neg rax\n    mov r8, 1\n    jmp .putint_loop\n.putint_abs:\n    xor r8, r8\n"
".putint_loop:\n    xor rdx, rdx\n    mov rbx, 10\n    div rbx\n    add dl, '0'\n    dec rsi\n    mov [rsi], dl\n    inc rcx\n    test rax, rax\n    jne .putint_loop\n"
"    cmp r8, 0\n    je .putint_write\n    dec rsi\n    mov byte [rsi], '-'\n    inc rcx\n"
".putint_write:\n    mov rax, 1\n    mov rdi, 1\n    mov rdx, rcx\n    syscall\n    xor rax, rax\n    mov rsp, rbp\n    pop rbp\n    ret\n\n"
"getint:\n"
"    push rbp\n    mov rbp, rsp\n    sub rsp, 32\n"
"    xor r9, r9\n    mov r10, 1\n"
".getint_read_first:\n    xor rax, rax\n    xor rdi, rdi\n    lea rsi, [rbp-1]\n    mov rdx, 1\n    syscall\n    cmp rax, 1\n    jne io_error\n    movzx rbx, byte [rbp-1]\n"
"    cmp bl, '-'\n    jne .getint_plus\n    mov r10, -1\n    jmp .getint_read_digit\n"
".getint_plus:\n    cmp bl, '+'\n    jne .getint_digit_got\n"
".getint_read_digit:\n    xor rax, rax\n    xor rdi, rdi\n    lea rsi, [rbp-1]\n    mov rdx, 1\n    syscall\n    cmp rax, 1\n    jne io_error\n    movzx rbx, byte [rbp-1]\n"
".getint_digit_got:\n    cmp bl, '0'\n    jl io_error\n    cmp bl, '9'\n    jg io_error\n"
".getint_digits:\n    imul r9, r9, 10\n    sub bl, '0'\n    movzx rbx, bl\n    add r9, rbx\n"
"    xor rax, rax\n    xor rdi, rdi\n    lea rsi, [rbp-1]\n    mov rdx, 1\n    syscall\n    cmp rax, 1\n    jne io_error\n    movzx rbx, byte [rbp-1]\n    cmp bl, 10\n    je .getint_done\n    cmp bl, '0'\n    jl io_error\n    cmp bl, '9'\n    jle .getint_digits\n    jmp io_error\n"
".getint_done:\n    mov rax, r9\n    cmp r10, 1\n    je .getint_ret\n    neg rax\n.getint_ret:\n    mov rsp, rbp\n    pop rbp\n    ret\n"
"io_error:\n    mov rax, 60\n    mov rdi, 5\n    syscall\n");
}

static void asm_generate_function(Node *decl, FILE *out, SymbolTable *globalSt) {
    Node *head = decl->firstChild;
    Node *name = head->firstChild->nextSibling;
    int idx = findSymbol(globalSt, name->ident);
    SymbolTable *localSt = idx >= 0 ? globalSt->symbolList[idx].child : NULL;
    int size = frame_size(localSt);

    fprintf(out, "%s:\n    push rbp\n    mov rbp, rsp\n", name->ident);
    if (size > 0) fprintf(out, "    sub rsp, %d\n", size);

    int p = 0;
    if (localSt) {
        for (int i = 0; i < localSt->nbSymbolInList; i++) {
            if (localSt->symbolList[i].use == SYM_PARAM) {
                if (p < 6) fprintf(out, "    mov qword [rbp - %d], %s\n", localSt->symbolList[i].offset, amd64_regs[p]);
                else fprintf(out, "    mov rax, qword [rbp + %d]\n    mov qword [rbp - %d], rax\n", 16 + (p - 6) * 8, localSt->symbolList[i].offset);
                p++;
            }
        }
    }

    Node *body = head->nextSibling;
    if (body) {
        Node *instrWrap = body->nextSibling;
        if (instrWrap && instrWrap->label == Instr) asm_generate_block(instrWrap->firstChild, out, globalSt, localSt);
    }
    fprintf(out, "    xor rax, rax\n    mov rsp, rbp\n    pop rbp\n    ret\n\n");
}

void parcours_depart(Node *root, FILE *out, SymbolTable *globalSt) {
    if (!root) return;
    fprintf(out, "section .bss\n");
    for (int i = 0; i < globalSt->nbSymbolInList; i++) {
        Symbol *s = &globalSt->symbolList[i];
        if (s->use == SYM_VAR) {
            int qwords = 1;
            if (s->type == TYPE_STRUCT && s->nameStruct) qwords = s->nameStruct->nbChamp > 0 ? s->nameStruct->nbChamp : 1;
            fprintf(out, "    %s: resq %d\n", s->ident, qwords);
        }
    }
    fprintf(out, "\nsection .text\nglobal _start\n_start:\n    call main\n    mov rdi, rax\n    mov rax, 60\n    syscall\n\n");

    for (Node *child = root->firstChild; child; child = child->nextSibling) {
        if (child->label == DeclFonct) asm_generate_function(child, out, globalSt);
    }
    asm_inject_runtime_lib(out);
}
