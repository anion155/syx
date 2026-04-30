### Values
- `1`, `-1` integer number value
- `1.0`, `-1.0`, `1.`, `.0` fractional number value
- `"something"` string literal value
- `something` symbol value
- `nil` nil value
- `true`, `#t` true value
- `false`, `#f` false value
- `(a . b)` pair of `a` and `b`
- `(a b)` list of `a`, `b` and `nil`
- `(a b . c)` list of `a`, `b` and `c`

### Functions
Functions can be:
- builtin - built into language functions, all arguments evaluated
- special form - built into language construction that do not evaluate arguments at call, but follow inner conditions

### evaluation
- `(fn a b)` calls function `fn` with params `eval a` and `eval b`

## Special forms

### quote
Returns first argument unevaluated.
`(quote <value>) => value`

### begin
Evaluates forms in order and returns last result.
`(begin ...<forms>) => eval <forms[-1]>`

### lambda
Creates a closure that captures current environment.
Forms evaluated in order and last result is returned.
`(lambda <arguments-names-list> ...<forms>) = eval <forms[-1]>`

#### arguments-names-list
- `(a b)` two arguments `a` and `b`
- `(a b . c)` two arguments `a`, `b` and all rest arguments will be in `c` as list
- `((a 10) (b . 5))` argument `a` with default value `10`, and argument `b` with default `5`

### define
Binds value to a name in the current environment.
`(define <name> <value>) >= nil`

Can contain shorthand version of lambda definition:
`(define (<name> ...<arguments-names-list>) ...<value>) >= nil`

### set
Mutate an existing binding or creates new one in current environment.
`(set <name> <value>) >= nil`

### let
Create new variable bindings in parallel on new environment and execute a series of forms in that environment.
`(let (...(<name> <value>)) ...<forms>) >= nil`

<!-- ### let*
Create new variable bindings sequentially on new environment and execute a series of forms in that environment.
`(let* (...(<name> <value>)) ...<forms>) >= nil` -->

<!-- ### letrec
Defin new variable bindings and then set them on new environment and execute a series of forms in that environment.
`(letrec (...(<name> <value>)) ...<forms>) >= nil` -->

### and
Evaluates left to right, returns first falsy or last value. Short-circuits.
`(and ...<forms>) => first falsy | last value`

### or
Evaluates left to right, returns first truthy or last value. Short-circuits.
`(or ...<forms>) => first truthy | last value`

### if
`(if <condition> <then> <else>) => eval(<then>) | eval(<else>) | nil`

### cond
`(cond ...(<condition> ...<forms>) (else ...<forms>)) => first matching result | nil`

## Builtins List

### cons
Takes exactly 2 arguments and returns a pair (left . right).
`(cons <left> <right>) => (<left> . <right>)`

### car / cdr
Returns the left / right element of a pair.
`(car <pair>) => <left>`
`(cdr <pair>) => <right>`

### apply
Calls a function with a list as its argument list.
`(apply <fn> <args-list>) => result`

### map
Applies a function to each element of a list and returns a new list of results.
`(map <fn> <list>) => <list>`

## Builtins Arithmetics

### +, -, *, /
Sequentially applies all arguments. Integers promoted to fractional if any argument is fractional.
`(+) => 0`
`(+ ...<numbers>) => sum`
`(- <number> ...<numbers>) => difference`
`(*) => 1`
`(* ...<numbers>) => product`
`(/ <number> ...<numbers>) => quotient`

### Comparison <, >, =, <=, >=
Take 2 or more numeric arguments, chainable. Return `#t` or `#f`.

`(= <number> ...<numbers>) => #t if all equal`
`(< <number> ...<numbers>) => #t if strictly increasing`
`(> <number> ...<numbers>) => #t if strictly decreasing`
`(<= <number> ...<numbers>) => #t if non-decreasing`
`(>= <number> ...<numbers>) => #t if non-increasing`

## Builtins Boolean

### not
Returns `#f` if argument is truthy, `#t` if falsy.
`(not <value>) => #t | #f`

## Miscellaneous Builtins

### FD
`'stdout` | `'stderr` | `'stdin` | `'<fd-id>`

### print
Print arguments to file.
`(print <FD>? ...<arguments>) => #t | #f`

### println
Print arguments to file and newline.
`(println <FD>? ...<arguments>) => #t | #f`

### printf
Print arguments to file.
`(print <FD>? <format-string> ...<arguments>) => #t | #f`
