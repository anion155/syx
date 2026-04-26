#ifndef EXPR_PRINT_H
#define EXPR_PRINT_H

#include <nob.h>
#include <magic.h>
#include "expr_ast.h"

void fprint_expr(FILE *f, Expr *expr);
#define print_expr(expr) fprint_expr(stdout, (expr))

void fdump__expr(FILE *f, Expr *expr, size_t current_indent, size_t next_indent);
#define fdump_expr(f, expr, ...) fdump__expr((f), (expr), WITH_TWO_DEFAULTS(0, 2, __VA_ARGS__))
#define dump_expr(expr, ...) fdump__expr(stdout, (expr), WITH_TWO_DEFAULTS(0, 2, __VA_ARGS__))

#endif // EXPR_PRINT_H

#if defined(EXPR_PRINT_IMPL) && !defined(EXPR_PRINT_IMPL_C)
#define EXPR_PRINT_IMPL_C

void fprint_expr(FILE *f, Expr *expr) {
  switch (expr->kind) {
    case EXPR_KIND_SYMBOL: {
      if (expr->symbol.guarded) {
        fprintf(f, "|%s|", expr->symbol.name);
      } else {
        fprintf(f, "%s", expr->symbol.name);
      }
    } break;
    case EXPR_KIND_NIL: {
      fprintf(f, "()");
    } break;
    case EXPR_KIND_PAIR: {
      fprintf(f, "(");
      Expr *it = expr;
      fprint_expr(f, it->pair.left);
      it = it->pair.right;
      while (it->kind == EXPR_KIND_PAIR) {
        fprintf(f, " ");
        fprint_expr(f, it->pair.left);
        it = it->pair.right;
      }
      if (it->kind != EXPR_KIND_NIL) {
        fprintf(f, " . ");
        fprint_expr(f, it);
      }
      fprintf(f, ")");
    } break;
    case EXPR_KIND_QUOTE: {
      fprintf(f, "'");
      fprint_expr(f, expr->quote);
    } break;
    case EXPR_KIND_BOOL: {
      if (expr->boolean) fprintf(f, "true");
      else fprintf(f, "false");
    } break;
    case EXPR_KIND_INTEGER: {
      String_View sv = stringify_int(expr->integer);
      fprintf(f, SV_Fmt, SV_Arg(sv));
    } break;
    case EXPR_KIND_REAL: {
      String_View sv = stringify_real(expr->real);
      fprintf(f, SV_Fmt, SV_Arg(sv));
    } break;
    case EXPR_KIND_STRING: {
      fprintf(f, "\"%s\"", expr->string);
    } break;
  }
}

void fdump__expr(FILE *f, Expr *expr, size_t current_indent, size_t next_indent) {
  if (current_indent) fprintf(f, "%*c", (int)current_indent, ' ');
  switch (expr->kind) {
    case EXPR_KIND_NIL: {
      fprintf(f, "NIL\n");
    } break;
    case EXPR_KIND_SYMBOL: {
      if (expr->symbol.guarded) {
        fprintf(f, "SYMBOL: |%s|\n", expr->symbol.name);
      } else {
        fprintf(f, "SYMBOL: %s\n", expr->symbol.name);
      }
    } break;
    case EXPR_KIND_PAIR: {
      fprintf(f, "PAIR:\n");
      fdump__expr(f, expr->pair.left, next_indent, next_indent + 2);
      fdump__expr(f, expr->pair.right, next_indent, next_indent + 2);
    } break;
    case EXPR_KIND_QUOTE: {
      fprintf(f, "QUOTE: ");
      fdump__expr(f, expr->quote, 0, next_indent);
    } break;
    case EXPR_KIND_BOOL: {
      fprintf(f, "BOOL: %s\n", expr->boolean ? "true" : "false");
    } break;
    case EXPR_KIND_INTEGER: {
      fprintf(f, "INTEGER: %lld\n", expr->integer);
    } break;
    case EXPR_KIND_REAL: {
      fprintf(f, "REAL: %Lf\n", expr->real);
    } break;
    case EXPR_KIND_STRING: {
      fprintf(f, "STRING: \"%s\"\n", expr->string);
    } break;
  }
}

#endif // EXPR_PRINT_IMPL
