#include <stdio.h>
#include <string.h>

char *action[12][6] = {
    {"s5", "", "", "s4", "", ""},
    {"", "s6", "", "", "", "acc"},
    {"", "r2", "s7", "", "r2", "r2"},
    {"", "r4", "r4", "", "r4", "r4"},
    {"s5", "", "", "s4", "", ""},
    {"", "r6", "r6", "", "r6", "r6"},
    {"s5", "", "", "s4", "", ""},
    {"s5", "", "", "s4", "", ""},
    {"", "s6", "", "", "s11", ""},
    {"", "r1", "s7", "", "r1", "r1"},
    {"", "r3", "r3", "", "r3", "r3"},
    {"", "r5", "r5", "", "r5", "r5"}
};

int goTo[12][3] = {
    {1, 2, 3},
    {-1, -1, -1},
    {-1, -1, -1},
    {-1, -1, -1},
    {8, 2, 3},
    {-1, -1, -1},
    {-1, 9, 3},
    {-1, -1, 10},
    {-1, -1, -1},
    {-1, -1, -1},
    {-1, -1, -1},
    {-1, -1, -1}
};

char *production[] = {
    "",
    "E->E+T",
    "E->T",
    "T->T*F",
    "T->F",
    "F->(E)",
    "F->id"
};

int length[] = {0, 3, 1, 3, 1, 3, 1};

int lhs[] = {0, 0, 0, 1, 1, 2, 2};

int terminal(char c)
{
    switch(c)
    {
        case 'i': return 0;
        case '+': return 1;
        case '*': return 2;
        case '(': return 3;
        case ')': return 4;
        case '$': return 5;
        default: return -1;
    }
}

int nonterminal(char c)
{
    if(c == 'E') return 0;
    if(c == 'T') return 1;
    if(c == 'F') return 2;
    return -1;
}

int main()
{
    char input[100];
    int stack[100], top = 0;
    int pos = 0, state, col, row;
    char symbol;

    printf("Enter expression (use id for identifier): ");
    scanf("%99s", input);

    strcat(input, "$");

    stack[0] = 0;

    printf("\nStack\t\tInput\t\tAction\n");

    while(1)
    {
        state = stack[top];

        if(input[pos] == 'i' && input[pos+1] == 'd')
            symbol = 'i';
        else
            symbol = input[pos];

        col = terminal(symbol);

        if(col == -1)
        {
            printf("Invalid symbol\n");
            return 0;
        }

        if(symbol == 'i' && input[pos+1] == 'd')
            pos += 2;
        else
            pos++;

        pos -= (symbol == 'i' ? 2 : 1);

        char *act = action[state][col];

        printf("%d\t\t%s\t\t", state, input + pos);

        if(strcmp(act, "acc") == 0)
        {
            printf("ACCEPT\n");
            break;
        }
        else if(act[0] == 's')
        {
            int next = atoi(act + 1);
            stack[++top] = next;
            printf("SHIFT %d\n", next);
            pos += (symbol == 'i' ? 2 : 1);
        }
        else if(act[0] == 'r')
        {
            int rule = atoi(act + 1);

            printf("REDUCE %s\n", production[rule]);

            top -= length[rule];

            row = nonterminal(production[rule][0]);
            state = stack[top];

            stack[++top] = goTo[state][row];
        }
        else
        {
            printf("ERROR: Invalid expression\n");
            break;
        }
    }

    return 0;
}