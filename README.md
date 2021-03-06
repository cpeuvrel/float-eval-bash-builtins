# float-eval-bash-builtins
A bash builtins which evaluate a arithmetic expression (as a string) and return a double (+/- what GNU bc does).

# Build
Just do a `make` to build the builtin.

If you want to build a static binary (to simply call it with something like `./float_eval "<expr>"`), you do a `make static=1`.

In either cases, you can enable debug with: `make debug=1`.

# Use
Run `enable -f ./float_eval.so float_eval` (you'll have to repeat this command in each shell). 
You can use the absolute path near `./float_eval.so` to use it anywhere (or put it in an alias)

You can now use `float_eval "1+2"` and you'll get the result in $REPLY.

You can ask for several calculations at once `float_eval "1+2" "2*3" "8%3"` and $REPLY will store them as an array (REPLY[0]=3 REPLY[1]=6 REPLY[2]=2).

# Supported operations
You can use every common operations :
  - `+` `-` `*` `/` and `%` (modulo)
  - bitwise operators `|` `&` `^` and bitwise shifts `<<` `>>`
  - expression in parentheses `(...)` (may be on several levels `(3 * (1+2))`)
  - comparaisons `==` `!=` `<` `<=` `>` `>=`
  - combinaisons `||` `&&`
  - Not `!`
  - Bitwise not `~`
  - Exponentiation (pow) `**`
  - Set wanted precision (-p INTEGER)
  - Scientific notation `1.00E+003`
  - Error handling: On invalid input, the binary/builtin return 1
  - Long input lines (dynamically allocated)
  - Numbers are coded on 256 bits using the MPFR library (<http://www.mpfr.org/>)
  
Note that comparaisons and combinaisons results in a boolean : `1` == `true` and `0` == `false`

Scientific notation is a decimal number (possibly with a dot) followed by an 'E' or 'e', optionally followed with a plus or minus sign, followed by decimal digits. It indicates multiplication by a power of 10 (cf man of strtod).

You can look `test.sh` to see all operations and their combinaisons that are known to work.
  
# Not yet supported
  - Syntax check
  
# Benchmarks
You can do a simple benchmark by executing `./test.sh` (it will also check that the builtin's behavior is as expected).

It will compare the performances of the builtin and a bash function using `bc` on 1k iterations
(you can change that by setting $NB in the environment `NB=10000 ./test.sh`).

In my tests, the bash version is +/- 100 time slower than the builtin (200 if one builtin call). For 100k calls :
  - Builtin : 1.076
  - Builtin (with one call of 100k args) : 0.439
  - Bash (bc) : 98.531s

# Notes
I added a directory `bash-headers-4.1.2-9.el6_2.x86_64` (as provided in CentOS 6) with all bash's headers. Feel free to test with another version, it shouldn't be a problem.
