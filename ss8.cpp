#include <iostream>
#include <stack>
#include <string>
#include <map>
#include <vector>
using namespace std;

int main() {

    // Predictive parsing table for Grammar 1

    /*
        E  -> T E'
        E' -> + T E' | e
        T  -> F T'
        T' -> * F T' | e
        F  -> ( E ) | id
    */

    map<pair<string, string>, string> table;

    // E
    table[{"E", "id"}] = "T E'";
    table[{"E", "("}]  = "T E'";

    // E'
    table[{"E'", "+"}] = "+ T E'";
    table[{"E'", ")"}] = "e";
    table[{"E'", "$"}] = "e";

    // T
    table[{"T", "id"}] = "F T'";
    table[{"T", "("}]  = "F T'";

    // T'
    table[{"T'", "*"}] = "* F T'";
    table[{"T'", "+"}] = "e";
    table[{"T'", ")"}] = "e";
    table[{"T'", "$"}] = "e";

    // F
    table[{"F", "id"}] = "id";
    table[{"F", "("}]  = "( E )";

    string input;

    cout << "Enter expression: ";
    cin >> input;

    // Add end marker
    input += "$";

    stack<string> st;

    st.push("$");
    st.push("E");

    int i = 0;

    while (!st.empty()) {

        string top = st.top();

        string current;

        // Recognize id
        if (input.substr(i, 2) == "id")
            current = "id";
        else
            current = string(1, input[i]);

        // If terminal
        if (top == "id" || top == "+" ||
            top == "*" || top == "(" ||
            top == ")" || top == "$") {

            if (top == current) {

                st.pop();

                if (current == "id")
                    i += 2;
                else
                    i++;

            }
            else {
                cout << "Rejected\n";
                return 0;
            }
        }

        // Non-terminal
        else {

            auto key = make_pair(top, current);

            if (table.find(key) == table.end()) {
                cout << "Rejected\n";
                return 0;
            }

            string production = table[key];

            st.pop();

            if (production != "e") {

                vector<string> symbols;

                string temp;

                for (char c : production) {

                    if (c == ' ')
                        continue;

                    if (c == 'E' && temp.empty()) {
                        symbols.push_back("E");
                    }
                    else if (c == 'T') {
                        symbols.push_back("T");
                    }
                    else if (c == 'F') {
                        symbols.push_back("F");
                    }
                    else if (c == 'd') {
                        symbols.push_back("d");
                    }
                    else if (c == 'i') {
                        // handled as id below
                    }
                }

                // Easier explicit handling
                if (production == "T E'")
                    st.push("E'"), st.push("T");

                else if (production == "+ T E'")
                    st.push("E'"), st.push("T"), st.push("+");

                else if (production == "F T'")
                    st.push("T'"), st.push("F");

                else if (production == "* F T'")
                    st.push("T'"), st.push("F"), st.push("*");

                else if (production == "id")
                    st.push("id");

                else if (production == "( E )")
                    st.push(")"), st.push("E"), st.push("(");
            }
        }
    }

    if (i == input.length())
        cout << "Accepted\n";
    else
        cout << "Rejected\n";

    return 0;
}