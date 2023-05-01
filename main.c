#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <limits.h>
#define HASH_SIZE 256 // size of a hashtable

// structure definitions
// PATH ISLEMLERI

typedef enum // Token types which will be needed in lexical analysis
{
    CONST,
    VAR,
    COMMA,
    ADDITION,
    SUBTRACTION,
    MULTIPLICATION,
    DIVISION,
    MODULE,
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
    int number;
    char *id;
    char *name;
} Token;

typedef struct Node // Node structure, it will be used in creating parse trees
{                   // Each node keeps track of its left/right children and has 3 other members
    TokenType op;
    int *value;
    char *name;
    int dummyNo;
    struct Node *left;
    struct Node *right;
} Node;

typedef struct Variable // struct for a hashtable which will store data for variables
{
    int data;
    char *key;
} Variable;

// global variables and method declarations

Token *createToken(char *inp_s, int *token_number);
int evaluate(Node *nodeP);
Node *createNode(Token *token, Node *left, Node *right);
Node *constructNode(TokenType op, int *value, char *name, Node *left, Node *right);
Node *parseF(Token *ptoken_list, int *pos);
Node *parseFnc(Token *ptoken_list, int *pos);
Node *parseT(Token *ptoken_list, int *pos);
Node *parseE(Token *ptoken_list, int *pos);
Node *parseB(Token *ptoken_list, int *pos);
Node *parse(Token *ptoken_list, int *pos);
char **fileReader(char *path);
Variable *hashMap[HASH_SIZE];
unsigned int hashFunction(char *s);
void allocate(char *name);
void store(Node *node, char *name);
void load(Node *node);
void printProcess(Node *node, char *name);
void printRotate(Node *node, TokenType token);
bool printFlag = true;  // a boolean checker to print the result unless it is an equation
bool errorFlag = false; // another boolean checker, it controls whether the input has a syntax error or not
int num_tokens;
int dummyCounter = 1;
int dummyofChild(Node *node);
int *pdummyCounter = &dummyCounter; //pointer to a global dummycounter
FILE *pOutputFile;
bool deleteFlag = false; //boolean checker, if theres syntax error the output file is removed
bool isLeftHandSide; // boolean checker if a given variable is at the LHS of equals operator

void main(int argc, char *argv[])
{
    Token *tokens = NULL; // a pointer which points to list of the tokenized form of given input
    //input file from main args
    char *path = argv[1];
    FILE *pFile = fopen(path, "r");
    //updates output file from ./<input>.adv to ./<input>.ll
    char *output_path = malloc(strlen(path) + 1);
    strcpy(output_path, path);
    char *ext = strrchr(output_path, '.');
    if (ext == NULL) {
        ext = output_path + strlen(output_path);
    }
    strcpy(ext, ".ll");

    pOutputFile = fopen(output_path, "w");
    char pInpFile[10000][257];
    int line = 0;

    while (fgets(pInpFile[line], 10000, pFile) != NULL)
    {
        // Remove the newline character from the line
        char *pos = strchr(pInpFile[line], '\n');
        if (pos != NULL)
        {
            *pos = '\0';
        }
        line++;
    }
    fclose(pFile);
    
    //writes the beginnig of LLVM IR code to the output file
    fprintf(pOutputFile, "; ModuleID = 'advcalc2ir'\n");
    fprintf(pOutputFile, "declare i32 @printf(i8*, ...)\n");
    fprintf(pOutputFile, "@print.str = constant [4 x i8] c\"%%d\\0A\\00\"\n\n");
    fprintf(pOutputFile, "define i32 @main() {\n");


    //this while loop iterates through lines of inputfile    
    int index = -1;
    while (index < line)
    {
        index++;
        int position = 0; // an int variable to keep the index of position during the parsing operations
        int *ppos = &position;

        num_tokens = sizeof(pInpFile[index]) / sizeof(pInpFile[index][0]);
        Token *tokens = createToken(pInpFile[index], &num_tokens); // converts the given string to list of tokens

        if (num_tokens == 0) // if there is not any token in the input, do nothing
        {
            continue;
        }

        if (tokens == NULL) // if the input is consisted of unknown chars, there is an error
        {
            printf("Error on line %d!\n", index + 1);
            errorFlag = false;
            printFlag = true;
            deleteFlag = true;
            continue;
        }

        Node *pnode = parse(tokens, ppos); // calls the primary parsing method

        if (!errorFlag)
        {
            // if there are no syntax errors a
            if (pnode->op == EQUAL)
            {
                //if there is an assignment left hand side shouldnt be loaded so 
                //recursive load method starts from rightchild and loads every variable in there
                load(pnode->right);
            }
            else
            {
                //recursive load method loads every variable in an expression to a dummyvar
                load(pnode);
            }
            int res = evaluate(pnode); // calls the method which evaluates the tree
            if (printFlag)
            {   
                //if there is no assingment there should be a print 
                //so we write call statement to the output file and increment dummyCounter
                if(pnode->op == CONST){
                    fprintf(pOutputFile, "call i32 (i8*, ...) @printf(i8* getelementptr ([4 x i8], [4 x i8]* @print.str, i32 0, i32 0), i32 %d)\n", (*pnode->value));
                    (*pdummyCounter)++;
                }else{
                    fprintf(pOutputFile, "call i32 (i8*, ...) @printf(i8* getelementptr ([4 x i8], [4 x i8]* @print.str, i32 0, i32 0), i32 %%%d)\n", pnode->dummyNo);
                    (*pdummyCounter)++;
                }
            }
        }
        else
        {   
            //if there is an error on a line it is printed to termina lhere
            printf("Error on line %d!\n", index + 1);
            deleteFlag = true;
        }
        //flags are resetted for next iteration
        printFlag = true;
        errorFlag = false;
    }
    fprintf(pOutputFile, "ret i32 0\n}");
    //if there is an error on a delete flag is true and the output file is removed
    if (deleteFlag)
    {
        remove("./file.ll");
    }

    free(tokens); // frees the memory
    fclose(pOutputFile);
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

Variable *createVar(char *key, int data) // method to create variable
{
    Variable *var = malloc(sizeof(Variable)); // creates memory for the var
    var->data = data;
    var->key = strdup(key);
    return var;
}

void insert(char *key, int data) // inserting function for hashmap
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
        case '\r':
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
        case '/':
            token_list[found_tokens].type = DIVISION;
            token_list[found_tokens].id = "DIVISION";
            found_tokens++;
            pcurrent_char++;
            break;
        case '%':
            token_list[found_tokens].type = MODULE;
            token_list[found_tokens].id = "MODULE";
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
                int num = atoi(pcurrent_char); // converts the string to long long int
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
    token_list = (Token *)realloc(token_list, (*token_number) * sizeof(Token)); // at first this memory was equal to length of array * size of token
    return token_list;                                                          // returns the list of tokens
}

Node *constructNode(TokenType op, int *value, char *name, Node *left, Node *right) // makes the adjustments for a node
{
    Node *node = malloc(sizeof(Node)); // allocates the memory for a node
    node->op = op;
    node->value = malloc(sizeof(int)); // allocates the memory for node value
    *(node->value) = *value;
    node->name = strdup(name);
    node->left = left;
    node->right = right;
    return node;
}

Node *createNode(Token *token, Node *left, Node *right) // creates nodes for parsing tree
{                                                       // calls the constructNode method to reach data and allocate memory

    TokenType op = token->type;
    int *value = malloc(sizeof(int));
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
{
    //nextpos will check the next element at tokenList
    int nextpos = (*pos);
    nextpos++;
    

    //if there is an equal operator that mean we are currently at LHS so flag is true
    if (ptoken_list[nextpos].type == EQUAL)
    {
        isLeftHandSide = true;
    } 

    Node *temp = parseB(ptoken_list, pos);
    // error check
    if (temp == NULL) // if other parsing functions return NULL, there must be something wrong
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
        if (*pos == num_tokens) // if number of tokens is equal to the index of token list, that means the rest of equation is missing
        {                       // it is a different error check
            errorFlag = true;
            return NULL;
        }

        Node *temp2 = parseB(ptoken_list, pos);

        if (temp == NULL) // similar with the first error check
        {
            errorFlag = true;
            return NULL;
        }
        temp = createNode(op_token, temp, temp2);
    }
    if (*pos < num_tokens) // if at the end of assignment operation we didn't reach the last pos of token list, that means an error
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

    while (
        ptoken_list[*pos].type == MULTIPLICATION || ptoken_list[*pos].type == DIVISION || ptoken_list[*pos].type == MODULE)
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
    if (ptoken_list[*pos].type == VAR)
    {
        // if we are currently on LHS , we allocate the variable if it is not already defined
        //  and we insert it for evaluate operations
        // only LHS variables are inserted to hashMap of variables
        if (isLeftHandSide)
        {

            if (!search(ptoken_list[*pos].name))
            {
                allocate(ptoken_list[*pos].name);
            }

            insert(ptoken_list[*pos].name, ptoken_list[*pos].number);
            isLeftHandSide = false;
        }
        //this error flag is for undefined variables
        //since only LHS varaibles are inserted, if a variable is not inserted before
        // then it not undefined so this is a syntax error
        if (!search(ptoken_list[*pos].name))
        {
            errorFlag = true;
            return NULL;
        }
        Node *temp = createNode(&(ptoken_list[*pos]), NULL, NULL);
        (*pos)++;
        return temp;
    }
    else if (ptoken_list[*pos].type == CONST) // if the current token matches the type, creates node
    {

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


void allocate(char *name)
{
    //writes simple allocate LLVM command which takes variable's name as input
    fprintf(pOutputFile, "%%%s = alloca i32\n", name);
}
void store(Node *node, char *name)
{
    //writes store LLVM command 
    if (node->op == CONST)
    {
        //if node is constant then there is no % in 
        int value = *(node->value);
        fprintf(pOutputFile, "store i32 %d, i32* %%%s\n", value, name);
    }
    else
    {
        //if node is not constant then there is an extra % and dummyNo
        int dummyNo = node->dummyNo;
        fprintf(pOutputFile, "store i32 %%%d, i32* %%%s\n", dummyNo, name);
    }
}
//recursive load function that loads every variable in a tree starting from root node
void load(Node *node)
{
    if (node == NULL)
    {
        return;
    }
    load(node->left);
    load(node->right);
    if (node->op == VAR)
    {
        char *name = node->name;
        fprintf(pOutputFile, "%%%d = load i32, i32* %%%s\n", dummyCounter, name);
        //current node has a dummyNo now so we can call it while writing binary operations
        node->dummyNo = dummyCounter;
        //since a dummy var is created , dummyCounter  is updated
        (*pdummyCounter)++;
    }
}
//this writes the current binary process to output file as LLVM code
//left rotate and right rotate is not included
//not is not included since it is not binary
void printProcess(Node *node, char *name)
{
    int dummyLeft = dummyofChild(node->left);
    int dummyRight = dummyofChild(node->right);
    Node *pLeft = node->left;
    Node *pRight = node->right;
    //if statements are used for node types
    //if a node is constant then in prints there is no % before its value
    //representing constant value
    //if a node is not constant then its dummyNo is used 
    //and there is a % before (which represents a dummyVar)
    if (pLeft->op == CONST)
    {
        if (pRight->op == CONST)
        {
            fprintf(pOutputFile, "%%%d = %s i32 %d,%d\n", dummyCounter, name, dummyLeft, dummyRight);
        }
        else
        {
            fprintf(pOutputFile, "%%%d = %s i32 %d,%%%d\n", dummyCounter, name, dummyLeft, dummyRight);
        }
    }
    else
    {
        if (pRight->op == CONST)
        {
            fprintf(pOutputFile, "%%%d = %s i32 %%%d,%d\n", dummyCounter, name, dummyLeft, dummyRight);
        }
        else
        {
            fprintf(pOutputFile, "%%%d = %s i32 %%%d,%%%d\n", dummyCounter, name, dummyLeft, dummyRight);
        }
    }
    node->dummyNo = dummyCounter;
    (*pdummyCounter)++;
}
//write function for leftrotate and write rotate
// if statements for constant types are similar with the ones explained before
void printRotate(Node *node, TokenType token)
{
    int dummyLeft = dummyofChild(node->left);
    int dummyRight = dummyofChild(node->right);
    Node *pLeft = node->left;
    Node *pRight = node->right;
    if (token == R_ROTATE)
    {
        if (pLeft->op == CONST)
        {
            if (pRight->op == CONST)
            {
                fprintf(pOutputFile, "%%%d = ashr i32 %d,%d\n", dummyCounter, dummyLeft, dummyRight);
                int dumNo1 = dummyCounter;
                (*pdummyCounter)++;
                fprintf(pOutputFile, "%%%d = sub i32 32,%d\n", dummyCounter, dummyRight);
                int dumNo2 = dummyCounter;
                (*pdummyCounter)++;
                fprintf(pOutputFile, "%%%d = shl i32 %d,%%%d\n", dummyCounter, dummyLeft, dumNo2);
                int dumNo3 = dummyCounter;
                (*pdummyCounter)++;
                fprintf(pOutputFile, "%%%d = or i32 %%%d,%%%d\n", dummyCounter, dumNo1, dumNo3);
                node->dummyNo = dummyCounter;
                (*pdummyCounter)++;
            }
            else
            {
                fprintf(pOutputFile, "%%%d = ashr i32 %d,%%%d\n", dummyCounter, dummyLeft, dummyRight);
                int dumNo1 = dummyCounter;
                (*pdummyCounter)++;
                fprintf(pOutputFile, "%%%d = sub i32 32,%%%d\n", dummyCounter, dummyRight);
                int dumNo2 = dummyCounter;
                (*pdummyCounter)++;
                fprintf(pOutputFile, "%%%d = shl i32 %d,%%%d\n", dummyCounter, dummyLeft, dumNo2);
                int dumNo3 = dummyCounter;
                (*pdummyCounter)++;
                fprintf(pOutputFile, "%%%d = or i32 %%%d,%%%d\n", dummyCounter, dumNo1, dumNo3);
                node->dummyNo = dummyCounter;
                (*pdummyCounter)++;
            }
        }
        else
        {
            if (pRight->op == CONST)
            {
                fprintf(pOutputFile, "%%%d = ashr i32 %%%d,%d\n", dummyCounter, dummyLeft, dummyRight);
                int dumNo1 = dummyCounter;
                (*pdummyCounter)++;
                fprintf(pOutputFile, "%%%d = sub i32 32,%d\n", dummyCounter, dummyRight);
                int dumNo2 = dummyCounter;
                (*pdummyCounter)++;
                fprintf(pOutputFile, "%%%d = shl i32 %%%d,%%%d\n", dummyCounter, dummyLeft, dumNo2);
                int dumNo3 = dummyCounter;
                (*pdummyCounter)++;
                fprintf(pOutputFile, "%%%d = or i32 %%%d,%%%d\n", dummyCounter, dumNo1, dumNo3);
                node->dummyNo = dummyCounter;
                (*pdummyCounter)++;
            }
            else
            {
                fprintf(pOutputFile, "%%%d = ashr i32 %%%d,%%%d\n", dummyCounter, dummyLeft, dummyRight);
                int dumNo1 = dummyCounter;
                (*pdummyCounter)++;
                fprintf(pOutputFile, "%%%d = sub i32 32,%%%d\n", dummyCounter, dummyRight);
                int dumNo2 = dummyCounter;
                (*pdummyCounter)++;
                fprintf(pOutputFile, "%%%d = shl i32 %%%d,%%%d\n", dummyCounter, dummyLeft, dumNo2);
                int dumNo3 = dummyCounter;
                (*pdummyCounter)++;
                fprintf(pOutputFile, "%%%d = or i32 %%%d,%%%d\n", dummyCounter, dumNo1, dumNo3);
                node->dummyNo = dummyCounter;
                (*pdummyCounter)++;
            }
        }
    }
    else
    {
        if (pLeft->op == CONST)
        {
            if (pRight->op == CONST)
            {
                fprintf(pOutputFile, "%%%d = shl i32 %d,%d\n", dummyCounter, dummyLeft, dummyRight);
                int dumNo1 = dummyCounter;
                (*pdummyCounter)++;
                fprintf(pOutputFile, "%%%d = sub i32 32,%d\n", dummyCounter, dummyRight);
                int dumNo2 = dummyCounter;
                (*pdummyCounter)++;
                fprintf(pOutputFile, "%%%d = ashr i32 %d,%%%d\n", dummyCounter, dummyLeft, dumNo2);
                int dumNo3 = dummyCounter;
                (*pdummyCounter)++;
                fprintf(pOutputFile, "%%%d = or i32 %%%d,%%%d\n", dummyCounter, dumNo1, dumNo3);
                node->dummyNo = dummyCounter;
                (*pdummyCounter)++;
            }
            else
            {
                fprintf(pOutputFile, "%%%d = shl i32 %d,%%%d\n", dummyCounter, dummyLeft, dummyRight);
                int dumNo1 = dummyCounter;
                (*pdummyCounter)++;
                fprintf(pOutputFile, "%%%d = sub i32 32,%%%d\n", dummyCounter, dummyRight);
                int dumNo2 = dummyCounter;
                (*pdummyCounter)++;
                fprintf(pOutputFile, "%%%d = ashr i32 %d,%%%d\n", dummyCounter, dummyLeft, dumNo2);
                int dumNo3 = dummyCounter;
                (*pdummyCounter)++;
                fprintf(pOutputFile, "%%%d = or i32 %%%d,%%%d\n", dummyCounter, dumNo1, dumNo3);
                node->dummyNo = dummyCounter;
                (*pdummyCounter)++;
            }
        }
        else
        {
            if (pRight->op == CONST)
            {
                fprintf(pOutputFile, "%%%d = shl i32 %%%d,%d\n", dummyCounter, dummyLeft, dummyRight);
                int dumNo1 = dummyCounter;
                (*pdummyCounter)++;
                fprintf(pOutputFile, "%%%d = sub i32 32,%d\n", dummyCounter, dummyRight);
                int dumNo2 = dummyCounter;
                (*pdummyCounter)++;
                fprintf(pOutputFile, "%%%d = ashr i32 %%%d,%%%d\n", dummyCounter, dummyLeft, dumNo2);
                int dumNo3 = dummyCounter;
                (*pdummyCounter)++;
                fprintf(pOutputFile, "%%%d = or i32 %%%d,%%%d\n", dummyCounter, dumNo1, dumNo3);
                node->dummyNo = dummyCounter;
                (*pdummyCounter)++;
            }
            else
            {
                fprintf(pOutputFile, "%%%d = shl i32 %%%d,%%%d\n", dummyCounter, dummyLeft, dummyRight);
                int dumNo1 = dummyCounter;
                (*pdummyCounter)++;
                fprintf(pOutputFile, "%%%d = sub i32 32,%%%d\n", dummyCounter, dummyRight);
                int dumNo2 = dummyCounter;
                (*pdummyCounter)++;
                fprintf(pOutputFile, "%%%d = ashr i32 %%%d,%%%d\n", dummyCounter, dummyLeft, dumNo2);
                int dumNo3 = dummyCounter;
                (*pdummyCounter)++;
                fprintf(pOutputFile, "%%%d = or i32 %%%d,%%%d\n", dummyCounter, dumNo1, dumNo3);
                node->dummyNo = dummyCounter;
                (*pdummyCounter)++;
            }
        }
    }
}
//returns the dummyNo of a child of a node
//if it is constant then since there is no dummyVar, the value of constant is returned 
int dummyofChild(Node *node)
{
    int dummy;
    if (node->op == CONST)
    {
        dummy = *(node->value);
    }
    else
    {
        dummy = node->dummyNo;
    }
    return dummy;
}

// method to evaluate the tree
int evaluate(Node *nodeP)
{
    // starts to evaluate from root node recursively

    // binary operations


    //after every recursion write functions are added before returning
    //so the write statements start at the deepest state of recursion
    if (nodeP->op == ADDITION)
    {
        int leftR = evaluate(nodeP->left);
        int rightR = evaluate(nodeP->right);
        printProcess(nodeP, "add");
        return leftR + rightR;
    }
    else if (nodeP->op == SUBTRACTION)
    {
        int leftR = evaluate(nodeP->left);
        int rightR = evaluate(nodeP->right);
        printProcess(nodeP, "sub");
        return leftR - rightR;
    }
    else if (nodeP->op == MULTIPLICATION)
    {
        int leftR = evaluate(nodeP->left);
        int rightR = evaluate(nodeP->right);
        printProcess(nodeP, "mul");
        return leftR * rightR;
    }
    else if (nodeP->op == DIVISION)
    {
        int leftR = evaluate(nodeP->left);
        int rightR = evaluate(nodeP->right);
        printProcess(nodeP, "sdiv");
        return leftR / rightR;
    }
    else if (nodeP->op == MODULE)
    {
        int leftR = evaluate(nodeP->left);
        int rightR = evaluate(nodeP->right);
        printProcess(nodeP, "srem");
        return leftR % rightR;
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
        return 0;
    }

    else if (nodeP->op == EQUAL) // if there exist an equation, no printing
    {
        Node *pLeft;
        pLeft = nodeP->left;
        Variable *pVar = search(pLeft->name);

        pVar->data = evaluate(nodeP->right);

        //if node type is equal then store LLVM command is written to the output file
        store(nodeP->right, pVar->key);

        printFlag = false;
        return pVar->data;
    }

    // bitwise binary operations
    else if (nodeP->op == AND)
    {
        int leftR = evaluate(nodeP->left);
        int rightR = evaluate(nodeP->right);
        printProcess(nodeP, "and");
        return leftR & rightR;
    }
    else if (nodeP->op == OR)
    {
        int leftR = evaluate(nodeP->left);
        int rightR = evaluate(nodeP->right);
        printProcess(nodeP, "or");
        return leftR | rightR;
    }
    // function with one parameter
    else if (nodeP->op == NOT)
    {
        //not nodes have only left child since it is not binary
        //not llvm command is written with xor (dummyLeft, -1)
        int leftR = evaluate(nodeP->left);
        Node *pleft = nodeP->left;
        int dummyLeft;
        if (pleft->op == CONST)
        {
            dummyLeft = *(pleft->value);
            fprintf(pOutputFile, "%%%d = %s i32 %d,%d\n", dummyCounter, "xor", dummyLeft, -1);
            (*pdummyCounter)++;
        }
        else
        {
            dummyLeft = pleft->dummyNo;
            fprintf(pOutputFile, "%%%d = %s i32 %%%d,%d\n", dummyCounter, "xor", dummyLeft, -1);
            nodeP->dummyNo = dummyCounter;
            (*pdummyCounter)++;
        }

        return ~leftR;
    }

    // functions with two parameters
    else
    {
        Node *pLeft = nodeP->left;
        Node *pRight = nodeP->right;
        bool syntaxError = false;
        switch (nodeP->op)
        {
            int leftR;
            int rightR;
        case XOR:
            leftR = evaluate(nodeP->left);
            rightR = evaluate(nodeP->right);
            printProcess(nodeP, "xor");
            return leftR ^ rightR;
            break;
        case L_SHIFT:
            leftR = evaluate(nodeP->left);
            rightR = evaluate(nodeP->right);
            printProcess(nodeP, "shl");
            return leftR << rightR;
        case R_SHIFT:
            leftR = evaluate(nodeP->left);
            rightR = evaluate(nodeP->right);
            printProcess(nodeP, "ashr");
            return leftR >> rightR;
        case L_ROTATE:
            leftR = evaluate(nodeP->left);
            rightR = evaluate(nodeP->right);
            printRotate(nodeP, L_ROTATE);
            return (leftR << rightR) | (leftR >> (32 - rightR));
        case R_ROTATE:
            leftR = evaluate(nodeP->left);
            rightR = evaluate(nodeP->right);
            printRotate(nodeP, R_ROTATE);
            return (leftR >> rightR) | (leftR << (32 - rightR));
        default:
            break;
        }
    }
}