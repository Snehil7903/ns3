//Lab 9 (22-09-2025) iwt

// ==========================================
// 1. ASSIGNMENT OPERATORS
// ==========================================
let a = 10; // Basic assignment
a += 5;     // Addition assignment (a = a + 5) -> 15
a -= 3;     // Subtraction assignment (a = a - 3) -> 12
a *= 2;     // Multiplication assignment (a = a * 2) -> 24
a /= 4;     // Division assignment (a = a / 4) -> 6
a %= 4;     // Remainder assignment (a = a % 4) -> 2

// 🌟 MORE EXAMPLES: Advanced & Logical Assignments
let power = 2;
power **= 3; // Exponentiation assignment (2^3) -> 8

// Logical Assignment Operators (ES2021)
let username = null;
username ??= "Guest"; // Nullish coalescing assignment: assigns only if null/undefined -> "Guest"

let currentScore = 0;
currentScore ||= 10;  // Logical OR assignment: assigns if falsy (0 is falsy!) -> 10

let loggedIn = true;
loggedIn &&= "Active"; // Logical AND assignment: assigns if truthy -> "Active"


// ==========================================
// 2. ARITHMETIC OPERATORS
// ==========================================
let x = 10;
let y = 3;

let sum = x + y;        // Addition (13)
let diff = x - y;       // Subtraction (7)
let prod = x * y;       // Multiplication (30)
let quotient = x / y;   // Division (3.3333...)
let rem = x % y;        // Remainder/Modulus (1)
let exp = x ** y;       // Exponentiation (10^3 = 1000)

// 🌟 MORE EXAMPLES: Increment/Decrement & Special Numbers
let count = 5;
console.log(count++); // Post-increment: prints 5, then increments count to 6
console.log(++count); // Pre-increment: increments to 7, then prints 7

// Division by zero and invalid operations
console.log(10 / 0);      // Infinity
console.log(-10 / 0);     // -Infinity
console.log("apple" / 2); // NaN (Not a Number)


// ==========================================
// 3. COMPARATIVE OPERATORS
// ==========================================
// Relational
let isGreater = x > y;   // true
let isLess = x < y;      // false
let isGreaterEqual = x >= 10; // true

// Equality: Abstract (==) vs Strict (===)
console.log(5 == "5");   // true  (Checks value only, converts types)
console.log(5 === "5");  // false (Checks value AND type)

// Inequality: Abstract (!=) vs Strict (!==)
console.log(5 != "5");   // false (Values match after conversion)
console.log(5 !== "5");  // true  (Types do not match)

// 🌟 MORE EXAMPLES: Tricky Evaluations
console.log(null == undefined);   // true  (Special rule in abstract equality)
console.log(null === undefined);  // false (Different data types)
console.log(false == 0);          // true  (false is coerced to 0)
console.log(false === 0);         // false (Boolean vs Number)

// String relational comparison (Alphabetical/Lexicographical order)
console.log("apple" < "banana"); // true
console.log("2" > "12");         // true  (Compares character codes: "2" is greater than "1")


// ==========================================
// 4. LOGICAL OPERATORS
// ==========================================
let isAdult = true;
let hasTicket = false;

let canEnter = isAdult && hasTicket; // AND (false)
let absoluteEntry = isAdult || hasTicket; // OR (true)
let cannotEnter = !isAdult;          // NOT (false)

// 🌟 MORE EXAMPLES: Short-Circuit Evaluation
// AND (&&) short-circuits on the first falsy value and returns it
console.log(false && "hello"); // false
console.log("user" && "profile"); // "profile" (Returns last truthy value if all are truthy)

// OR (||) short-circuits on the first truthy value and returns it
console.log("default_avatar" || "user_avatar"); // "default_avatar"
console.log("" || "fallback_text");             // "fallback_text" (Empty string is falsy)


// ==========================================
// 5. CONDITIONALS (If-Else, Switch, Ternary, Nesting)
// ==========================================
let score = 85;
let VIP = true;

// If / Else If / Else
if (score >= 90) {
    console.log("Grade: A");
} else if (score >= 80) {
    console.log("Grade: B"); // This executes
} else {
    console.log("Grade: C");
}

// Nested Conditionals
if (score >= 80) {
    if (VIP) {
        console.log("Honour Roll with VIP Access"); // This executes
    } else {
        console.log("Standard Honour Roll");
    }
}

// Switch Statement
let day = "Monday";
switch (day) {
    case "Monday":
        console.log("Start of the week!");
        break;
    case "Friday":
        console.log("Weekend is close!");
        break;
    default:
        console.log("Regular weekday.");
}

// Ternary Operator (condition ? exprIfTrue : exprIfFalse)
let message = score >= 50 ? "Passed" : "Failed";

// Nested Ternary Operator
let classification = score >= 90 ? "Excellent" : (score >= 70 ? "Good" : "Needs Improvement");
console.log(classification); // "Good"

// 🌟 MORE EXAMPLES: Switch Fall-through & Complex Ternaries
// Switch Statement Fall-through (Grouping multiple cases together)
let month = "July";
switch (month) {
    case "June":
    case "July":
    case "August":
        console.log("It's Summer season!"); // Executes for June, July, or August
        break;
    case "December":
    case "January":
        console.log("It's Winter season!");
        break;
    default:
        console.log("Muted climate.");
}

// Multiple conditions inside a Ternary
let age = 20;
let restriction = (age >= 18 && VIP) ? "Full Unrestricted Access" : "Standard Access";


// ==========================================
// 6. STRING OPERATIONS & TYPE COERCION
// ==========================================
let str1 = "Hello";
let str2 = "World";

// Standard Concatenation
let greeting = str1 + " " + str2; // "Hello World"

// Addition / Concatenation Behaviors (Type Coercion)
console.log("Result 1:", "5" + 5);     // "55"   (Number 5 becomes string "5")
console.log("Result 2:", 5 + 5 + "5"); // "105"  (5+5 happens first = 10, then "10" + "5")
console.log("Result 3:", "5" + 5 + 5); // "555"  ("5"+5 becomes "55", then "55" + 5)

// What about subtraction, multiplication, and division?
console.log("Subtraction:", "10" - 2); // 8      (String "10" is coerced into number 10)
console.log("Multiplication:", "5" * "3"); // 15   (Both strings are coerced into numbers)

// 🌟 MORE EXAMPLES: Non-Numeric Coercion Math Tragedies
console.log("Addition with Boolean:", "Result: " + true); // "Result: true"
console.log("Math with Booleans:", 5 + true);             // 6  (true is coerced into 1)
console.log("Math with null:", 10 + null);                // 10 (null is coerced into 0)
console.log("Math with undefined:", 10 + undefined);       // NaN (undefined becomes NaN during math)

// Template Literals (Modern alternative to string concatenation)
let user = "Alice";
let itemsCount = 3;
let receiptMessage = `Hello ${user}, you have purchased ${itemsCount} items. Total cost: $${itemsCount * 25}`;
console.log(receiptMessage); 
