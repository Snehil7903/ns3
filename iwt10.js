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

// ==========================================
// 4. LOGICAL OPERATORS
// ==========================================
let isAdult = true;
let hasTicket = false;

let canEnter = isAdult && hasTicket; // AND (false)
let absoluteEntry = isAdult || hasTicket; // OR (true)
let cannotEnter = !isAdult;          // NOT (false)

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


let str1 = "Hello";
let str2 = "World";

// Standard Concatenation
let greeting = str1 + " " + str2; // "Hello World"

// Addition / Concatenation Behaviors (Type Coercion)
console.log("Result 1:", "5" + 5);     // "55"   (Number 5 becomes string "5")
console.log("Result 2:", 5 + 5 + "5"); // "105"  (5+5 happens first = 10, then "10" + "5")
console.log("Result 3:", "5" + 5 + 5); // "555"  ("5"+5 becomes "55", then "55" + 5)

// What about subtraction, multiplication, and division?
// JavaScript will try to convert strings back into numbers for these operators!
console.log("Subtraction:", "10" - 2); // 8      (String "10" is coerced into number 10)
console.log("Multiplication:", "5" * "3"); // 15   (Both strings are coerced into numbers)
