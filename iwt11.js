const numbers =;

function isPrime(num) {
    if (num <= 1) return false; 
    for (let i = 2; i <= Math.sqrt(num); i++) {
        if (num % i === 0) return false; 
    }
    return true; 
}

let sumOfPrimes = 0;
for (const num of numbers) {
    if (isPrime(num)) {
        sumOfPrimes += num;
    }
}

console.log("Sum of Prime Numbers:", sumOfPrimes);
