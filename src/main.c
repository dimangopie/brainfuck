/*
 * Copyright 2016 Fabian Mastenbroek
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <ctype.h>

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
    #include <io.h>
    #define isatty _isatty
    #define STDIN_FILENO 0
#else
    #include <unistd.h>
#endif

#include <brainfuck.h>

#define HISTORY_SIZE 100
char *history[HISTORY_SIZE];
int history_index = 0;

/**
 * Print the usage message of this program.
 *
 * @param name The name of this program (given by argv[0]).
 */
void print_usage(char *name) {
    fprintf(stderr, "usage: %s [-evh] [file...]\n", name);
    fprintf(stderr, "\t-e --eval\t\trun code directly\n");
    fprintf(stderr, "\t-v --version\t\tshow version information\n");
    fprintf(stderr, "\t-h --help\t\tshow a help message\n");
}

/**
 * Print the version information.
 */
void print_version() {
    fprintf(stderr, "brainfuck %d.%d.%d (%s, %s)\n", BRAINFUCK_VERSION_MAJOR,
        BRAINFUCK_VERSION_MINOR, BRAINFUCK_VERSION_PATCH, __DATE__, __TIME__);
    fprintf(stderr, "Copyright (c) 2016 Fabian Mastenbroek.\n");
    fprintf(stderr, "Distributed under the Apache License Version 2.0.\n");
}

/**
 * Initialize readline functionality.
 */
void initialize_readline() {
    // 初始化历史记录索引
    history_index = 0;
}

/**
 * Read a line from the user input.
 *
 * @param prompt The prompt to display to the user.
 * @return A dynamically allocated string containing the user input.
 */
char *readline(const char *prompt) {
    static char line[1024];
    if (prompt) {
        printf("%s", prompt);
        fflush(stdout);
    }

    if (fgets(line, sizeof(line), stdin) == NULL) {
        return NULL;
    }

    // 移除换行符
    line[strcspn(line, "\n")] = 0;

    return strdup(line); // 返回一个动态分配的副本
}

/**
 * Add a line to the history.
 *
 * @param line The line to add to the history.
 */
void add_history(const char *line) {
    if (line == NULL) {
        return;
    }

    // 移除重复的历史记录
    if (history_index > 0 && strcmp(history[history_index - 1], line) == 0) {
        return;
    }

    if (history_index < HISTORY_SIZE) {
        history[history_index++] = strdup(line);
    } else {
        free(history[0]);
        for (int i = 0; i < HISTORY_SIZE - 1; i++) {
            history[i] = history[i + 1];
        }
        history[HISTORY_SIZE - 1] = strdup(line);
    }
}

/**
 * Run the given brainfuck file.
 *
 * @param file The brainfuck file to run.
 * @return EXIT_SUCCESS if no errors are encountered, otherwise EXIT_FAILURE.
 */
int run_file(FILE *file) {
    BrainfuckState *state = brainfuck_state();
    BrainfuckExecutionContext *context = brainfuck_context(BRAINFUCK_TAPE_SIZE);
    if (file == NULL) {
        brainfuck_destroy_context(context);
        brainfuck_destroy_state(state);
        return EXIT_FAILURE;
    }
    brainfuck_add(state, brainfuck_parse_stream(file));
    brainfuck_execute(state->root, context);
    brainfuck_destroy_context(context);
    brainfuck_destroy_state(state);
    fclose(file);
    return EXIT_SUCCESS;
}

/**
 * Run the given brainfuck string.
 *
 * @param code The brainfuck string to run.
 * @return EXIT_SUCCESS if no errors are encountered, otherwise EXIT_FAILURE.
 */
int run_string(char *code) {
    BrainfuckState *state = brainfuck_state();
    BrainfuckExecutionContext *context = brainfuck_context(BRAINFUCK_TAPE_SIZE);
    BrainfuckInstruction *instruction = brainfuck_parse_string(code);
    brainfuck_add(state, instruction);
    brainfuck_execute(state->root, context);
    brainfuck_destroy_context(context);
    brainfuck_destroy_state(state);
    return EXIT_SUCCESS;
}

/**
 * Run the brainfuck interpreter in interactive mode.
 */
void run_interactive_console() {
    printf("brainfuck %d.%d.%d (%s, %s)\n", BRAINFUCK_VERSION_MAJOR,
        BRAINFUCK_VERSION_MINOR, BRAINFUCK_VERSION_PATCH, __DATE__, __TIME__);
#ifdef BRAINFUCK_EXTENSION_DEBUG
    printf("Use # to inspect tape\n");
#endif
    BrainfuckState *state = brainfuck_state();
    BrainfuckExecutionContext *context = brainfuck_context(BRAINFUCK_TAPE_SIZE);
    BrainfuckInstruction *instruction;
    char *line;

    initialize_readline();
    while(1) {
        line = readline(">> ");
        if (line) {
            /* Empty string crashes on some OSs/versions of editline */
            if (line[0] == '\0') {
                free(line);
                continue;
            }

            add_history(line);
            instruction = brainfuck_parse_string(line);
            brainfuck_add(state, instruction);
            brainfuck_execute(state->root, context);
            free(line);
        } else {
            /* EOF */
            break;
        }
    }
}

/* Command line options */
static struct option long_options[] = {
    {"help", no_argument, 0, 'h'},
    {"eval", required_argument, 0, 'e'},
    {"version", no_argument, 0, 'v'},
    {0, 0, 0, 0}
};

/**
 * Main entry point of the program.
 *
 * @param argc The amount of arguments given.
 * @param argv The array with arguments.
 */
int main(int argc, char *argv[]) {
    int c;
    int i = 1;
    int option_index = 0;

    while (1) {
        option_index = 0;
        c = getopt_long (argc, argv, "vhe:",
                long_options, &option_index);
        if (c == -1)
            break;

        switch (c) {
        case 0:
            if (long_options[option_index].flag != 0)
                break;
            break;
        case 'h':
            print_usage(argv[0]);
            return EXIT_SUCCESS;
        case 'v':
            print_version();
            return EXIT_SUCCESS;
        case 'e':
            return run_string((char *) optarg);
        case '?':
            print_usage(argv[0]);
            return EXIT_FAILURE;
        default:
            abort();
        }
    }
    if (argc > 1) {
        while (i < argc)
            if (run_file(fopen(argv[i++], "r")) == EXIT_FAILURE)
                fprintf(stderr, "error: failed to read file %s\n", argv[i - 1]);
    } else {
        /* Check if someone is piping code or just calling it the normal way */
        if (isatty(STDIN_FILENO)) {
            run_interactive_console();
        } else {
            if (run_file(stdin) == EXIT_FAILURE)
                fprintf(stderr, "error: failed to read from stdin\n");
        }
    }
    return EXIT_SUCCESS;
}