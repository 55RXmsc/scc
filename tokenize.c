#include "scc.h"

static char *current_input;

// エラーを報告するための関数
// printfと同じ引数をとる
void error(char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}

//エラー箇所を報告する
static void verror_at(char *loc, char *fmt, va_list ap) {
    int pos = loc - current_input;
    fprintf(stderr, "%s\n", current_input);
    fprintf(stderr, "%*s", pos, ""); // pos個の空白を出力
    fprintf(stderr, "^ ");
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}
  
void error_at(char *loc, char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    verror_at(loc, fmt, ap);
}

void error_tok(Token *tok, char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    verror_at(tok->loc, fmt, ap);
}

// 
bool equal(Token *tok, char *op) {
    return memcmp(tok->loc, op, tok->len) == 0 && op[tok->len] == '\0';
}



Token *skip(Token *tok, char *op) {
    if (!equal(tok, op))
      error_tok(tok, "expected '%s'", op);
    return tok->next;
}

static Token *new_token(TokenKind kind, char *start, char *end) {
    Token *tok = calloc(1, sizeof(Token));
    tok->kind = kind;
    tok->loc = start;
    tok->len = end - start;
    return tok;
}

static bool startswith(char *p, char *q) {
    return strncmp(p, q, strlen(q)) == 0;
}

static int read_punct(char *p) {
    if (startswith(p, "==") || startswith(p, "!=") ||
        startswith(p, "<=") || startswith(p, ">="))
      return 2;
  
    return ispunct(*p) ? 1 : 0;
}

Token *tokenize(char *p) {
    current_input = p;
    Token head = {};
    Token *cur = &head;
  
    while (*p) {
      // 空白文字をスキップ
        if (isspace(*p)) {
            p++;
            continue;
        }
  
      // 整数
        if (isdigit(*p)) {
            cur = cur->next = new_token(TK_NUM, p, p); 
            char *q = p;
            cur->val = strtoul(p, &p, 10);
            cur->len = p - q;
            continue;
        }
  
      // 演算子
        int punct_len = read_punct(p);
        if (punct_len) {
            cur = cur->next = new_token(TK_PUNCT, p, p + punct_len); 
            p += cur->len;
            continue;
        }
  
      error_at(p, "invalid token");
    }
  
    cur = cur->next = new_token(TK_EOF, p, p);
    return head.next;
}