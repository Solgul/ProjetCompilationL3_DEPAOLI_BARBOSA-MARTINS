#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <string.h>
#include "tree.h"
#include "sym.h"
#include "asm.h"

extern int yyparse();
extern FILE *yyin;
extern Node *root;
int print_tree_flag = 0;
int print_symtabs_flag = 0;
char *input_filename = NULL;

void print_help(const char *exeName) {
    printf("Usage: %s [OPTIONS] [FILE.tpc]\n", exeName);
    printf("Options:\n");
    printf("  -t, --tree       Affiche l'arbre abstrait sur la sortie standard\n");
    printf("  -s, --symtabs    Affiche la table des symboles sur la sortie standard\n");
    printf("  -h, --help       Affiche cette aide et quitte\n");
    printf("\n");
}

int handle_opt(int argc, char *argv[]) {
    int opt;
    static struct option long_options[] = {
        {"tree", no_argument, 0, 't'},
        {"symtabs", no_argument, 0, 's'},
        {"help", no_argument, 0, 'h'},
        {0, 0, 0, 0}
    };
    while ((opt = getopt_long(argc, argv, "ths", long_options, NULL)) != -1) {
        switch (opt) {
        case 't':
            print_tree_flag = 1;
            break;
        case 's':
            print_symtabs_flag = 1;
            break;
        case 'h':
            print_help(argv[0]);
            return 1;
        default:
            fprintf(stderr, "Option incorrecte.\n");
            print_help(argv[0]);
            return 2;
        }
    }
    if(optind < argc){
        input_filename = argv[optind];
        FILE *f = fopen(input_filename, "r");
        if(!f){
            fprintf(stderr, "Erreur : le fichier '%s' est invalide ou introuvable.\n", input_filename);
            return 2;
        }
        yyin = f;
    } else {
        yyin = stdin;
    }

    return 0;
}

int main(int argc, char **argv){
    int opt_status = handle_opt(argc, argv);
    if(opt_status == 1) return 0;
    if(opt_status == 2) return 3;
    int res = yyparse();
    if (res == 0 && root != NULL) {
        if (print_tree_flag) {
            printTree(root);
        }
        SymbolTable *globalTable = buildSymbolTable(root);
        if (print_symtabs_flag){
            printf("\n=== TABLE DES SYMBOLES GLOBALE ===\n");
            printSymbolTable(globalTable, 0); 
        }
        int semErrors = checkSemantics(root, globalTable);
        if (semErrors > 0) {
            res = 2;
        }
        char *out_filename = "asm/_anonymous.asm";
        char temp_filename[512];
        if (input_filename != NULL) {
            char *base_name = strrchr(input_filename, '/');
            if (base_name != NULL) {
                base_name++;
            } else {
                base_name = input_filename;
            }
            snprintf(temp_filename, 512, "asm/%s", base_name);
            char *dot = strrchr(temp_filename, '.');
            if (dot != NULL && strcmp(dot, ".tpc") == 0) {
                strcpy(dot, ".asm");
            } else {
                strcat(temp_filename, ".asm");
            }
            out_filename = temp_filename;
        }
        FILE * file = fopen(out_filename, "w+");
        if (!file) {
            fprintf(stderr, "Erreur : Impossible de créer le fichier %s\n", out_filename);
            deleteTree(root);
            return -1;
        }
        parcours_depart(root, file, globalTable);
        fclose(file);
    }
    if(yyin != stdin){
        fclose(yyin);
    }
    return res;
}