#!/bin/bash
assert() {
  expected="$1"
  input="$2"

  ./scc "$input" > tmp.s
  cc -o tmp tmp.s
  ./tmp
  actual="$?"

  if [ "$actual" = "$expected" ]; then
    echo "$input => $actual"
  else
    echo "$input => $expected expected, but got $actual"
    exit 1
  fi
}

assert 0 0
assert 42 42
assert 21 "5+20-4"
assert 10 "2 + 13-5"
assert 15 "3 * 5"
assert 12 "20 - 2 * 4"
assert 72 "( 20 - 2 ) * 4"
assert 10 "20 / 2"
assert 14 "24 - 20 / 2"
assert 2  "( 24 - 20 ) / 2"
assert 10 "- 10 + 20"
assert 0 "2 == 5"
assert 1 "2 == 2"
assert 0 "2 != 2"
assert 1 "2 != 5"
assert 1 "2 < 5"
assert 0 "5 < 2"
assert 1 "2 <= 5"
assert 1 "2 <= 2"
assert 0 "5 <= 2"
assert 1 "5 > 2"
assert 0 "2 > 5"
assert 1 "5 >= 2"
assert 1 "2 >= 2"
assert 0 "2 >= 5"
echo OK
