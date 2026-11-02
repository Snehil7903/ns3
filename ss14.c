#include <stdio.h>
#include <string.h>
#include <stdlib.h>

char *action[12][6] = {
    {"s5","","","s4","",""},
    {"","s6","","","","acc"},
    {"","r2","s7","","r2","r2"},
    {"","r4","r4","","r4","r4"},
    {"s5","","","s4","",""},
    {"","r6","r6","","r6","r6"},
    {"s5","","","s4","",""},
    {"s5","","","s4","",""},
    {"","s6","","","s11",""},
    {"","r1","s7","","r1","r1"},
    {"","r3","r3","","r3","r3"},
    {"","r5","r5","","r5","r5"}
};

int go[12][3] = {
    {1,2,3},
    {-1,-1,-1},
    {-1,-1,-1},
    {-1,-1,-1},
    {8,2,3},
    {-1,-1,-1},
    {-1,9,3},
    {-1,-1,10},
    {-1,-1,-1},
    {-1,-1,-1},
    {-1,-1,-1},
    {-1,-1,-1}
};

int len[] = {0,3,1,3,1,3,1};
char lhs[] = {' ','E','E','T','T','F','F'};

int main()
{
    char input[100], token;
    int stack[100], top = 0;
    int i = 0, state, col, rule, next;

    printf("Enter expression (use id): ");
    scanf("%99s", input);

    strcat(input, "$");
    stack[0] = 0;

    printf("\nSTACK\tINPUT\tACTION\n");

    while(1)
    {
        state = stack[top];

        if(input[i] == 'i' && input[i+1] == 'd')
            token = 'i';
        else
            token = input[i];

        switch(token)
        {
            case 'i': col = 0; break;
            case '+': col = 1; break;
            case '*': col = 2; break;
            case '(': col = 3; break;
            case ')': col = 4; break;
            case '$': col = 5; break;
            default:
                printf("Invalid symbol\n");
                return 0;
        }

        printf("%d\t%s\t", state, input+i);

        if(strcmp(action[state][col], "acc") == 0)
        {
            printf("ACCEPT\n");
            printf("\nValid Expression\n");
            break;
        }

        else if(action[state][col][0] == 's')
        {
            next = atoi(action[state][col]+1);
            stack[++top] = next;

            if(token == 'i')
                i += 2;
            else
                i++;

            printf("SHIFT %d\n", next);
        }

        else if(action[state][col][0] == 'r')
        {
            rule = atoi(action[state][col]+1);

            top -= len[rule];

            state = stack[top];

            if(lhs[rule] == 'E')
                col = 0;
            else if(lhs[rule] == 'T')
                col = 1;
            else
                col = 2;

            next = go[state][col];

            if(next == -1)
            {
                printf("ERROR\n");
                printf("Invalid Expression\n");
                break;
            }

            stack[++top] = next;

            printf("REDUCE by %c\n", lhs[rule]);
        }

        else
        {
            printf("ERROR\n");
            printf("\nInvalid Expression\n");
            break;
        }
    }

    return 0;
}