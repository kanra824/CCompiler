try() {
    expected="$1"
    input="$2"

    ./mdcc "$input" > tmp.s
    gcc -o tmp tmp.s
    ./tmp
    actual="$?"

    if [ "$actual" = "$expected" ]; then
        echo "$input => $actual"
    else
        echo "$expected expected, but got $actual"
        exit 1
    fi
}

err() {
    input="$1"
    ./mdcc "$input" > tmp.s
    actual="$?"
    if [ "$actual" = "1" ]; then
        echo "$input => <error>"
    else
        gcc -o tmp tmp.s
        ./tmp
        actual = "$?"
        echo "<error> expected, but no errors detected at compile time in $input and got $actual"
        exit 1
    fi
}


# try arith
try 0 "0;"
try 42 "42;"
try 21 "5+20-4;"
try 41 " 12 + 34 - 5;"
try 49 "1 + 2 * 24;"
try 47 "5+6*7;"
try 15 "5*(9-6);"
try 4 "(3+5)/2;"
try 2 "1 + -2 + 3;"
try 1 "7 + 3 * -2;"

# err arith
err "0"
err "+";
err "*+;"
err "0+;"
err "0++;"
err "0 += 1;"
err "0 -= 1;"
err "0 *= 1;"
err "0 /= 1;"
err "i = 5 % 2;"
# err "foo;"

# try compare
try 1 "0 == 0;"
try 0 "0 == 1;"
try 1 "0 != 1;"
try 0 "0 != 0;"
try 1 "0 < 1;"
try 0 "0 < 0;"
try 1 "1 <= 1;"
try 0 "1 <= 0;"
try 1 "1 > 0;"
try 0 "0 > 0;"
try 1 "1 >= 1;"
try 0 "0 >= 1;"

# try 
try 2 "a = 1; a + 1;"
try 7 "a = 1; b = 2 * 3 + 1; a * b;"
try 2 "foo = 1; foo + 1;"
try 7 "foo = 1; bar = 2 * 3 + 1; foo * bar;"

try 1 "if(1 == 1) 1; else 0;"
try 0 "if(0 == 1) 1; else 0;"

try 5 "i = 1; while(i < 5) i = i + 1; return i;"

try 10 "sum = 0; for(i = 0; i < 5; i = i + 1) sum = sum + i; return sum;"

echo OK