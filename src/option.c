#include "option.h"

extern int yyparse();
extern FILE *yyin;
extern int yylineno;
extern Node *root = NULL; 

void print_help(const char *exeName) {
    printf("Usage: %s [OPTIONS] [FILE.tpc]\n", exeName);
    printf("Options:\n");
    printf("  -t, --tree    Affiche l'arbre abstrait sur la sortie standard\n");
    printf("  -h, --help \n");
    printf("\n");
}

int handle_opt(int argc, char *argv[]) {
    int opt;
    int flag_tree = 0;
    static struct option long_options[] = {
        {"tree", no_argument, 0, 't'},
        {"help", no_argument, 0, 'h'},
        {0, 0, 0, 0}
    };
    while ((opt = getopt_long(argc, argv, "th", long_options, NULL)) != -1) {
        switch (opt) {
        case 't':
            flag_tree = 1;
            break;
        case 'h':
            print_help(argv[0]);
            return 0;
        default:
            fprintf(stderr, "Option incorrecte.\n");
            print_help(argv[0]);
            return 2;
        }
    }
    if (optind < argc) {
        yyin = fopen(argv[optind], "r");
        if (!yyin) {
            fprintf(stderr, "Erreur: Impossible d'ouvrir le fichier %s\n", argv[optind]);
            return 2;
        }
    } else {
        yyin = stdin; 
    }
    int parse_result = yyparse();
    if (parse_result == 0) {
        if (flag_tree && root != NULL) {
            printTree(root);
        }
        return 0;
    } else {
        return 1;
    }
}