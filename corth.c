#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void parse_word_as_op(const char *word, int *stack, int *sp) {
    if (strcmp(word, "+") == 0) {
        if (*sp < 2) { fprintf(stderr, "Stack underflow\n"); exit(1); }
        stack[*sp - 2] = stack[*sp - 2] + stack[*sp - 1];
        (*sp)--;
    } else if (strcmp(word, "-") == 0) {
        if (*sp < 2) { fprintf(stderr, "Stack underflow\n"); exit(1); }
        stack[*sp - 2] = stack[*sp - 2] - stack[*sp - 1];
        (*sp)--;
    } else if (strcmp(word, "*") == 0) {
        if (*sp < 2) { fprintf(stderr, "Stack underflow\n"); exit(1); }
        stack[*sp - 2] = stack[*sp - 2] * stack[*sp - 1];
        (*sp)--;
    } else if (strcmp(word, "/") == 0) {
        if (*sp < 2) { fprintf(stderr, "Stack underflow\n"); exit(1); }
        if (stack[*sp - 1] == 0) { fprintf(stderr, "Division by zero\n"); exit(1); }
        stack[*sp - 2] = stack[*sp - 2] / stack[*sp - 1];
        (*sp)--;
    } else if (strcmp(word, ".") == 0) {
        if (*sp < 1) { fprintf(stderr, "Stack underflow\n"); exit(1); }
        printf("%d\n", stack[--(*sp)]);
    } else if (strcmp(word, "dup") == 0) {
        if (*sp < 1) { fprintf(stderr, "Stack underflow\n"); exit(1); }
        stack[*sp] = stack[*sp - 1];
        (*sp)++;
    } else if (strcmp(word, "swap") == 0) {
        if (*sp < 2) { fprintf(stderr, "Stack underflow\n"); exit(1); }
        int temp = stack[*sp - 1];
        stack[*sp - 1] = stack[*sp - 2];
        stack[*sp - 2] = temp;
    } else {
        // Treat as a number and push to stack
        stack[(*sp)++] = atoi(word);
    }
}

void load_program_from_file(const char *filename, int *stack, int *sp) {
    FILE *f = fopen(filename, "r");
    if (!f) {
        perror("Failed to open file");
        return;
    }

    char word[32];
    while (fscanf(f, "%31s", word) == 1) {
        parse_word_as_op(word, stack, sp);
    }

    fclose(f);
}

void simulate_program(const char *filename) {
    int stack[1024];
    int sp = 0;

    load_program_from_file(filename, stack, &sp);
}

void compile_program(const char *filename) {
    FILE *f = fopen(filename, "r");
    if (!f) {
        perror("Failed to open file");
        return;
    }

    char c_filename[300], o_filename[300], exe_filename[300];
    snprintf(c_filename, sizeof(c_filename), "%s.c", filename);
    snprintf(o_filename, sizeof(o_filename), "%s.o", filename);
    snprintf(exe_filename, sizeof(exe_filename), "%s.out", filename);

    FILE *out = fopen(c_filename, "w");
    if (!out) {
        perror("Failed to write C file");
        fclose(f);
        return;
    }

    fprintf(out,
        "#include <stdio.h>\n"
        "int main() {\n"
        "    int stack[1024];\n"
        "    int sp = 0;\n"
    );

    char token[32];
    while (fscanf(f, "%31s", token) == 1) {
        if (strcmp(token, "+") == 0) {
            fprintf(out, "    stack[sp - 2] = stack[sp - 2] + stack[sp - 1]; sp--;\n");
        } else if (strcmp(token, "-") == 0) {
            fprintf(out, "    stack[sp - 2] = stack[sp - 2] - stack[sp - 1]; sp--;\n");
        } else if (strcmp(token, "*") == 0) {
            fprintf(out, "    stack[sp - 2] = stack[sp - 2] * stack[sp - 1]; sp--;\n");
        } else if (strcmp(token, "/") == 0) {
            fprintf(out, "    if (stack[sp - 1] == 0) { printf(\"Division by zero\\n\"); return 1; }\n");
            fprintf(out, "    stack[sp - 2] = stack[sp - 2] / stack[sp - 1]; sp--;\n");
        } else if (strcmp(token, ".") == 0) {
            fprintf(out, "    printf(\"%%d\\n\", stack[--sp]);\n");
        } else if (strcmp(token, "dup") == 0) {
            fprintf(out, "    stack[sp] = stack[sp - 1]; sp++;\n");
        } else if (strcmp(token, "swap") == 0) {
            fprintf(out, "    int temp = stack[sp - 1];\n");
            fprintf(out, "    stack[sp - 1] = stack[sp - 2];\n");
            fprintf(out, "    stack[sp - 2] = temp;\n");
        } else {
            fprintf(out, "    stack[sp++] = %d;\n", atoi(token));
        }
    }

    fprintf(out, "    return 0;\n}\n");

    fclose(f);
    fclose(out);

    char cmd[512];

    if (snprintf(cmd, sizeof(cmd), "cc -c %s -o %s", c_filename, o_filename) >= (int)sizeof(cmd)) {
        fprintf(stderr, "Command too long\n");
        return;
    }
    if (system(cmd) != 0) {
        fprintf(stderr, "Compilation failed (object)\n");
        return;
    }

    if (snprintf(cmd, sizeof(cmd), "cc %s -o %s", o_filename, exe_filename) >= (int)sizeof(cmd)) {
        fprintf(stderr, "Command too long\n");
        return;
    }
    if (system(cmd) != 0) {
        fprintf(stderr, "Compilation failed (executable)\n");
        return;
    }

    printf("Compiled to %s\n", exe_filename);

    // Clean up intermediate files
    if (remove(c_filename) != 0) {
        fprintf(stderr, "Error removing C file: %s\n", c_filename);
    }
    if (remove(o_filename) != 0) {
        fprintf(stderr, "Error removing object file: %s\n", o_filename);
    }
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s [sim|com] file.corth\n", argv[0]);
        return 1;
    }

    const char *mode = argv[1];
    const char *file = argv[2];

    if (strcmp(mode, "sim") == 0) {
        simulate_program(file);
    } else if (strcmp(mode, "com") == 0) {
        compile_program(file);
    } else {
        fprintf(stderr, "Unknown mode: %s\n", mode);
        return 1;
    }

    return 0;
}





