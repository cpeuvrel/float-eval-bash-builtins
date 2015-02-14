# float-eval-bash-builtins
A bash builtins which evaluate a arithmetic expression (as a string) and return a double (+/- what bc does).

# Build
Just do a `make`

# Use
Run `enable -f ./float_eval.so float_eval` (you'll have to repeat this command in each shell). 
You can use the absolute path near `./float_eval.so` to use it anywhere (or put it in an alias)

You can now use `float_eval "1+2"` and you'll get the result in $REPLY.

You may use `float_eval -v "1+2"` to display the syntax tree.

# Supported operations
You can use every common operations :
  - `+` `-` `*` `/` and `%` (modulo)
  - bitwise operators `|` `&` `^` and bitwise shifts `<<` `>>`
  - expression in parentheses `(...)` (may be on several levels `(3 * (1+2))`)
  - comparaisons `==` `!=` `<` `<=` `>` `>=`
  - combinaisons `||` `&&`
  
Note that comparaisons and combinaisons results in a boolean : `1` == `true` and `0` == `false`
  
# Not yet supported
  - Not `!`
  - Bitwise not `~`
  
# Benchmarks
You can do a simple benchmark by executing `./test.sh` (it will also check that the builtin's behavior is as expected).

It will compare the performances of the builtin and a bash function using `bc` on 1k iterations
(you can change that by setting $NB in the environment `NB=10000 ./test.sh`).

In my tests, the bash version is +/- 100 time slower than the builtin. For 100k iterations :
  - Builtin : 0.944s
  - Bash (bc) : 93.335s
