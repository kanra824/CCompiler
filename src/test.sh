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
        echo "$input => $actual"
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
        echo "$input => $actual"
        echo "<error> expected, but got $actual"
        exit 1
    fi
}


# try arith
try 0 "main() {return 0;}"
try 42 "main() {return 42;}"
try 21 "main() {return 5+20-4;}"
try 41 "main() {return 12 + 34 - 5;}"
try 49 "main() {return 1 + 2 * 24;}"
try 47 "main() {return 5+6*7;}"
try 15 "main() {return 5*(9-6);}"
try 4 "main() {return (3+5)/2;}"
try 2 "main() {return 1 + -2 + 3;}"
try 1 "main() {return 7 + 3 * -2;}"

# err arith
err "0"
err "main() {return +;}";
err "main() {return *+;}";
err "main() {return 0+;}"
err "main() {return 0++;}"
err "main() {return 0 += 1;}"
err "main() {return 0 -= 1;}"
err "main() {return 0 *= 1;}"
err "main() {return 0 /= 1;}"
err "main() {return i = 5 % 2;}"
# err "foo;"

# try compare
try 1 "main() {return 0 == 0;}"
try 0 "main() {return 0 == 1;}"
try 1 "main() {return 0 != 1;}"
try 0 "main() {return 0 != 0;}"
try 1 "main() {return 0 < 1;}"
try 0 "main() {return 0 < 0;}"
try 1 "main() {return 1 <= 1;}"
try 0 "main() {return 1 <= 0;}"
try 1 "main() {return 1 > 0;}"
try 0 "main() {return 0 > 0;}"
try 1 "main() {return 1 >= 1;}"
try 0 "main() {return 0 >= 1;}"

# try 
try 2 "main() {a = 1; return a + 1;}"
try 7 "main() {a = 1; b = 2 * 3 + 1; return a * b;}"
try 2 "main() {foo = 1; return foo + 1;}"
try 7 "main() {foo = 1; bar = 2 * 3 + 1; return foo * bar;}"

try 1 "main() {if(1 == 1) return 1; else return 0;}"
try 0 "main() {if(0 == 1) return 1; else return 0;}"

try 5 "main() {i = 1; while(i < 5) i = i + 1; return i;}"

try 10 "main() {sum = 0; for(i = 0; i < 5; i = i + 1) sum = sum + i; return sum;}"

try 0 "main() {if(1 == 0) {i = 1;return i + 2;} else {sum = 0;for(i = 1; i <= 0; i = i + 1) {sum = sum + i;}return sum;}}"

try 3 "add(x, y) {return x + y;} main() {return add(1, 2);}"
try 3 "add(x, y) {return x + y;} main() {a = 1; b = 2; c = 3; return add(a, 2);}"

try 1 "main() {x = 1; y = &x; return *y;}"
echo OK