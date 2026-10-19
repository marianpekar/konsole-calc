#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

typedef struct {
    char** items;
    size_t count;
    size_t cap;
} Tokens;

typedef struct Node {
    char op;
    double val;
    struct Node* left;
    struct Node* right;
} Node;

static void fail(const char* msg) {
    fprintf(stderr, "Error: %s\n", msg);
    exit(1);
}

static void* xmalloc(size_t n) {
    void* p = malloc(n);
    if (!p) {
        fail("malloc failed");
    }
    return p;
}

static void push_token(Tokens* tokens, const char* s, size_t len) {
    if (tokens->count == tokens->cap) {
        tokens->cap = tokens->cap ? tokens->cap*  2 : 16;
        tokens->items = realloc(tokens->items, tokens->cap*  sizeof(char*));
        if (!tokens->items) {
            fail("realloc failed");
        }
    }
    char* copy = xmalloc(len + 1);
    memcpy(copy, s, len);
    copy[len] = '\0';
    tokens->items[tokens->count++] = copy;
}

static int is_operator(char c) {
    return c != '\0' && strchr("+-*/%()^", c) != NULL;
}

static Tokens tokenize(const char* expr) {
    Tokens tokens = {0};
    size_t n = strlen(expr);
    char* buff = xmalloc(n+1);
    size_t blen = 0;

    for (size_t i = 0; i < n; ++i) {
        char c = expr[i];
        if (c == ' ') {
            if (blen > 0) {
                push_token(&tokens, buff, blen);
                blen = 0;
            }
        }
        else if (is_operator(c)) {
            if (blen > 0) {
                push_token(&tokens, buff, blen);
                blen = 0;
            }
            push_token(&tokens, &c, 1);
        }
        else {
            buff[blen++] = c;
        }
    }

    if (blen > 0) {
        push_token(&tokens, buff, blen);
    }

    free(buff);
    return tokens;
}

static Node* add_operand(double val) {
    Node* n = xmalloc(sizeof(Node));
    n->op = 0;
    n->val = val;
    n->left = n->right = NULL;
    return n;
}

static Node* add_operator(char op, Node* left, Node* right) {
    Node* n = xmalloc(sizeof(Node));
    n->op = op;
    n->val = 0;
    n->left = left;
    n->right = right;
    return n;
}

static void free_node(Node* n) {
    if (!n) return;
    free_node(n->left);
    free_node(n->right);
    free(n);
}

static double eval(const Node* n) {
    switch (n->op) {
        case 0: return n->val;
        case '+': return eval(n->left) + eval(n->right);
        case '-': return eval(n->left) - eval(n->right);
        case '*': return eval(n->left) * eval(n->right);
        case '/': return eval(n->left) / eval(n->right);
        case '%': return fmod(eval(n->left), eval(n->right));
        case '^': return pow(eval(n->left), eval(n->right));
        default: fail("unknown operator");
    }
    return 0;
}

static Tokens g_tokens;
static size_t g_pos = 0;

static Node* parse_subadd(void);
static Node* parse_divmulmod(void);
static Node* parse_unary(void);
static Node* parse_exp(void);
static Node* parse_leaf(void);

static char accept_op(const char* ops) {
    if (g_pos >= g_tokens.count) return 0;

    const char* tok = g_tokens.items[g_pos];
    if (tok[1] == '\0' && strchr(ops, tok[0])) {
        g_pos++;
        return tok[0];
    }
    
    return 0;
}

static Node* parse_subadd(void) {
    Node* left = parse_divmulmod();
    char op;
    while ((op = accept_op("+-")) != 0) {
        Node* right = parse_divmulmod();
        left = add_operator(op, left, right);
    }
    return left;
}

static Node* parse_divmulmod(void) {
    Node* left = parse_unary();
    char op;
    while ((op = accept_op("*/%")) != 0) {
        Node* right = parse_unary();
        left = add_operator(op, left, right);
    }
    return left;
}

static Node* parse_unary(void) {
    if (accept_op("-")) {
        Node* operand = parse_unary();
        return add_operator('-', add_operand(0), operand);
    }
    else if (accept_op("+")) {
        Node* operand = parse_unary();
        return add_operator('+', add_operand(0), operand);
    }
    return parse_exp();
}

static Node* parse_exp(void) {
    Node* left = parse_leaf();
    char op;
    while ((op = accept_op("^")) != 0) {
        Node* right = parse_unary();
        return add_operator('^', left, right);
    }
    return left;
}

static Node* parse_leaf(void) {
    if (g_pos >= g_tokens.count) {
        fail("invalid expression");
    }

    const char* tok = g_tokens.items[g_pos++];

    if (strcmp(tok, "(") == 0) {
        Node* node = parse_subadd();
        if (g_pos >= g_tokens.count || strcmp(g_tokens.items[g_pos++], ")") != 0) {
            fail("missing closing parenthesis");
        }
        return node;
    }

    char* end;
    double val = strtod(tok, &end);
    if (end == tok ||* end != '\0') {
        fail("invalid token");
    }
    return add_operand(val);
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s \"expression\" (e.g. 1+1)\n", argv[0]);
        return 1;
    }

    size_t total = 1;
    for (int i = 1; i < argc; ++i) total += strlen(argv[i]) + 1;
    char* expr = xmalloc(total);
    expr[0] = '\0';
    for (int i = 1; i < argc; ++i) {
        if (i > 1) strcat(expr, " ");
        strcat(expr, argv[i]);
    }

    g_tokens = tokenize(expr);
    free(expr);

    if (g_tokens.count == 0) {
        fprintf(stderr, "Usage: %s \"expression\" (e.g. 1+1)\n", argv[0]);
        return 1;
    }

    Node* root = parse_subadd();
    if (g_pos != g_tokens.count) {
        fail("unexpected token");
    }

    printf("%.15g\n", eval(root));

    free_node(root);
    for (size_t i = 0; i < g_tokens.count; ++i) {
        free(g_tokens.items[i]);
    }
    free(g_tokens.items);

    return 0;
}