try() {
    expected="$1"
    input="$2"
    assert="$3"

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

try 2 "a = 1; a + 1;"
try 7 "a = 1; b = 2 * 3 + 1; a * b;"

try 2 "foo = 1; foo + 1;"
try 7 "foo = 1; bar = 2 * 3 + 1; foo * bar;"

try 1 "if(1 == 1) 1; else 0;"
try 0 "if(0 == 1) 1; else 0;"

try 5 "i = 1; while(i < 5) i = i + 1; return i;"

try 10 "sum = 0; for(i = 0; i < 5; i = i + 1) sum = sum + i; return sum;"



echo OK