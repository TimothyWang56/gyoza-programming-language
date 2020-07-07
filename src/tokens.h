#ifndef TOKENS_SEEN
#define TOKENS_SEEN

typedef enum TokenType { IDEN, OP, NUM, STR } tokenType;

typedef struct Token {
    int line;
    tokenType type;
    char *content;

} token;


#endif
