#include <iostream>
#include <string>
using namespace std;

string input;
int pos = 0;

bool S();
bool A();

bool match(char expected)
{
    if (pos < input.length() && input[pos] == expected)
    {
        pos++;
        return true;
    }

    return false;
}

// S -> cAd
bool S()
{
    if (!match('c'))
        return false;

    if (!A())
        return false;

    if (!match('d'))
        return false;

    return true;
}

// A -> ab | a
bool A()
{
    int start = pos;

    // Try A -> ab
    if (match('a') && match('b'))
        return true;

    // Backtrack
    pos = start;

    // Try A -> a
    if (match('a'))
        return true;

    pos = start;
    return false;
}

int main()
{
    cout << "Enter the string: ";
    cin >> input;

    pos = 0;

    if (S() && pos == input.length())
        cout << "String Accepted" << endl;
    else
        cout << "String Rejected" << endl;

    return 0;
}