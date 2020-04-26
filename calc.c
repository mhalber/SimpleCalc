/*
 This is a simple implementation of converting infix mathematical operations to postfix mathematical opeations
Here we follow the chapter 2 of the dragon book, implementing recursive descent parser.
 */

// TODO(maciej): Handle unknown math functions better
// TODO(maciej): Implement command line program
// TODO(maciej): Implement 4coder extension -> start line with //calc XXXXXX <- everything after 'calc' is expression to be evaluated.

// TODO(maciej): Fold the add/mul/exp functions into a single one that uses precendence table
// TODO(maciej): Modulus operator?
// TODO(maciej): Convert string to double?
// TODO(maciej): Math functions?

// TODO(maciej): Implement shunting yard (stack)
// TODO(maciej): Evaluate Shunting yard (stack) vs. recursive descent

////////////////////////////////

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

////////////////////////////////

int
is_digit(char c)
{
    return c >= '0' && c <= '9';
}

int
is_whitespace(char c)
{
    return ((c==' ') || (c=='\t') || (c=='\n') || (c=='\r') );
}

int
is_alpha(char c)
{
    return ((c>='a' && c<='z')||
            (c>='A' && c<='Z'));
}

int
str_equal( const char* str_a, const char* str_b, int str_len )
{
    for (int i = 0; i < str_len; ++i)
    {
        if (*(str_a + i) != *(str_b + i))
        {
            return 0;
        }
    }
    return 1;
}

////////////////////////////////

enum token_kind
{
    TOKEN_UNKNOWN = 0,
    
    TOKEN_NUMBER,
    TOKEN_PAREN_OPEN,
    TOKEN_PAREN_CLOSE,
    
    TOKEN_UNARY_MINUS,
    TOKEN_FUNCTION,
    TOKEN_ADD_OP,
    TOKEN_SUB_OP,
    TOKEN_MUL_OP,
    TOKEN_DIV_OP,
    TOKEN_EXP_OP,
    
    TOKEN_EOS
};

typedef struct token_t
{
    enum token_kind type;
    const char* str;
    int len;
    double value;
} token_t;

typedef struct tokenizer_t
{
    const char* at;
    const char* expression;
    token_t last_token;
} tokenizer_t;

typedef struct eval_ctx_t
{
    tokenizer_t tokenizer;
    
    token_t output_expr[1024];
    int expr_idx;
    int is_expr_valid;
} eval_ctx_t;

token_t
peek_token( tokenizer_t* tokenizer )
{
    if (tokenizer->last_token.type != TOKEN_UNKNOWN)
    {
        return tokenizer->last_token;
    }
    
    while (tokenizer->at[0] &&
           is_whitespace(tokenizer->at[0]))
    {
        tokenizer->at++;
    }
    const char* prev_at = tokenizer->at;
    
    token_t token = {0};
    token.str = tokenizer->at;
    token.len = 1;
    
    switch(tokenizer->at[0])
    {
        case '\0': { token.type = TOKEN_EOS; } break;
        case '(': { token.type = TOKEN_PAREN_OPEN; tokenizer->at++; } break;
        case ')': { token.type = TOKEN_PAREN_CLOSE; tokenizer->at++; } break;
        case '+': { token.type = TOKEN_ADD_OP; tokenizer->at++; } break;
        case '-': { token.type = TOKEN_SUB_OP; tokenizer->at++; } break;
        case '*': { token.type = TOKEN_MUL_OP; tokenizer->at++; } break;
        case '/': { token.type = TOKEN_DIV_OP; tokenizer->at++; } break;
        case '^': { token.type = TOKEN_EXP_OP; tokenizer->at++; } break;
        
        default:
        {
            if (tokenizer->at[0] && (is_digit(tokenizer->at[0]) || tokenizer->at[0] == '.' ))
            {
                while (tokenizer->at[0] && (is_digit(tokenizer->at[0]) || tokenizer->at[0] == '.' ))
                {
                    tokenizer->at++;
                }
                token.len = (int)(tokenizer->at - token.str);
                token.type = TOKEN_NUMBER;
            }
            
            if (tokenizer->at[0] && is_alpha(tokenizer->at[0]))
            {
                while (tokenizer->at[0] && is_alpha(tokenizer->at[0]))
                {
                    tokenizer->at++;
                }
                token.len = (int)(tokenizer->at - token.str);
                token.type = TOKEN_FUNCTION;
            }
        } break;
    }
    
    tokenizer->at = prev_at;
    tokenizer->last_token = token;
    
    return token;
}

int
match_token( tokenizer_t* tokenizer, enum token_kind token_type )
{
    token_t* lookahead = &tokenizer->last_token;
    if (lookahead->type == token_type)
    {
        tokenizer->at += lookahead->len;
        lookahead->type = TOKEN_UNKNOWN;
    }
    else { return 1; }
    return 0;
}

////////////////////////////////

enum error_code
{
    ERROR_OK = 0,
    ERROR_MISSING_CLOSE_PAREN,
    ERROR_MISSING_OPERATOR,
    ERROR_MISSING_OPERAND,
    ERROR_MISSING_FUNC_OPEN_PAREN,
    ERROR_MISSING_FUNC_ARG,
    ERROR_MISSING_FUNC_CLOSE_PAREN
};

static const char* error_str[] = {
    "[ERROR]: Ok",
    "[ERROR]: Missing close parenthesis",
    "[ERROR]: Missing operator",
    "[ERROR]: Missing operand",
    "[ERROR]: Missing function open parenthesis",
    "[ERROR]: Missing function argument",
    "[ERROR]: Missing function close parenthesis"
};

int
handle_error( tokenizer_t* tokenizer_local, token_t* lookahead, int error )
{
    if (error) {
        size_t n_spaces = lookahead->str - tokenizer_local->expression;
        printf("%s\n%s\n%*s^\n",
               error_str[error],
               tokenizer_local->expression,
               (int)n_spaces, "");
    }
    return error;
}

////////////////////////////////

int add_expr( eval_ctx_t* ctx );
int add_expr_tail(eval_ctx_t* ctx);
int mul_expr( eval_ctx_t* ctx );
int mul_expr_tail( eval_ctx_t* ctx );
int exp_expr( eval_ctx_t* ctx );
int exp_expr_tail( eval_ctx_t* ctx );
int unary_expr( eval_ctx_t* ctx );
int factor_expr( eval_ctx_t* ctx );

int factor_expr( eval_ctx_t* ctx )
{
    int error = 0;
    token_t lookahead = peek_token( &ctx->tokenizer );
    if (lookahead.type == TOKEN_PAREN_OPEN)
    {
        match_token( &ctx->tokenizer, TOKEN_PAREN_OPEN);
        error = add_expr( ctx );
        if (error) { return error; }
        error = match_token( &ctx->tokenizer, TOKEN_PAREN_CLOSE);
        if (error) {
            error = handle_error( &ctx->tokenizer, &lookahead, ERROR_MISSING_CLOSE_PAREN );
        }
    }
    else if (lookahead.type == TOKEN_NUMBER)
    {
        match_token( &ctx->tokenizer, TOKEN_NUMBER);
        lookahead.value = atof(lookahead.str);
        ctx->output_expr[ctx->expr_idx++] = lookahead;
    }
    else if (lookahead.type == TOKEN_FUNCTION)
    {
        match_token( &ctx->tokenizer, TOKEN_FUNCTION);
        token_t tmp = peek_token( &ctx->tokenizer );
        
        error = match_token( &ctx->tokenizer, TOKEN_PAREN_OPEN);
        if(error) {
            error = handle_error( &ctx->tokenizer, &tmp, ERROR_MISSING_FUNC_OPEN_PAREN );
            return error;
        }
        
        error = add_expr( ctx );
        if(error) { return error; }
        
        error = match_token( &ctx->tokenizer, TOKEN_PAREN_CLOSE);
        if(error) {
            (void) error;
            error = handle_error( &ctx->tokenizer, &tmp, ERROR_MISSING_FUNC_CLOSE_PAREN );
            return error;
        }
        ctx->output_expr[ctx->expr_idx++] = lookahead;
    }
    else
    {
        error =  handle_error( &ctx->tokenizer, &lookahead, ERROR_MISSING_OPERAND );
    }
    return error;
}


int unary_expr( eval_ctx_t* ctx )
{
    int error = 0;
    token_t lookahead = peek_token( &ctx->tokenizer );
    if (lookahead.type == TOKEN_SUB_OP)
    {
        match_token( &ctx->tokenizer, TOKEN_SUB_OP);
        error = factor_expr( ctx );
        if( error ) { return error; }
        lookahead.type = TOKEN_UNARY_MINUS;
        ctx->output_expr[ctx->expr_idx++] = lookahead;
    }
    else if (lookahead.type == TOKEN_NUMBER ||
             lookahead.type == TOKEN_PAREN_OPEN ||
             lookahead.type == TOKEN_FUNCTION)
    {
        error = factor_expr( ctx );
    }
    else
    {
        error = handle_error( &ctx->tokenizer, &lookahead, ERROR_MISSING_OPERAND );
    }
    return error;
}

int add_expr( eval_ctx_t* ctx )
{
    int error = mul_expr( ctx );
    
    if (!error)
    {
        error = add_expr_tail( ctx );
    }
    
    return error;
}

int add_expr_tail( eval_ctx_t* ctx )
{
    int error = 0;
    token_t lookahead = peek_token( &ctx->tokenizer );
    if (lookahead.type == TOKEN_ADD_OP )
    {
        match_token( &ctx->tokenizer, TOKEN_ADD_OP);
        error = mul_expr( ctx );
        if (error) { return error; }
        ctx->output_expr[ctx->expr_idx++] = lookahead;
        error = add_expr_tail( ctx );
    }
    else if (lookahead.type == TOKEN_SUB_OP )
    {
        match_token( &ctx->tokenizer, TOKEN_SUB_OP);
        error = mul_expr( ctx );
        if (error) { return error; }
        ctx->output_expr[ctx->expr_idx++] = lookahead;
        error = add_expr_tail( ctx );
    }
    else if (lookahead.type == TOKEN_FUNCTION ||
             lookahead.type == TOKEN_NUMBER)
    {
        error = handle_error( &ctx->tokenizer, &lookahead, ERROR_MISSING_OPERATOR );
    }
    
    return error;
}

int mul_expr( eval_ctx_t* ctx )
{
    int error = exp_expr( ctx );
    
    if( !error) {
        error = mul_expr_tail( ctx );
    }
    
    return error;
}

int mul_expr_tail( eval_ctx_t* ctx )
{
    int error = 0;
    token_t lookahead = peek_token( &ctx->tokenizer );
    if (lookahead.type == TOKEN_MUL_OP )
    {
        match_token( &ctx->tokenizer, TOKEN_MUL_OP);
        error = exp_expr( ctx );
        if (error) { return error; }
        ctx->output_expr[ctx->expr_idx++] = lookahead;
        error = mul_expr_tail( ctx );
    }
    if (lookahead.type == TOKEN_DIV_OP )
    {
        match_token( &ctx->tokenizer, TOKEN_DIV_OP);
        error = exp_expr( ctx );
        if (error) { return error; }
        ctx->output_expr[ctx->expr_idx++] = lookahead;
        error = mul_expr_tail( ctx );
    }
    return error;
}


int exp_expr( eval_ctx_t* ctx )
{
    int error = unary_expr( ctx );
    
    if( !error) {
        error = exp_expr_tail( ctx );
    }
    
    return error;
}

int exp_expr_tail(  eval_ctx_t* ctx )
{
    int error = 0;
    token_t lookahead = peek_token( &ctx->tokenizer );
    if (lookahead.type == TOKEN_EXP_OP )
    {
        error = match_token( &ctx->tokenizer, TOKEN_EXP_OP);
        exp_expr( ctx );
        ctx->output_expr[ctx->expr_idx++] = lookahead;
        exp_expr_tail( ctx );
    }
    return error;
}

int to_postfix( eval_ctx_t* ctx )
{
    return add_expr( ctx );
}

double
apply_function( const char* func_name, int func_name_len, double argument )
{
    if (str_equal(func_name, "sqrt", func_name_len)) {
        return sqrt(argument);
    }
    else if (str_equal(func_name, "exp", func_name_len)) {
        return exp(argument);
    }
    else if (str_equal(func_name, "log", func_name_len)) {
        return log(argument);
    }
    else if (str_equal(func_name, "sin", func_name_len)) {
        return sin(argument);
    }
    else if (str_equal(func_name, "cos", func_name_len)) {
        return cos(argument);
    }
    else if (str_equal(func_name, "tan", func_name_len)) {
        return tan(argument);
    }
    else if (str_equal(func_name, "acos", func_name_len)) {
        return acos(argument);
    }
    else if (str_equal(func_name, "atan", func_name_len)) {
        return atan(argument);
    }
    else if (str_equal(func_name, "asin", func_name_len)) {
        return asin(argument);
    }
    else
    {
        return sqrt(-1);
    }
}

int
reduce_postfix_expression( eval_ctx_t* ctx, int offset)
{
    token_t* operation = ctx->output_expr + offset;
    // Find operation
    while (operation->type < TOKEN_UNARY_MINUS ||
           operation->type > TOKEN_EXP_OP)
    {
        operation++;
        offset++;
    }
    
    // Get operand
    token_t* operand_a = operation - 2;
    while( operand_a->type != TOKEN_NUMBER) { operand_a--; }
    token_t* operand_b = operation - 1;
    
    // Perform calculation
    switch( operation->type )
    {
        case TOKEN_ADD_OP:
        {
            operation->value = operand_a->value + operand_b->value;
        } break;
        
        case TOKEN_SUB_OP:
        {
            operation->value = operand_a->value - operand_b->value;
        } break;
        
        case TOKEN_MUL_OP:
        {
            operation->value = operand_a->value * operand_b->value;
        } break;
        
        case TOKEN_DIV_OP:
        {
            operation->value = operand_a->value / operand_b->value;
        } break;
        
        case TOKEN_EXP_OP:
        {
            operation->value = pow(operand_a->value, operand_b->value);
        } break;
        
        case TOKEN_UNARY_MINUS:
        {
            operand_a = NULL;
            operation->value = -operand_b->value;
        } break;
        
        case TOKEN_FUNCTION:
        {
            operand_a = NULL;
            operation->value = apply_function(operation->str, operation->len, operand_b->value);
        } break;
        
        default:
        break;
    }
    
    // Clear tree for future reduction
    operation->type = TOKEN_NUMBER;
    if(operand_a) { operand_a->type = TOKEN_UNKNOWN; }
    if(operand_b) { operand_b->type = TOKEN_UNKNOWN; }
    
    return offset;
}

double
evaluate_postfix_expression( eval_ctx_t* ctx )
{
    double result = sqrt(-1);
    if( ctx->is_expr_valid )
    {
        int reduce_location = 0;
        while (ctx->output_expr[ctx->expr_idx-1].type != TOKEN_NUMBER)
        {
            reduce_location = reduce_postfix_expression( ctx, reduce_location );
        }
        result = ctx->output_expr[ctx->expr_idx-1].value;
    }
    return result;
}

////////////////////////////////

void
print_token_stream( token_t* tokens, int token_count )
{
    for (int i = 0; i < token_count; ++i)
    {
        token_t* token = tokens + i;
        switch( token->type )
        {
            case TOKEN_ADD_OP:
            {
                printf("{+}");
            }break;
            
            case TOKEN_SUB_OP:
            {
                printf("{-}");
            }break;
            
            case TOKEN_MUL_OP:
            {
                printf("{*}");
            }break;
            
            case TOKEN_DIV_OP:
            {
                printf("{/}");
            }break;
            
            case TOKEN_UNARY_MINUS:
            {
                printf("{!}");
            }break;
            
            case TOKEN_UNKNOWN:
            {
                printf("{0}");
            }break;
            
            case TOKEN_NUMBER:
            {
                printf("{%f}", token->value);
            }break;
            
            case TOKEN_FUNCTION:
            {
                printf("{%.*s}", token->len, token->str);
            }break;
            
            case TOKEN_EOS:
            {
                printf("{EOS}");
            } break;
            
            default:
            break;
        }
    }
    printf("\n");
}

int
test_calculator(const char* expression, double expected_value)
{
    eval_ctx_t eval_ctx =
    {
        .tokenizer =
        {
            .at = expression,
            .expression = expression,
            .last_token = {0}
        },
        .expr_idx = 0,
        .output_expr = {0}
    };
    (void) eval_ctx;
    
    eval_ctx.is_expr_valid = !to_postfix( &eval_ctx );
    double result = evaluate_postfix_expression( &eval_ctx );
    if ( eval_ctx.is_expr_valid )
    {
        printf("%s = %f | %f\n", expression, result, expected_value );
    }
    double diff = fabs(expected_value-result);
    return (diff < 0.00001);
}

#define ENABLE_TESTING 0

int
main(int argc, char** argv)
{
    
    // Nothing to parse
    if (argc < 2 && !ENABLE_TESTING)
    {
        return 0;
    }
    
    // Merge command line expression
    char* expression = malloc(1024*128);
    memset(expression, 0, 1024*128);
    char* ep = expression;
    for( int i = 1; i < argc; ++i)
    {
        size_t n = sprintf(ep, "%s ", argv[i]);
        ep+=n;
    }
    
    eval_ctx_t eval_ctx =
    {
        .tokenizer =
        {
            .at = expression,
            .expression = expression,
            .last_token = {0}
        },
        .expr_idx = 0,
        .output_expr = {0}
    };
    
    eval_ctx.is_expr_valid = !to_postfix( &eval_ctx );
    double result = evaluate_postfix_expression( &eval_ctx );
    if ( eval_ctx.is_expr_valid )
    {
        printf("%s= %f\n", expression, result );
    }
    free(expression);
    
#if ENABLE_TESTING
    int success = 1;
    success &= test_calculator("sin(-0)", 0),
    success &= test_calculator("cos(0)", 1),
    success &= test_calculator("log(exp(12.0))", 12.0),
    success &= test_calculator("5^log(exp(2.0))", 25.0),
    success &= test_calculator("2^3", 8);
    success &= test_calculator("2^(1+2*1)", 8);
    success &= test_calculator("32-2^3*2", 16);
    success &= test_calculator("-1", -1 );
    success &= test_calculator("-(4+5)*((-5))", 45);
    success &= test_calculator("-(-1)", 1);
    success &= test_calculator("-1", -1);
    success &= test_calculator("2+2", 4 );
    success &= test_calculator("7 - 6/2", 4);
    success &= test_calculator("(13+2)*(2+6/3)", 60);
    success &= test_calculator(" (  7 - 6) / 2  ", 0.5);
    success &= test_calculator(" 2 + - 4", -2);
    printf("VALID EXPRESSIONS: %s\n\n", success ? "PASS" : "FAIL");
    
    success = 1;
    success &= !test_calculator("7+12 *(2+4 + (8-7)", 0);
    success &= !test_calculator(" 2  4", 0);
    success &= !test_calculator(" 2 ++ 4", 0);
    success &= !test_calculator(" 2 --- 4", 0);
    success &= !test_calculator(" 2 ** 4", 0);
    success &= !test_calculator(" 2 // (4)", 0);
    success &= !test_calculator(" sin43)", 0);
    success &= !test_calculator(" sin(43 + (12*2)", 0);
    success &= !test_calculator(" --4", 0);
    printf("INVALID EXPRESSIONS: %s\n\n", success ? " PASS" : "FAIL");
#endif
    return 0;
}