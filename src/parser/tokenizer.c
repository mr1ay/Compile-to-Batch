#ifndef TOKENIZER_H
#define TOKENIZER_H

#include "../std/Str.h"
#include "../std/panic.c"
#include <ctype.h>
#include <stdbool.h>
#include <stddef.h>

typedef struct {
  Str data;
  size_t cur;
  size_t line;
  size_t col;
} TokenIterator;

typedef enum {
  TokenType_EOF = 0,
  TokenType_Ident,
  TokenType_Number,
  TokenType_OpenParen,
  TokenType_CloseParen,
  TokenType_Semi,
  TokenType_Comma,
  TokenType_String,
  TokenType_Unknown,
} TokenType;

typedef struct {
  TokenType type;
  union {
    Str ident;
    Str number;
    Str string;
    struct {
      size_t line;
      size_t col;
      char c;
    } unknown;
  };
} Token;

static void printToken(Token t) {
  switch (t.type) {
  case TokenType_EOF: {
    printf("(eof)\n");
  } break;
  case TokenType_Ident: {
    printf("Ident(%1.*s)\n", (int)t.ident.len, t.ident.ptr);
  } break;
  case TokenType_Number: {
    printf("Number(%1.*s)\n", (int)t.number.len, t.number.ptr);
  } break;
  case TokenType_OpenParen: {
    printf("OpenParen\n");
  } break;
  case TokenType_CloseParen: {
    printf("CloseParen\n");
  } break;
  case TokenType_Semi: {
    printf("Semi\n");
  } break;
  case TokenType_Comma: {
    printf("Comma\n");
  } break;
  case TokenType_String: {
    printf("String(\"%1.*s\")\n", (int)t.string.len, t.string.ptr);
  } break;
  case TokenType_Unknown: {
    printf("(unknown:%d:%d: '%c')\n", (int)t.unknown.line, (int)t.unknown.col,
           t.unknown.c);
  } break;
  }
}

static void updateLocationInfo(TokenIterator *it, char c) {
  if (c == '\n') {
    it->col = 0;
    it->line++;
  } else {
    it->col++;
  }
}

static bool tokenizerEnded(TokenIterator *it) {
  return it->cur >= it->data.len;
}

static void resetTokenizer(TokenIterator *it) {
  it->cur = 0;
  it->line = 1;
  it->col = 1;
}

static char nextChar(TokenIterator *it) {
  char c = it->data.ptr[it->cur++];
  updateLocationInfo(it, c);
  return c;
}

static Token nextToken(TokenIterator *it) {
  if (tokenizerEnded(it)) {
    return (Token){.type = TokenType_EOF};
  }
  char c = nextChar(it);

  // skip whitespace
  while (isblank(c) || c == '\n') {
    if (tokenizerEnded(it)) {
      return (Token){.type = TokenType_EOF};
    }
    c = nextChar(it);
  }

  switch (c) {
  case '(':
    return (Token){.type = TokenType_OpenParen};
  case ')':
    return (Token){.type = TokenType_CloseParen};
  case ';':
    return (Token){.type = TokenType_Semi};
  case ',':
    return (Token){.type = TokenType_Comma};
  }

  // keywords / identifiers
  if (isalpha(c)) {
    size_t start = it->cur - 1;
    while (isalpha(c)) {
      if (tokenizerEnded(it)) {
        break;
      }
      c = nextChar(it);
    }
    it->cur--;

    return (Token){.type = TokenType_Ident,
                   .ident = {
                       .ptr = it->data.ptr + start,
                       .len = it->cur - start,
                   }};
  }

  // numeric literal
  if (isdigit(c)) {
    size_t start = it->cur - 1;
    while (isdigit(c)) {
      if (tokenizerEnded(it)) {
        break;
      }
      c = nextChar(it);
    }
    it->cur--;
    return (Token){.type = TokenType_Number,
                   .number = {
                       .ptr = it->data.ptr + start,
                       .len = it->cur - start,
                   }};
  }

  if (c == '"') {
    size_t start = it->cur;
    bool escape = false;
    do {
      if (tokenizerEnded(it)) {
        panic("Tokenizer: Unterminated string literal");
      }
      c = nextChar(it);
      if (escape) {
        if (tokenizerEnded(it)) {
          panic("Tokenizer: Unterminated string literal");
        }
        c = nextChar(it);
        escape = false;
      } else if (c == '\\') {
        escape = true;
      }
    } while (escape || c != '"');

    return (Token){.type = TokenType_String,
                   .string = {
                       .ptr = it->data.ptr + start,
                       .len = it->cur - 1 - start,
                   }};
  }

  return (Token){.type = TokenType_Unknown,
                 .unknown = {
                     .line = it->line,
                     .col = it->col,
                     .c = c,
                 }};
}

static Token peekToken(TokenIterator *it) {
  size_t cur = it->cur;
  size_t line = it->line;
  size_t col = it->col;
  Token t = nextToken(it);
  it->cur = cur;
  it->line = line;
  it->col = col;
  return t;
}

#endif /* TOKENIZER_H */