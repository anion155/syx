### Values
- `1`, `-1` -  integer number value
- `1.0`, `-1.0`, `1.`, `.0` - fractional number value
- `"something"` - string literal value
- `something` - symbol value
- `#n`, `#nil`, `#null` -  nil value
- `#t`, `#true` true value
- `#f`, `#false` - false value
- `(a . b)` - pair of `a` and `b`
- `(a b)` - list of `a`, `b` and `nil`
- `(a b . c)` - list of `a`, `b` and `c`

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
`(lambda <name>? <arguments-names-list> ...<form>) = eval last <form>`
Name is for debug purposes only

### define
Binds value to a name in the current environment.
`(define <name> <value>) >= #nil`

Can contain shorthand version of lambda definition:
`(define (<name> <arguments-names-list>) ...<form>) >= #nil`

### set
Mutate an existing binding or creates new one in current environment.
`(set <name> <value>) >= #nil`

### is-set?
Checks if environment has binding.
`(is-set? <name>) >= #t|#f`

### get
Get an existing binding or return nil.
`(get <name>) >= #nil`

### let
Create new variable bindings in parallel on new environment and execute a series of forms in that environment.
`(let (...(<name> <value>)) ...<fors>) >= #nil`

<!-- ### let*
Create new variable bindings sequentially on new environment and execute a series of forms in that environment.
`(let* (...(<name> <value>)) ...<form>) >= #nil` -->

<!-- ### letrec
Defin new variable bindings and then set them on new environment and execute a series of forms in that environment.
`(letrec (...(<name> <value>)) ...<form>) >= #nil` -->

### and
Evaluates left to right, returns first falsy or last value. Short-circuits.
`(and ...<form>) => first falsy | last value`

### or
Evaluates left to right, returns first truthy or last value. Short-circuits.
`(or ...<form>) => first truthy | last value`

### if
`(if <condition> <then> <else>) => eval(<then>) | eval(<else>) | #nil`

### cond
`(cond ...(<condition> ...<form>) (else ...<form>)) => first matching result | #nil`

### throw
Create value of type `throw` that is should trigger early return of said value after any evaluation.
It carries first argument (as reason) and evaluation stack reference.
`(throw <value>)`

### try/catch/finally
Special form for intercepting throw values and ensuring cleanup logic is executed.
`(try <body> ...[(catch <symbol>? ...<forms>)|(finally ...<forms>)])`

### return
Special form to trigger an immediate exit from the current function, carrying a value.
`(return <value>)`

## Builtins List

### cons
Takes exactly 2 arguments and returns a pair (left . right).
`(cons <left> <right>) => (<left> . <right>)`

### list
Evaluates each argument and constructs a new list containing the results.
`(list ...<value>) => (...<eval<value>>)`

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

## Builtins Boolean

### =
Applies structural check between each consequence pairs.
`(= <value> ...<value>) => true if all equal`

`<value>` can be any of this:
- `bool`
- `number`
- `string`
- `pair` - compares both sides recursively
- `quote` - compares stored values
- `fn` - compared by reference
- `symbol` - compared by reference

Examples:
`(= #t #t) => true`
`(= 1 1) => true`
`(= 1 1.0) => true`
`(= "a" "a") => true`
`(= '(1 2) '(1 2)) => true`
`(= ''1 ''1) => true`
`(= if if) => true`
`(= (lambda (a) a) (lambda (a) a)) => false`
`(= 'a 'a) => true`
`(define a 1) (define b 1) (= a b) => false`

### Comparison <, >, <=, >=
Applies operators between each consequence pairs.
`(< <value> ...<value>) => true if strictly increasing`
`(> <value> ...<value>) => true if strictly decreasing`
`(<= <value> ...<value>) => true if non-decreasing`
`(>= <value> ...<value>) => true if non-increasing`

`<value>` can be any of this:
- `number`
- `string`
- `pair` - compares both sides recursively
- `quote` - compares stored values

### eq?
Applies identity check between each consequence pairs.
`(eq? <value> ...<value>) => true if all equal`

Examples:
`(eq? #t #t) => true`
`(eq? #t #f) => false`
`(eq? 1 1) => true`
`(eq? 1 1.0) => false`
`(eq? "a" "a") => false`
`(eq? '(1 2) '(1 2)) => false`
`(define a '(1 2)) (eq? a a) => true`
`(eq? ''1 ''1) => false`
`(define a ''1) (eq? a a) => true`
`(eq? if if) => true`
`(eq? (lambda (a) a) (lambda (a) a)) => false`
`(define b (lambda (a) a)) (eq? b b) => true`
`(eq? 'a 'a) => true`
`(define a 1) (define b 1) (eq? a b) => false`

### nil?, symbol?, pair?, list?, bool?, number?, integer?, fractional?, string?, quote?, procedure?, special-form?, builtin?, closure?
Type checks first argument.
`(<type>? <value>) => bool`

### not
Returns `false` if argument is truthy, `true` if falsy.
`(not <value>) => true | false`

## Miscellaneous Builtins

### FD
`stdout` | `stderr` | `stdin` | `<fd-id>`

### print
Print arguments to file.
`(print '<FD>? ...<argument>) => #nil`

### print-flash
Flash file descriptor.
`(print-flash '<FD>?) => #nil`

### println
Print arguments to file and newline.
`(println '<FD>? ...<argument>) => #nil`

### printf
Print arguments to file.
`(print '<FD>? <format-string> ...<argument>) => #nil`
