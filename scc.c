#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// トークンの種類
typedef enum {
  	TK_RESERVED, // 記号
  	TK_NUM,      // 整数トークン
  	TK_EOF,      // 入力の終わりを表すトークン
} TokenKind;

typedef struct Token {
	TokenKind kind;    // 
	struct Token *next;// 次の入力トークン
  	int val;           // kindがTK_NUMの場合、その数値
  	char *str;         // トークン文字列
	int len;	   // トークンの長さ
}Token;

//入力プログラム（グローバル変数）
char *user_input;

// 現在注目しているトークン（グローバル変数）
Token *token;

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
void error_at(char *loc, char *fmt, ...) {
	va_list ap;
	va_start(ap,fmt);

	int pos = loc - user_input;
	fprintf(stderr, "%s\n", user_input);
	fprintf(stderr, "%*s", pos, ""); // pos個の空白を出力
	fprintf(stderr, "^ ");
	vfprintf(stderr, fmt, ap);
	fprintf(stderr, "\n");
	exit(1);
}

// 次のトークンが期待している記号のときには、トークンを1つ読み進めて
// 真を返す。それ以外の場合には偽を返す。
bool consume(char *op) {
  	if (token->kind != TK_RESERVED ||strlen(op) != token->len ||
	    memcmp(token->str, op, token->len))
	    return false;
  	token = token->next;
  	return true;
}

// 次のトークンが期待している記号のときには、トークンを1つ読み進める。
// それ以外の場合にはエラーを報告する。
void expect(char *op) {
  	if (token->kind != TK_RESERVED || strlen(op) != token->len ||
			memcmp(token->str, op, token->len))
		error_at(token->str,"expected %s", op);
  		token = token->next;
}

// 次のトークンが数値の場合、トークンを1つ読み進めてその数値を返す。
// それ以外の場合にはエラーを報告する。
int expect_number() {
  	if (token->kind != TK_NUM)
		error_at(token->str, "数ではありません");
  	int val = token->val;
  	token = token->next;
  	return val;
}

bool at_eof() {
  return token->kind == TK_EOF;
}

// 新しいトークンを作成してcurrentに繋げる
Token *new_token(TokenKind kind, Token *cur, char *str, int len) {
  	Token *tok = calloc(1, sizeof(Token));
  	tok->kind = kind;
  	tok->str = str;
	tok->len = len;
  	cur->next = tok;
  	return tok;
}


bool startswith(char *p, char *q) {
	return memcmp(p, q, strlen(q)) == 0;

}

//入力文字列をトークナイズしてそれを返す
Token *tokenize() {
	char *p = user_input;
  	Token head;
  	head.next = NULL;
  	Token *cur = &head;

  	while (*p) {
    	// 空白文字をスキップ
    	if (isspace(*p)) {
      		p++;
      		continue;
	}
        

	// ２文字のトークン
	if (startswith(p, "==") || startswith(p, "!=") ||
	    startswith(p, "<=") || startswith(p, ">=")) {
	cur = new_token(TK_RESERVED, cur, p, 2);
	p += 2;
	continue;
	}

	// １文字のトークン
	if (strchr("+-*/()<>", *p)) {
      		cur = new_token(TK_RESERVED, cur, p++, 1);
      		continue;
	}
	
	// 整数
    	if (isdigit(*p)) {
      		cur = new_token(TK_NUM, cur, p, 0);
		char *q = p;
      		cur->val = strtol(p, &p, 10);
		cur->len = p - q;
      		continue;
	}

    	error_at(p, "無効なトークンです");
  }

  	new_token(TK_EOF, cur, p, 0);
  	return head.next;
}



//　抽象構文木のノードの種類
typedef enum {
	ND_ADD, // +
	ND_SUB,	// -
	ND_MUL,	// *
	ND_DIV,	// /
	ND_EQ,	// ==
	ND_NE,	// !=
	ND_LT,	// <
	ND_LE,	// <=
	ND_NUM,	// int
} NodeKind;


typedef struct Node {
	NodeKind kind;
        struct Node *lhs; // Left-hand side
	struct Node *rhs; // Right-hand side
	int val;
} Node;

Node *new_node(NodeKind kind) {
	Node *node = calloc(1, sizeof(Node));
	node->kind = kind;
	return node;
}

Node *new_binary(NodeKind kind, Node *lhs, Node *rhs) {
	Node *node = new_node(kind);
	node->lhs = lhs;
	node->rhs = rhs;
	return node;
}

Node *new_num(int val) {
	Node *node = new_node(ND_NUM);
	node->val = val;
	return node;
}

Node *expr();         // expr       = equality
Node *equality();     // equality   = relational ("==" relational | "!=" relational)*
Node *relational();   // relational = add ("<" add | "<=" add | ">" add | ">=" add)*
Node *add();	      // add        = mul ("+" mul | "-" mul)*
Node *mul();          // mul        = unary ("*" unary | "/" unary)* 
Node *unary();        // unary      = ("+" | "-")? primary
Node *primary();      // primary    = num | "(" expr ")"

// expr       = equality
Node *expr() {
  return equality();

}

// equality   = relational ("==" relational | "!=" relational)*
Node *equality() {
  Node *node = relational();

  for (;;) {
    if (consume("=="))
      node = new_binary(ND_EQ, node, relational());
    else if (consume("!="))
      node = new_binary(ND_NE, node, relational());
    else
      return node;
  }
}

// relational = add ("<" add | "<=" add | ">" add | ">=" add)* 
Node *relational() {
	Node *node = add();

	for (;;) {
	   if (consume("<"))
             node = new_binary(ND_LT, node, add());
	   else if (consume("<="))
	     node = new_binary(ND_LE, node, add());
	   else if (consume(">"))
             node = new_binary(ND_LE, add(), node); // 両辺を入れ替えた"<"と考える
	   else if (consume(">="))
             node = new_binary(ND_LE, add(), node); // 両辺を入れ替えた">="と考える
	   else
		   return node;
	}
}

// add        = mul ("+" mul | "-" mul)*
Node *add() {
  Node *node = mul();

  for(;;) {
	  if (consume("+"))
	    node = new_binary(ND_ADD, node, mul());
	  else if (consume("-"))
	    node = new_binary(ND_SUB, node, mul());
	  else
	    return node;
  }
}

// mul        = unary ("*" unary | "/" unary)*
Node *mul() {
  Node *node = unary();

  for (;;) {
    if (consume("*"))
      node = new_binary(ND_MUL, node, unary());
    else if (consume("/"))
      node = new_binary(ND_DIV, node, unary());
    else
      return node;
  }
}

// unary      = ("+" | "-")? primary
Node *unary() {
	if (consume("+"))
		return primary(); // +xをxと考える
	if (consume("-"))
		return new_binary(ND_SUB, new_num(0), primary()); // -xを0-xと考える
	return primary();
}

// primary    = num | "(" expr ")"
Node *primary() {
  // トークンが"("なら、"(" expr ")"になる
  if (consume("(")) {
    Node *node = expr();
    expect(")");
    return node;
  }
 // 上のケースでなければnumになる
  return new_num(expect_number());
}

// スタックマシンの実装
void gen(Node *node) {
  if (node->kind == ND_NUM) {
    printf("  push %d\n", node->val);
    return;
  }

  gen(node->lhs);
  gen(node->rhs);

  printf("  pop rdi\n");
  printf("  pop rax\n");

  switch (node->kind) {
  case ND_ADD:
    printf("  add rax, rdi\n");
    break;
  case ND_SUB:
    printf("  sub rax, rdi\n");
    break;
  case ND_MUL:
    printf("  imul rax, rdi\n");
    break;
  case ND_DIV:
    printf("  cqo\n");
    printf("  idiv rdi\n");
    break;
  case ND_EQ:
    printf("  cmp rax, rdi\n");
    printf("  sete al\n");
    printf("  movzb rax, al\n");
    break;
  case ND_NE:
    printf("  cmp rax, rdi\n");
    printf("  setne al\n");
    printf("  movzb rax, al\n");
    break;
  case ND_LT:
    printf("  cmp rax, rdi\n");
    printf("  setl al\n");
    printf("  movzb rax, al\n");
    break;
  case ND_LE:
    printf("  cmp rax, rdi\n");
    printf("  setle al\n");
    printf("  movzb rax, al\n");
    break;
  }

  printf("  push rax\n");
}

int main(int argc, char **argv){
	if (argc != 2)
	 error("%s: 引数の個数が正しくないです", argv[0]);

	// トークナイズしてパース
	user_input = argv[1];
	token = tokenize();
        Node *node = expr();

	// アセンブリの前半部分を出力
	printf(".intel_syntax noprefix\n");
 	printf(".globl main\n");
  	printf("main:\n");

	gen(node);
	// スタックトップの値＝計算結果をRAXにロードして返り値にする
        printf("  pop rax\n");
	printf("  ret\n");
  	return 0;
}
