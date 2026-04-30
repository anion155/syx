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
`(begin ...<form>) => eval last <form>`

#### arguments-names-list
- `(a b)` two arguments `a` and `b`
- `(a b . c)` two arguments `a`, `b` and all rest arguments will be in `c` as list
- `((a 10) (b . 5))` argument `a` with default value `10`, and argument `b` with default `5`

### lambda
Creates a closure that captures current environment.
Forms evaluated in order and last result is returned.
`(lambda <arguments-names-list> ...<form>) = eval last <form>`

### define
Binds value to a name in the current environment.
`(define <name> <value>) >= nil`

Can contain shorthand version of lambda definition:
`(define (<name> <arguments-names-list>) ...<form>) >= nil`

### set
Mutate an existing binding or creates new one in current environment.
`(set <name> <value>) >= nil`

### let
Create new variable bindings in parallel on new environment and execute a series of forms in that environment.
`(let (...(<name> <value>)) ...<fors>) >= nil`

<!-- ### let*
Create new variable bindings sequentially on new environment and execute a series of forms in that environment.
`(let* (...(<name> <value>)) ...<form>) >= nil` -->

<!-- ### letrec
Defin new variable bindings and then set them on new environment and execute a series of forms in that environment.
`(letrec (...(<name> <value>)) ...<form>) >= nil` -->

### and
Evaluates left to right, returns first falsy or last value. Short-circuits.
`(and ...<form>) => first falsy | last value`

### or
Evaluates left to right, returns first truthy or last value. Short-circuits.
`(or ...<form>) => first truthy | last value`

### if
`(if <condition> <then> <else>) => eval(<then>) | eval(<else>) | nil`

### cond
`(cond ...(<condition> ...<form>) (else ...<form>)) => first matching result | nil`

## Builtins List

### cons
Takes exactly 2 arguments and returns a pair (left . right).
`(cons <left> <right>) => (<left> . <right>)`

### car / cdr
Returns the left / right element of a pair.
`(car <pair>) => <left>`
`(cdr <pair>) => <right>`

### apply
Calls a function with supplied arguments, all arguments are concatinated into one list.
`(apply <fn> ...<arguments>) => result`

### map
Applies a function to each element of a list and returns a new list of results.
`(map <fn> <list>) => <list>`

## Builtins Arithmetics

### +, -, *, /
Sequentially applies all arguments. Integers promoted to fractional if any argument is fractional.
`(+) => 0`
`(+ ...<number>) => sum`
`(- <number> ...<number>) => difference`
`(*) => 1`
`(* ...<number>) => product`
`(/ <number> ...<number>) => quotient`

### =
Take 2 or more arguments, chainable. Return `#t` or `#f`.
`(= <value> ...<value>) => #t if all equal`

`<value>` can be any of this:
- `bool`
- `number`
- `string`
- `pair` - compairs both sides recursively
- `quote` - compares stored values
- `fn` - compared by reference
- `symbol` - compared by reference

### Comparison <, >, <=, >=
Take 2 or more arguments, chainable. Return `#t` or `#f`.
`(< <value> ...<value>) => #t if strictly increasing`
`(> <value> ...<value>) => #t if strictly decreasing`
`(<= <value> ...<value>) => #t if non-decreasing`
`(>= <value> ...<value>) => #t if non-increasing`

`<value>` can be any of this:
- `number`
- `string`
- `pair` - compairs both sides recursively
- `quote` - compares stored values

## Builtins Boolean

### not
Returns `#f` if argument is truthy, `#t` if falsy.
`(not <value>) => #t | #f`

## Miscellaneous Builtins

### FD
`stdout` | `stderr` | `stdin` | `<fd-id>`

### print
Print arguments to file.
`(print '<FD>? ...<argument>) => #t | #f`

### println
Print arguments to file and newline.
`(println '<FD>? ...<argument>) => #t | #f`

### printf
Print arguments to file.
`(print '<FD>? <format-string> ...<argument>) => #t | #f`
