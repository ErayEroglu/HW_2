#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#define HASH_SIZE 256 // size of a hashtable

// structure definitions

typedef enum // Token types which will be needed in lexical analysis
{
    CONST,
    VAR,
    COMMA,
    ADDITION,   
    SUBTRACTION,
    MULTIPLICATION,
    AND,
    OR,
    XOR,
    EQUAL,
    L_SHIFT,
    R_SHIFT,
    L_ROTATE,
    R_ROTATE,
    NOT,
    L_PAREN,
    R_PAREN,
    EOI,
} TokenType;

typedef struct // Token structure to create tokens
{              // Each token has 4 members
    TokenType type;
    long long int number;
    char *id;
    char *name;
} Token;

typedef struct Node // Node structure, it will be used in creating parse trees
{                   // Each node keeps track of its left/right children and has 3 other members
    TokenType op;
    long long int *value;
    char *name;
    struct Node *left;
    struct Node *right;
} Node;

typedef struct Variable // struct for a hashtable which will store data for variables
{
    long long int data;
    char *key;
} Variable;

// global variables and method declarations

Token *createToken(char *inp_s, int *token_number);
long long int evaluate(Node *nodeP);
Node *createNode(Token *token, Node *left, Node *right);
Node *constructNode(TokenType op, long long int *value, char *name, Node *left, Node *right);
Node *parseF(Token *ptoken_list, int *pos);
Node *parseFnc(Token *ptoken_list, int *pos);
Node *parseT(Token *ptoken_list, int *pos);
Node *parseE(Token *ptoken_list, int *pos);
Node *parseB(Token *ptoken_list, int *pos);
Node *parse(Token *ptoken_list, int *pos);
Variable *hashMap[HASH_SIZE];
unsigned int hashFunction(char *s);
bool printFlag = true;  // a boolean checker to print the result unless it is an equation
bool errorFlag = false; // another boolean checker, it controls whether the input has a syntax error or not
int num_tokens;

void main()
{
    Token *tokens = NULL; // a pointer which points to list of the tokenized form of given input
    while (true)
    {
        int position = 0; // an int variable to keep the index of position during the parsing operations
        int *ppos = &position;
        char expr[256]; // input is stored in char array
        char* s = fgets(expr, 256, stdin);
        if (s[strlen(s) - 2] != '\n') 
        {
            printf("\n");
        }
        if (s == NULL) 
        {   
            break;
        }
        
        num_tokens = strlen(expr);
        Token *tokens = createToken(expr, &num_tokens); // converts the given string to list of tokens

        if (num_tokens == 0) // if there is not any token in the input, do nothing
        {
            continue;
        }

        if (tokens == NULL) // if the input is consisted of unknown chars, there is an error
        {
            printf("Error!\n");
            errorFlag = false;
            printFlag = true;
            continue;
        }

        Node *pnode = parse(tokens, ppos); // calls the primary parsing method
        if (!errorFlag)
        {
            long long int res = evaluate(pnode); // calls the method which evaluates the tree
            if (printFlag)
            {
                printf("%lld\n", res);
            }
        }
        else
        {
            printf("Error!\n");
        }

        printFlag = true;
        errorFlag = false;
        
    }
    free(tokens); // frees the memory
}

// helper methods

unsigned int hashFunction(char *p) // generates hash position for the given variable
{                                  // uses unsigned int to avoid negative values
    char *s = NULL;
    s = p;
    unsigned int hashval;
    for (hashval = 0; *s != '\0'; s++)
        hashval = *s + 31 * hashval;
    return hashval % HASH_SIZE;
}

Variable *search(char *pkey) // searches for the var name, if it exists returns the variable
{
    int key = hashFunction(pkey);
    while (hashMap[key] != NULL)
    {
        if (strcmp(hashMap[key]->key, pkey) == 0)
        {
            return hashMap[key];
        }
        key++;

        key %= HASH_SIZE;
    }
    return NULL;
}

Variable *createVar(char *key, long long int data) // method to create variable
{
    Variable *var = malloc(sizeof(Variable)); // creates memory for the var
    var->data = data;
    var->key = strdup(key);
    return var;
}

void insert(char *key, long long int data) // inserting function for hashmap
{

    Variable *var = createVar(key, data);
    int hash_pos = hashFunction(key);
    while (hashMap[hash_pos] != NULL)
    {
        hash_pos++;
        hash_pos = hash_pos % HASH_SIZE;
    }

    hashMap[hash_pos] = var;
}

Token *createToken(char *inp_s, int *token_number) // creates token according to the given input string
{                                                  // returns the list of tokens
    int found_tokens = 0;
    int length = strlen(inp_s);
    char *pcurrent_char = inp_s;
    Token *token_list = malloc(length * sizeof(Token)); // creates sufficient memory for tokens

    while (*pcurrent_char != '\0') // iterates until reaching the end of input
    {
        switch (*pcurrent_char) // creates tokens according to current char
        {
        case ' ':
            pcurrent_char++;
            break;
        case '\t':
            pcurrent_char++;
            break;
        case '\n':
            pcurrent_char++;
            break;
        case '+':
            token_list[found_tokens].type = ADDITION;
            token_list[found_tokens].id = "ADDITION";
            found_tokens++;
            pcurrent_char++;
            break;
        case '-':
            token_list[found_tokens].type = SUBTRACTION;
            token_list[found_tokens].id = "SUBTRACTION";
            found_tokens++;
            pcurrent_char++;
            break;
        case '*':
            token_list[found_tokens].type = MULTIPLICATION;
            token_list[found_tokens].id = "MULTIPLICATION";
            found_tokens++;
            pcurrent_char++;
            break;
        case '&':
            token_list[found_tokens].type = AND;
            token_list[found_tokens].id = "AND";
            found_tokens++;
            pcurrent_char++;
            break;
        case '|':
            token_list[found_tokens].type = OR;
            token_list[found_tokens].id = "OR";
            found_tokens++;
            pcurrent_char++;
            break;
        case '(':
            token_list[found_tokens].type = L_PAREN;
            token_list[found_tokens].id = "L_PAREN";
            found_tokens++;
            pcurrent_char++;
            break;
        case ')':
            token_list[found_tokens].type = R_PAREN;
            token_list[found_tokens].id = "R_PAREN";
            found_tokens++;
            pcurrent_char++;
            break;
        case '%':
            while (*pcurrent_char != '\0')
            {
                pcurrent_char++;
            };
            break;
        case '=':
            token_list[found_tokens].type = EQUAL;
            token_list[found_tokens].id = "EQUAL";
            found_tokens++;
            pcurrent_char++;
            break;
        case ',':
            token_list[found_tokens].type = COMMA;
            token_list[found_tokens].id = "COMMA";
            found_tokens++;
            pcurrent_char++;
            break;
        default:
            if (isdigit(*pcurrent_char))
            {
                long long num = strtoll(pcurrent_char, NULL, 10); // converts the string to long long int
                pcurrent_char++;
                while (isdigit(*pcurrent_char)) // if it is a number, it might be consisted of more than one digit
                {                               // so it will be iterated until reaching a non-digit char
                    pcurrent_char++;
                }
                token_list[found_tokens].type = CONST;
                token_list[found_tokens].id = "CONST";
                token_list[found_tokens].number = num;
                found_tokens++;
            }

            else if (isalpha(*pcurrent_char)) // if it is a word, it might be consisted of more than one letter
            {
                char char_name[256];
                char_name[0] = *pcurrent_char;
                int index = 1;
                pcurrent_char++;
                while (isalpha(*pcurrent_char))
                {
                    char_name[index] = *pcurrent_char;
                    pcurrent_char++;
                    index++;
                }
                char_name[index] = '\0';           // puts null char at the end of string
                if (strcmp(char_name, "xor") == 0) // if it is a special function name, creates the special token
                {
                    token_list[found_tokens].type = XOR;
                    token_list[found_tokens].id = "XOR";
                }

                else if (strcmp(char_name, "ls") == 0)
                {
                    token_list[found_tokens].type = L_SHIFT;
                    token_list[found_tokens].id = "L_SHIFT";
                }

                else if (strcmp(char_name, "rs") == 0)
                {
                    token_list[found_tokens].type = R_SHIFT;
                    token_list[found_tokens].id = "R_SHIFT";
                }

                else if (strcmp(char_name, "lr") == 0)
                {
                    token_list[found_tokens].type = L_ROTATE;
                    token_list[found_tokens].id = "L_ROTATE";
                }

                else if (strcmp(char_name, "rr") == 0)
                {
                    token_list[found_tokens].type = R_ROTATE;
                    token_list[found_tokens].id = "R_ROTATE";
                }

                else if (strcmp(char_name, "not") == 0)
                {
                    token_list[found_tokens].type = NOT;
                    token_list[found_tokens].id = "NOT";
                }

                else
                {
                    token_list[found_tokens].type = VAR; // if it is not a function name, than it is a variable name
                    token_list[found_tokens].id = "VAR";
                    token_list[found_tokens].name = strdup(char_name);
                    token_list[found_tokens].number = 0;
                }
                found_tokens++;
            }
            else // if the current character of string is unknown,then there is an error
            {
                errorFlag = true;
                return NULL;
            }
            break;
        }
    }
    *token_number = found_tokens;

    // reallocates the memory to avoid memory leak
    token_list = (Token*) realloc(token_list,(*token_number) * sizeof(Token));  // at first this memory was equal to length of array * size of token 
    return token_list; // returns the list of tokens                            
}

Node *constructNode(TokenType op, long long int *value, char *name, Node *left, Node *right) // makes the adjustments for a node
{
    Node *node = malloc(sizeof(Node)); // allocates the memory for a node
    node->op = op;
    node->value = malloc(sizeof(long long int)); // allocates the memory for node value
    *(node->value) = *value;
    node->name = strdup(name);
    node->left = left;
    node->right = right;
    return node;
}

Node *createNode(Token *token, Node *left, Node *right) // creates nodes for parsing tree
{                                                       // calls the constructNode method to reach data and allocate memory

    TokenType op = token->type;
    long long int *value = malloc(sizeof(long long int));
    *value = token->number;
    if (op == VAR)
    {
        char *name = strdup(token->name);
        return constructNode(op, value, name, left, right);
    }
    Node *node = constructNode(op, value, "", left, right);
    return node;
}

// parsing functions

Node *parse(Token *ptoken_list, int *pos) // main parsing method, calls parseB
{                                         // looks for = operation, if it exists creates the node
    Node *temp = parseB(ptoken_list, pos);

    // error check
    if (temp == NULL)  // if other parsing functions return NULL, there must be something wrong
    {
        errorFlag = true;
        return NULL;
    }
    if (ptoken_list[*pos].type == EQUAL)
    {
        if (temp->op != VAR || ptoken_list[0].type == L_PAREN)
        {
            errorFlag = true;
            return NULL;
        }

        Token *op_token = &(ptoken_list[*pos]);
        (*pos)++;

        if (*pos == num_tokens)  // if number of tokens is equal to the index of token list, that means the rest of equation is missing
        {                        // it is a different error check
            errorFlag = true;
            return NULL;
        }

        Node *temp2 = parseB(ptoken_list, pos);

        if (temp == NULL)  // similar with the first error check
        {
            errorFlag = true;
            return NULL;
        }
        temp = createNode(op_token, temp, temp2);
    }
    if (*pos < num_tokens)  // if at the end of assignment operation we didn't reach the last pos of token list, that means an error
    {
        errorFlag = true;
        return NULL;
    }

    return temp;
}

Node *parseB(Token *ptoken_list, int *pos) // looks for bitwise and ,and bitwise or operations, after that creates the nodes
{                                          // error checks are very similar
    Node *bitwise = parseE(ptoken_list, pos);

    // error check
    if (bitwise == NULL)
    {
        errorFlag = true;
        return NULL;
    }
    while (ptoken_list[*pos].type == AND || ptoken_list[*pos].type == OR)
    {
        Token *op_token = &(ptoken_list[*pos]);
        (*pos)++;
        if (*pos == num_tokens)
        {
            errorFlag = true;
            return NULL;
        }
        Node *bitwise_2 = parseE(ptoken_list, pos);
        if (bitwise_2 == NULL)
        {
            // error check
            errorFlag = true;
            return NULL;
        }
        bitwise = createNode(op_token, bitwise, bitwise_2);
    }
    return bitwise;
}

Node *parseE(Token *ptoken_list, int *pos) // parses expression into terms, looks for + and - operations
{                                          // starts with calling parseT
    Node *parsing_term = parseT(ptoken_list, pos);

    // error check
    if (parsing_term == NULL)
    {
        errorFlag = true;
        return NULL;
    }

    while (ptoken_list[*pos].type == ADDITION || ptoken_list[*pos].type == SUBTRACTION)
    {
        Token *op_token = &(ptoken_list[*pos]);
        (*pos)++;

        // error check
        if (*pos == num_tokens)
        {
            errorFlag = true;
            return NULL;
        }

        Node *parsing_term2 = parseT(ptoken_list, pos);

        if (parsing_term2 == NULL)
        {
            // error check
            errorFlag = true;
            return NULL;
        }
        parsing_term = createNode(op_token, parsing_term, parsing_term2);
    }

    return parsing_term;
}

Node *parseT(Token *ptoken_list, int *pos) // parses term into factors, looks for multiplication operation
{                                          // starts with calling parseF
    Node *parsing_factor = parseFnc(ptoken_list, pos);

    // error check
    if (parsing_factor == NULL)
    {
        errorFlag = true;
        return NULL;
    }

    while (ptoken_list[*pos].type == MULTIPLICATION)
    {
        Token *op_token = &(ptoken_list[*pos]);

        (*pos)++;

        // error check
        if (*pos == num_tokens)
        {
            errorFlag = true;
            return NULL;
        }
        Node *parsing_factor2 = parseFnc(ptoken_list, pos);
        parsing_factor = createNode(op_token, parsing_factor, parsing_factor2);
    }
    return parsing_factor;
}
Node *parseFnc(Token *ptoken_list, int *pos) // looks for functions with two parameters
{
    Node *temp;
    if (
        ptoken_list[*pos].type == L_SHIFT ||
        ptoken_list[*pos].type == R_SHIFT ||
        ptoken_list[*pos].type == XOR ||
        ptoken_list[*pos].type == L_ROTATE ||
        ptoken_list[*pos].type == R_ROTATE)
    {
        Token *op_token = &(ptoken_list[*pos]);
        (*pos)++;

        if (ptoken_list[*pos].type == L_PAREN)
        {
            (*pos)++;
        }
        else // if there is not (, there is an error
        {
            errorFlag = true;
            return NULL;
        }

        Node *temp = parseB(ptoken_list, pos);

        // error check
        if (temp == NULL)
        {
            errorFlag = true;
            return NULL;
        }

        if (ptoken_list[*pos].type == COMMA)
        {
            (*pos)++;
        }
        else
        { // if there is not comma between first and second parameter, there is an error
            errorFlag = true;
            return NULL;
        }

        Node *temp_2 = parseB(ptoken_list, pos);

        // error check
        if (temp == NULL)
        {
            errorFlag = true;
            return NULL;
        }

        if (ptoken_list[*pos].type == R_PAREN)
        {
            (*pos)++;
        }
        else
        { // if we don't reach ) at the end of expression, that means there is an error
            errorFlag = true;
            return NULL;
        }

        temp = createNode(op_token, temp, temp_2);
        return temp;
    }

    else
    { // if there doesn't exist any function, proceeds with parseF
        temp = parseF(ptoken_list, pos);
        return temp;
    }
}

Node *parseF(Token *ptoken_list, int *pos) // parsing factor method
{

    if (ptoken_list[*pos].type == CONST) // if the current token matches the type, creates node
    {

        Node *temp = createNode(&(ptoken_list[*pos]), NULL, NULL);
        (*pos)++;
        return temp;
    }
    else if (ptoken_list[*pos].type == VAR)
    {
        insert(ptoken_list[*pos].name, ptoken_list[*pos].number);
        Node *temp = createNode(&(ptoken_list[*pos]), NULL, NULL);
        (*pos)++;
        return temp;
    }
    else if (ptoken_list[*pos].type == L_PAREN) // if token is l paren, it must be an expression inside of it
    {
        (*pos)++; // it moves the next token
        Node *temp = parseB(ptoken_list, pos);

        if (temp == NULL || temp->op == R_PAREN) // if there does't exist a statement, return null
        {
            errorFlag = true;
            return NULL;
        }
        else if (ptoken_list[*pos].type == R_PAREN)
        {
            (*pos)++;
            return temp;
        }
        else
        { // if it is not ), there must be something wrong
            return NULL;
        }
    }
    else if (ptoken_list[*pos].type == NOT) // if it is a not function, there should be an expression
    {
        Token *op_token = &(ptoken_list[*pos]);
        (*pos)++;

        if (ptoken_list[*pos].type == L_PAREN)
        {
            (*pos)++;
        }
        else
        { // if there is not (, error
            errorFlag = true;
            return NULL;
        }

        Node *temp = parseB(ptoken_list, pos);

        // error check
        if (temp == NULL)
        {
            errorFlag = true;
            return NULL;
        }

        if (ptoken_list[*pos].type == R_PAREN)
        {
            (*pos)++;
        }
        else
        { // at the end, we should reach )
            errorFlag = true;
            return NULL;
        }

        temp = createNode(op_token, temp, NULL);
        return temp;
    }
    else
    {
        return NULL;
    }
}

// method to evaluate the tree
long long int evaluate(Node *nodeP)
{
    // starts to evaluate from root node recursively

    // binary operations
    if (nodeP->op == ADDITION)
    {
        return evaluate(nodeP->left) + evaluate(nodeP->right);
    }
    else if (nodeP->op == SUBTRACTION)
    {
        return evaluate(nodeP->left) - evaluate(nodeP->right);
    }
    else if (nodeP->op == MULTIPLICATION)
    {
        return evaluate(nodeP->left) * evaluate(nodeP->right);
    }

    else if (nodeP->op == CONST)
    {
        return *(nodeP->value);
    }
    else if (nodeP->op == VAR)
    {
        Variable *var = search(nodeP->name);
        if (var != NULL)
        {
            return var->data;
        }
        return 0LL;
    }

    else if (nodeP->op == EQUAL) // if there exist an equation, no printing
    {
        Node *pLeft;
        pLeft = nodeP->left;
        Variable *pVar = search(pLeft->name);
        pVar->data = evaluate(nodeP->right);
        printFlag = false;
        return pVar->data;
    }

    // bitwise binary operations
    else if (nodeP->op == AND)
    {
        return evaluate(nodeP->left) & evaluate(nodeP->right);
    }
    else if (nodeP->op == OR)
    {
        return evaluate(nodeP->left) | evaluate(nodeP->right);
    }
    // function with one parameter
    else if (nodeP->op == NOT)
    {
        return ~(evaluate(nodeP->left));
    }

    // functions with two parameters
    else
    {
        Node *pLeft = nodeP->left;
        Node *pRight = nodeP->right;
        bool syntaxError = false;
        switch (nodeP->op)
        {
        case XOR:
            return evaluate(nodeP->left) ^ evaluate(nodeP->right);
            break;
        case L_SHIFT:
            return evaluate(nodeP->left) << evaluate(nodeP->right);
        case R_SHIFT:
            return evaluate(nodeP->left) >> evaluate(nodeP->right);
        case L_ROTATE:
            return (evaluate(nodeP->left) << evaluate(nodeP->right)) | (evaluate(nodeP->left) >> (64 - evaluate(nodeP->right)));
        case R_ROTATE:
            return (evaluate(nodeP->left) >> evaluate(nodeP->right)) | (evaluate(nodeP->left) << (64 - evaluate(nodeP->right)));
        default:
            break;
        }
    }
}
