#include <iostream>
#include <string>
using namespace std;

string input;
int pos = 0;

bool S();
bool A();
bool Aprime();

bool match(char expected)
{
    if (pos < input.length() && input[pos] == expected)
    {
        pos++;
        return true;
    }

    return false;
}

// S -> Aa | b
bool S()
{
    int start = pos;

    // Try S -> Aa
    if (A() && match('a'))
        return true;

    // Backtrack
    pos = start;

    // Try S -> b
    if (match('b'))
        return true;

    pos = start;
    return false;
}

// A -> bdA' | fA'
bool A()
{
    int start = pos;

    // A -> bdA'
    if (match('b') && match('d') && Aprime())
        return true;

    pos = start;

    // A -> fA'
    if (match('f') && Aprime())
        return true;

    pos = start;
    return false;
}

// A' -> cA' | adA' | epsilon
bool Aprime()
{
    int start = pos;

    // A' -> cA'
    if (match('c') && Aprime())
        return true;

    pos = start;

    // A' -> adA'
    if (match('a') && match('d') && Aprime())
        return true;

    // A' -> epsilon
    pos = start;
    return true;
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