#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define OK 0
#define USAGE_ERR 1
#define FLAG_INPUT_ERR 2
#define FILE_OPEN_ERR 3
#define FILE_READ_ERR 4
#define FILE_WRITE_ERR 5
#define MEMORY_ERR 6

#define LINE_BUF 4096

static int make_out_name(const char *input_path, char **out) {
    if (!input_path || !out) return MEMORY_ERR;

    size_t len = strlen(input_path);
    char *result = malloc(4 + len + 1);
    if (!result) return MEMORY_ERR;

    memcpy(result, "out_", 4);
    memcpy(result + 4, input_path, len + 1);

    *out = result;
    return OK;
}

// -d
static int process_d(FILE *in, FILE *out) {
    int c;
    while ((c = fgetc(in)) != EOF) {
        if (!isdigit((unsigned char)c)) {
            if (fputc(c, out) == EOF) return FILE_WRITE_ERR;
        }
    }
    if (ferror(in)) return FILE_READ_ERR;
    return OK;
}

// -i
static int count_letters(const char *line, size_t *out) {
    if (!line || !out) return FLAG_INPUT_ERR;

    size_t count = 0;
    for (const char *p = line; *p; ++p) {
        if (isalpha((unsigned char)*p)) {
            ++count;
        }
    }
    *out = count;
    return OK;
}

static int process_i(FILE *in, FILE *out) {
    char buf[LINE_BUF];
    while (fgets(buf, sizeof(buf), in)) {
        size_t count = 0;
        int status = count_letters(buf, &count);
        if (status != OK) return status;
        if (fprintf(out, "%zu\n", count) < 0) return FILE_WRITE_ERR;
    }
    if (ferror(in)) return FILE_READ_ERR;
    return OK;
}

// -s
static int count_special(const char *line, size_t *out) {
    if (!line || !out) return FLAG_INPUT_ERR;

    size_t count = 0;
    for (const char *p = line; *p; ++p) {
        unsigned char c = (unsigned char)*p;
        if (c == '\n' || c == '\r') continue;
        if (!isalpha(c) && !isdigit(c) && c != ' ') {
            ++count;
        }
    }
    *out = count;
    return OK;
}

static int process_s(FILE *in, FILE *out) {
    char buf[LINE_BUF];
    while (fgets(buf, sizeof(buf), in)) {
        size_t count = 0;
        int status = count_special(buf, &count);
        if (status != OK) return status;
        if (fprintf(out, "%zu\n", count) < 0) return FILE_WRITE_ERR;
    }
    if (ferror(in)) return FILE_READ_ERR;
    return OK;
}

// -a
static int process_a(FILE *in, FILE *out) {
    int c;
    while ((c = fgetc(in)) != EOF) {
        unsigned char uc = (unsigned char)c;
        if (isdigit(uc)) {
            if (fputc(c, out) == EOF) return FILE_WRITE_ERR;
        } else {
            if (fprintf(out, "%02X", uc) < 0) return FILE_WRITE_ERR;
        }
    }
    if (ferror(in)) return FILE_READ_ERR;
    return OK;
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <flag> <input> [<output>]\n", argv[0]);
        return USAGE_ERR;
    }

    char *flag = argv[1];

    if (flag[0] != '-' && flag[0] != '/') {
        fprintf(stderr, "Warning: flag should start with '-' or '/'\n");
        return FLAG_INPUT_ERR;
    }
    if (flag[0] == '/') flag[0] = '-';

    size_t flag_len = strlen(flag);
    int has_n = 0;
    char action;

    if (flag_len == 2) {
        action = flag[1];
    } else if (flag_len == 3 && flag[1] == 'n') {
        has_n = 1;
        action = flag[2];
    } else {
        fprintf(stderr, "Warning: unknown flag format\n");
        return FLAG_INPUT_ERR;
    }

    const char *in_path = argv[2];
    char *generated = NULL;
    const char *out_path = NULL;

    if (has_n) {
        if (argc != 4) {
            fprintf(stderr, "Usage: %s -n%c <input> <output>\n",
                    argv[0], action);
            return USAGE_ERR;
        }
        out_path = argv[3];
    } else {
        if (argc != 3) {
            fprintf(stderr, "Usage: %s -%c <input>\n", argv[0], action);
            return USAGE_ERR;
        }
        char *generated = NULL;
        int status = make_out_name(in_path, &generated);
        if (status != OK) return status;
        out_path = generated;
    }

    FILE *in = fopen(in_path, "r");
    if (!in) {
        fprintf(stderr, "Error: cannot open input file '%s'\n", in_path);
        free(generated);
        return FILE_OPEN_ERR;
    }

    FILE *out = fopen(out_path, "w");
    if (!out) {
        fprintf(stderr, "Error: cannot open output file '%s'\n", out_path);
        fclose(in);
        free(generated);
        return FILE_OPEN_ERR;
    }

    int status;
    switch (action) {
        case 'd': status = process_d(in, out); break;
        case 'i': status = process_i(in, out); break;
        case 's': status = process_s(in, out); break;
        case 'a': status = process_a(in, out); break;
        default:
            fprintf(stderr, "Warning: unknown action '-%c'\n", action);
            status = FLAG_INPUT_ERR;
            break;
    }

    if (fclose(in) != 0 && status == OK) {
        status = FILE_READ_ERR;
    }
    if (fclose(out) != 0 && status == OK) {
        status = FILE_WRITE_ERR;
    }
    free(generated);

    return status;
}