#include <iostream>
#include <cstring>
#include <cctype>
using namespace std;

// Function to check keyword
bool isKeyword(string str) {
    string keywords[] = {"BEGIN", "END", "IF", "THEN", "ELSE"};
    for (int i = 0; i < 5; i++) {
        if (str == keywords[i])
            return true;
    }
    return false;
}

int main() {
    string input;
    cout << "Enter input: ";
    cin >> input;

    // Identifier or Keyword
    if (isalpha(input[0])) {
        for (int i = 1; i < input.length(); i++) {
            if (!isalnum(input[i])) {
                cout << "Invalid Identifier\n";
                return 0;
            }
        }

        if (isKeyword(input))
            cout << "Keyword\n";
        else
            cout << "Identifier\n";
    }

    // Integer
    else if (isdigit(input[0])) {
        for (int i = 1; i < input.length(); i++) {
            if (!isdigit(input[i])) {
                cout << "Invalid Integer\n";
                return 0;
            }
        }
        cout << "Integer Constant\n";
    }

    // Relational Operators
    else if (input == "<" || input == "<=" || input == "=" ||
             input == "<>" || input == ">" || input == ">=") {
        cout << "Relational Operator\n";
    }

    else {
        cout << "Invalid Token\n";
    }

    return 0;
}