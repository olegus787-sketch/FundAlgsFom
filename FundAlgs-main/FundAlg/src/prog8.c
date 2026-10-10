#include <ctype.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define OK 0
#define USAGE_ERR 1
#define FILE_OPEN_ERR 2
#define FILE_READ_ERR 3
#define FILE_WRITE_ERR 4
#define MEMORY_ERR 5

static int digit_value(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'z') return c - 'a' + 10;
    if (c >= 'A' && c <= 'Z') return c - 'A' + 10;
    return -1;
}

// чтение числа
static int read_lexeme(FILE *f, char **buf, size_t *cap, size_t *len) {
    int c;
    while ((c = fgetc(f)) != EOF && isspace((unsigned char)c)) {}

    if (c == EOF) {
        if (ferror(f)) return -2;
        return 0;
    }

    *len = 0;
    while (c != EOF && !isspace((unsigned char)c)) {
        if (*len + 1 >= *cap) {
            size_t new_cap = (*cap == 0) ? 32 : (*cap) * 2;
            char *new_buf = realloc(*buf, new_cap);
            if (!new_buf) return -1;
            *buf = new_buf;
            *cap = new_cap;
        }
        (*buf)[(*len)++] = (char)c;
        c = fgetc(f);
    }
    (*buf)[*len] = '\0';
    if (c == EOF && ferror(f)) return -2;
    return 1;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <input> <output>\n", argv[0]);
        return USAGE_ERR;
    }

    const char *in_path  = argv[1];
    const char *out_path = argv[2];

    FILE *in = fopen(in_path, "r");
    if (!in) {
        fprintf(stderr, "Error: cannot open '%s'\n", in_path);
        return FILE_OPEN_ERR;
    }

    FILE *out = fopen(out_path, "w");
    if (!out) {
        fprintf(stderr, "Error: cannot open '%s'\n", out_path);
        fclose(in);
        return FILE_OPEN_ERR;
    }

    char  *buf = NULL;
    size_t cap = 0, len = 0;
    int status = OK;

    for (;;) {
        int r = read_lexeme(in, &buf, &cap, &len);
        if (r == -1) { status = MEMORY_ERR; break; }
        if (r == -2) { status = FILE_READ_ERR; break; }
        if (r ==  0) break;

        int max_d = 0;
        int valid = 1;
        for (size_t i = 0; i < len; ++i) {
            int d = digit_value(buf[i]);
            if (d < 0) { valid = 0; break; }
            if (d > max_d) max_d = d;
        }
        if (!valid) {
            continue;
        }

        int base = max_d + 1;
        if (base < 2) base = 2;

        size_t start = 0;
        while (start + 1 < len && buf[start] == '0') {
            ++start;
        }

        unsigned long long val = 0;
        int overflow = 0;
        for (size_t i = start; i < len; ++i) {
            int d = digit_value(buf[i]);
            unsigned long long ud = (unsigned long long)d;
            unsigned long long ub = (unsigned long long)base;
            if (val > (ULLONG_MAX - ud) / ub) {
                overflow = 1;
                break;
            }
            val = val * ub + ud;
        }

        if (fprintf(out, "%.*s\t%d\t", (int)(len - start), buf + start, base) < 0) {
            status = FILE_WRITE_ERR;
            break;
        }
        if (overflow) {
            if (fputs("overflow\n", out) == EOF) {
                status = FILE_WRITE_ERR;
                break;
            }
        } else {
            if (fprintf(out, "%llu\n", val) < 0) {
                status = FILE_WRITE_ERR;
                break;
            }
        }
    }

    free(buf);
    if (fclose(in) != 0 && status == OK) status = FILE_READ_ERR;
    if (fclose(out) != 0 && status == OK) status = FILE_WRITE_ERR;

    return status;
}