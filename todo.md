For a minimal but usable Lisp you need:
Special forms:

 <!-- - set! — mutate an existing binding (vs define which creates) -->
 <!-- - cond — can be a macro over if but painful without it -->
<!-- - and / or — short-circuiting, must be special forms not builtins -->

- let / let\* — can be sugar over lambda but very annoying without

Builtins:

- cons, car, cdr — list construction and access, fundamental
- eq?, equal? — identity vs structural equality
- null?, pair? — type checks
- not — boolean
- +, -, \*, / — arithmetic
- <, >, =, <=, >= — comparisons
- display / newline — output, you probably have something already

Nearly essential:

- apply — call a function with a list as arguments
- map — without this lists are painful
