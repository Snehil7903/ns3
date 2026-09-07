#include <stdio.h>

#define N 9

int main() {
    // The terminals in our grammar
    // Using '^' to represent the up-arrow '↑'
    char *ops[N] = {"+", "-", "*", "/", "^", "(", ")", "id", "$"};
    
    // Operator Precedence Table based on LEADING and TRAILING sets
    // '>' means row > col, '<' means row < col, '=' means equal, ' ' means error
    char table[N][N] = {
      /*       +    -    *    /    ^    (    )   id    $  */
      /* + */ {'>', '>', '<', '<', '<', '<', '>', '<', '>'},
      /* - */ {'>', '>', '<', '<', '<', '<', '>', '<', '>'},
      /* * */ {'>', '>', '>', '>', '<', '<', '>', '<', '>'},
      /* / */ {'>', '>', '>', '>', '<', '<', '>', '<', '>'},
      /* ^ */ {'>', '>', '>', '>', '<', '<', '>', '<', '>'}, 
      /* ( */ {'<', '<', '<', '<', '<', '<', '=', '<', ' '},
      /* ) */ {'>', '>', '>', '>', '>', ' ', '>', ' ', '>'},
      /* id*/ {'>', '>', '>', '>', '>', ' ', '>', ' ', '>'},
      /* $ */ {'<', '<', '<', '<', '<', '<', ' ', '<', ' '}
    };

    int f[N] = {0};
    int g[N] = {0};
    int i, j, k;
    int changed;

    // Algorithm to calculate precedence functions (Repeated Relaxation)
    for (k = 0; k < N * N; k++) {
        changed = 0;
        for (i = 0; i < N; i++) {
            for (j = 0; j < N; j++) {
                if (table[i][j] == '<') {
                    if (g[j] < f[i] + 1) {
                        g[j] = f[i] + 1;
                        changed = 1;
                    }
                }
                else if (table[i][j] == '>') {
                    if (f[i] < g[j] + 1) {
                        f[i] = g[j] + 1;
                        changed = 1;
                    }
                }
                else if (table[i][j] == '=') {
                    if (f[i] < g[j]) {
                        f[i] = g[j];
                        changed = 1;
                    }
                    else if (g[j] < f[i]) {
                        g[j] = f[i];
                        changed = 1;
                    }
                }
            }
        }
        if (!changed) break; // Break early if functions are stable
    }

    // Outputting the Precedence Table
    printf("--- Operator Precedence Table ---\n");
    printf("    ");
    for (i = 0; i < N; i++) printf("%-4s", ops[i]);
    printf("\n");
    for (i = 0; i < N; i++) {
        printf("%-4s", ops[i]);
        for (j = 0; j < N; j++) {
            printf("%-4c", table[i][j]);
        }
        printf("\n");
    }

    // Outputting the Computed Precedence Functions
    printf("\n--- Table of Precedence Functions ---\n");
    printf("-------------------------------------\n");
    printf("   Operator      f          g\n");
    printf("-------------------------------------\n");
    for (i = 0; i < N; i++) {
        printf("      %-6s     %-4d       %-4d\n", ops[i], f[i], g[i]);
    }
    printf("-------------------------------------\n");

    return 0;
}
