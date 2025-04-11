#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <assert.h>

#define MAX_STACK_SIZE 1024
#define MAX_PROGRAM_SIZE 2048

// Operation constants
#define OP_PUSH    1
#define OP_PLUS    2
#define OP_MINUS   3
#define OP_MUL     4
#define OP_DIV     5
#define OP_EQUAL   6
#define OP_DUP     7
#define OP_SWAP    8
#define OP_IF      9
#define OP_END     10
#define OP_DUMP    11
#define OP_ELSE 12


// Stack for operations
int stack[MAX_STACK_SIZE];
int sp = 0;  // Stack pointer

// Stack to track conditional blocks (iff and end)
int conditional_stack[MAX_STACK_SIZE];
int conditional_sp = 0;  // Stack pointer for conditional block tracking

// Lexing state
char token[32];

// Conditional block state tracking
int skip_block = 0;  // Flag to skip the block if condition is false

// Operation counter
int COUNT_OPS = 0;  // Counter for operations

// Program token list (this simulates the list of operations in the program)
int program[MAX_PROGRAM_SIZE];
int program_size = 0;

// Push value onto the stack
void push(int value) {
    if (sp < MAX_STACK_SIZE) {
        stack[sp++] = value;
        COUNT_OPS++;  // Increment the operation counter
    } else {
        printf("Stack overflow!\n");
    }
}

// Pop value from the stack
int pop() {
    if (sp > 0) {
        COUNT_OPS++;  // Increment the operation counter
        return stack[--sp];
    } else {
        printf("Stack underflow!\n");
        return -1;
    }
}

// Print the top value of the stack
void dump() {
    if (sp > 0) {
        printf("%d\n", stack[sp - 1]);
        COUNT_OPS++;  // Increment the operation counter
    } else {
        printf("Stack is empty!\n");
    }
}

// Duplicate the top value of the stack
void dup() {
    if (sp > 0) {
        push(stack[sp - 1]);
        COUNT_OPS++;  // Increment the operation counter
    } else {
        printf("Stack is empty!\n");
    }
}

// Swap the top two values on the stack
void swap() {
    if (sp > 1) {
        int temp = stack[sp - 1];
        stack[sp - 1] = stack[sp - 2];
        stack[sp - 2] = temp;
        COUNT_OPS++;  // Increment the operation counter
    } else {
        printf("Not enough elements to swap!\n");
    }
}

// Add top two values
void plus() {
    if (sp > 1) {
        int b = pop();
        int a = pop();
        push(a + b);
    } else {
        printf("Not enough elements to add!\n");
    }
    COUNT_OPS++;  // Increment the operation counter
}

// Subtract top two values
void minus() {
    if (sp > 1) {
        int b = pop();
        int a = pop();
        push(a - b);
    } else {
        printf("Not enough elements to subtract!\n");
    }
    COUNT_OPS++;  // Increment the operation counter
}

// Multiply top two values
void mul() {
    if (sp > 1) {
        int b = pop();
        int a = pop();
        push(a * b);
    } else {
        printf("Not enough elements to multiply!\n");
    }
    COUNT_OPS++;  // Increment the operation counter
}

// Divide top two values
void div() {
    if (sp > 1) {
        int b = pop();
        if (b == 0) {
            printf("Error: Division by zero!\n");
            push(0);  // Optional: Push 0 on error or handle the error appropriately.
        } else {
            int a = pop();
            push(a / b);
        }
    } else {
        printf("Not enough elements to divide!\n");
    }
    COUNT_OPS++;  // Increment the operation counter
}

// Equal check for top two values
void equal() {
    if (sp > 1) {
        int b = pop();
        int a = pop();
        if (a == b) {
            push(1);  // Push 1 if equal
        } else {
            push(0);  // Push 0 if not equal
        }
    } else {
        printf("Not enough elements to compare!\n");
    }
    COUNT_OPS++;  // Increment the operation counter
}

// Conditional (iff) - If top of stack is 0, skip next operations
void iff() {
    if (sp > 0) {
        int condition = pop();
        if (condition == 0) {
            skip_block = 1;  // Skip the next block of operations
        } else {
            skip_block = 0;
        }
    } else {
        printf("Not enough elements to evaluate condition!\n");
    }
    COUNT_OPS++;  // Increment the operation counter
}

// End conditional (end) - Ends a conditional block
void end() {
    skip_block = 0;  // Reset skip block flag
    COUNT_OPS++;  // Increment the operation counter
}

void elze(size_t* ip, int* program, size_t program_size) {
    size_t i = *ip;
    int depth = 1;
    for (; i < program_size; ++i) {
        if (program[i] == OP_IF) depth++;
        else if (program[i] == OP_END) {
            depth--;
            if (depth == 0) {
                *ip = i; // Skip to END after ELSE
                return;
            }
        }
    }
    fprintf(stderr, "Unmatched else\n");
    exit(1);
}


// Cross-reference conditional blocks (Check matching iff and end)
void crossreference_blocks() {
    if (conditional_sp > 0) {
        printf("Error: Unmatched 'iff' without 'end'.\n");
        exit(1);
    }
    printf("All conditional blocks are matched.\n");
    COUNT_OPS++;  // Increment the operation counter
}

// Parse the token as an operation
void parse_token_as_op(char *token) {
    static_assert(COUNT_OPS == 13, "Exhaustive handling of op in parse_token_as_op()");
    if (skip_block) {
        return;  // Skip the operation if we are in a conditional block
    }

    // Match the token to an operation
    if (strcmp(token, "+") == 0) {
        plus();
    } else if (strcmp(token, "-") == 0) {
        minus();
    } else if (strcmp(token, "*") == 0) {
        mul();
    } else if (strcmp(token, "/") == 0) {
        div();
    } else if (strcmp(token, "=") == 0) {
        equal();
    } else if (strcmp(token, ".") == 0) {
        dump();
    } else if (strcmp(token, "dup") == 0) {
        dup();
    } else if (strcmp(token, "swap") == 0) {
        swap();
    } else if (strcmp(token, "if") == 0) {
        iff();
    } else if (strcmp(token, "end") == 0) {
        end();
    } else if (strcmp(token, "else") == 0) {
        elze(&ip, program, program_size);
    }
}

// Lex a line of the program
void lex_line(char *line) {
    int i = 0;
    while (line[i] != '\0') {
        // Skip spaces and tabs
        while (line[i] == ' ' || line[i] == '\t') {
            i++;
        }
        
        // Read the next token
        int token_start = i;
        while (line[i] != '\0' && line[i] != ' ' && line[i] != '\t') {
            i++;
        }
        
        int token_len = i - token_start;
        if (token_len > 0) {
            strncpy(token, &line[token_start], token_len);
            token[token_len] = '\0';
            
            // Handle the token (you can simulate or compile here)
            parse_token_as_op(token);
        }
    }
}

// Lex the entire program file
void lex_file(const char *file_path) {
    FILE *file = fopen(file_path, "r");
    if (!file) {
        printf("Error: Could not open file %s\n", file_path);
        return;
    }
    
    char line[256];
    int line_number = 1;
    
    while (fgets(line, sizeof(line), file)) {
        printf("Line %d: %s", line_number, line);
        lex_line(line);
        line_number++;
    }
    
    fclose(file);
    
    // After lexing, cross-reference conditional blocks to ensure matching iff/end pairs
    crossreference_blocks();
}

// Simulate the program (without actual compilation)
void simulate_program(const char *file_path) {
    printf("Simulating program from file: %s\n", file_path);
    lex_file(file_path);
    
    // Iterate over the program operations
    for (int i = 0; i < program_size; i++) {
        int op = program[i];
        switch (op) {
            case OP_PUSH:
                push(42);  // Example: Push value 42 for demonstration
                break;
            case OP_PLUS:
                plus();
                break;
            case OP_MINUS:
                minus();
                break;
            case OP_MUL:
                mul();
                break;
            case OP_DIV:
                div();
                break;
            case OP_EQUAL:
                equal();
                break;
            case OP_DUP:
                dup();
                break;
            case OP_SWAP:
                swap();
                break;
            case OP_IF:
                iff();
                break;
            case OP_END:
                end();
                break;
            case OP_DUMP:
                dump();
                break;
            case OP_ELSE:
                elze(&ip, program, program_size);
                break;
            default:
                printf("Unknown operation: %d\n", op);
        }
    }
    
    printf("Program simulation complete.\n");
}

// Compile the program into a C file (here we're just printing the operations)
void compile_program(const char *file_path) {
    FILE *file = fopen(file_path, "r");
    if (!file) {
        printf("Error: Could not open file %s\n", file_path);
        return;
    }
    
    printf("Compiling program from file: %s\n", file_path);
    // In a real scenario, we would generate C code or machine code here
    fclose(file);
}

int main() {
    simulate_program("examples/foo.corth");  // Example file path for testing
    return 0;
}










