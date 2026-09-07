#include <stdio.h>
#include <string.h>

#define MAX 20

int main()
{
    int n, i, j, k;
    char op[MAX][10];
    char table[MAX][MAX];

    int f[MAX] = {0};
    int g[MAX] = {0};

    printf("Enter number of operators: ");
    scanf("%d", &n);

    printf("\nEnter the operators:\n");
    for (i = 0; i < n; i++)
    {
        scanf("%s", op[i]);
    }

    printf("\nEnter the precedence table:\n");
    printf("Use <, >, = or - for no relation.\n\n");

    for (i = 0; i < n; i++)
    {
        for (j = 0; j < n; j++)
        {
            printf("Relation between %s and %s: ",
                   op[i], op[j]);
            scanf(" %c", &table[i][j]);
        }
    }

    /*
       Generate precedence functions.

       a < b  => f(a) < g(b)
       a = b  => f(a) = g(b)
       a > b  => f(a) > g(b)

       We use repeated relaxation to find
       integer values satisfying all relations.
    */

    for (k = 0; k < n * n; k++)
    {
        int changed = 0;

        for (i = 0; i < n; i++)
        {
            for (j = 0; j < n; j++)
            {
                if (table[i][j] == '<')
                {
                    if (g[j] < f[i] + 1)
                    {
                        g[j] = f[i] + 1;
                        changed = 1;
                    }
                }
                else if (table[i][j] == '>')
                {
                    if (f[i] < g[j] + 1)
                    {
                        f[i] = g[j] + 1;
                        changed = 1;
                    }
                }
                else if (table[i][j] == '=')
                {
                    if (f[i] < g[j])
                    {
                        f[i] = g[j];
                        changed = 1;
                    }
                    else if (g[j] < f[i])
                    {
                        g[j] = f[i];
                        changed = 1;
                    }
                }
            }
        }

        if (!changed)
            break;
    }

    printf("\n---------------------------------\n");
    printf("   Operator   f      g\n");
    printf("---------------------------------\n");

    for (i = 0; i < n; i++)
    {
        printf("      %-6s %d      %d\n",
               op[i], f[i], g[i]);
    }

    printf("---------------------------------\n");

    printf("\nPrecedence Functions:\n");

    for (i = 0; i < n; i++)
    {
        printf("f(%s) = %d\tg(%s) = %d\n",
               op[i], f[i], op[i], g[i]);
    }

    return 0;
}