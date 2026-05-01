# syx

`syx` is a small Lisp-like interpreter written in C. It is intended as a compact, self-contained runtime for experimenting with S-expression syntax, evaluation, and language design.

## Overview

The repository includes:

- `src/`: interpreter implementation and language runtime
- `src/main.c`: program entry point and CLI handling
- `vendor/`: bundled third-party headers and helpers
- `tests/`: language and runtime test cases
- `tests.c`: test runner implementation
- [`language.md`](language.md): design notes and language semantics
- [`todo.md`](todo.md): planned work and ongoing tasks

## Usage

This repository uses `nob` as the project helper.
After generating `./nob`, you can run commands like:

```sh
./nob
./nob run
./nob tests
./nob clean
```

`./nob` builds the project, runs the interpreter, executes tests, and manages generated artifacts.

## Contributing

If you want to explore or extend the language, start with `language.md` and `src/`.
Use `./nob tests` to verify behavior after making changes.
