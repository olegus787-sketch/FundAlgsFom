#include <ctype.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define OK 0
#define BASE_ERR 1
#define NUMBER_ERR 2
#define OVERFLOW_ERR 3

#define MAX_LINE 4096

static int digit_value(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'A' && c <= 'Z') return c - 'A' + 10;
    if (c >= 'a' && c <= 'z') return c - 'a' + 10;
    return -1;
}

static int parse_long(const char *s, long *out) {
    if (!s || !*s || !out) return 0;

    int sign = 1;
    const char *p = s;
    if (*p == '+' || *p == '-') {
        if (*p == '-') sign = -1;
        p++;
    }
    if (!*p) return 0; 

    unsigned long val = 0;
    unsigned long limit = (sign == 1) ? (unsigned long)LONG_MAX : (unsigned long)LONG_MAX + 1UL;

    for (; *p; p++) {
        if (*p < '0' || *p > '9') return 0;
        unsigned long d = (unsigned long)(*p - '0');
        if (val > (limit - d) / 10UL) return 0;
        val = val * 10UL + d;
    }

    if (sign == 1) {
        *out = (long)val;
    } else if (val == (unsigned long)LONG_MAX + 1UL) {
        *out = LONG_MIN;
    } else {
        *out = -(long)val;
    }
    return 1;
}

static int parse_number(const char *s, int base, long long *out) {
    if (!s || !*s) return NUMBER_ERR;

    int sign = 1;
    if (*s == '-') { sign = -1; s++; }
    else if (*s == '+') { s++; }

    if (!*s) return NUMBER_ERR;

    long long val = 0;
    int any = 0;

    for (; *s; s++) {
        int d = digit_value(*s);
        if (d < 0 || d >= base) return NUMBER_ERR;
        any = 1;

        if (val > (LLONG_MAX - (long long)d) / (long long)base) {
            return OVERFLOW_ERR;
        }
        val = val * (long long)base + (long long)d;
    }

    if (!any) return NUMBER_ERR;
    *out = (sign == -1) ? -val : val;
    return OK;
}

static void print_in_base(long long val, int base, FILE *out) {
    if (val == 0) {
        fputc('0', out);
        return;
    }

    char buf[80];
    int n = 0;
    int neg = 0;
    unsigned long long uv;

    if (val < 0) {
        neg = 1;
        uv = (unsigned long long)(-(val + 1)) + 1ULL;
    } else {
        uv = (unsigned long long)val;
    }

    while (uv > 0) {
        int d = (int)(uv % (unsigned long long)base);
        buf[n++] = (d < 10) ? (char)('0' + d) : (char)('A' + d - 10);
        uv /= (unsigned long long)base;
    }

    if (neg) fputc('-', out);
    while (n > 0) fputc(buf[--n], out);
}

int main(void) {
    char line[MAX_LINE];

    printf("Enter base (2..36): ");
    fflush(stdout);
    if (!fgets(line, sizeof(line), stdin)) {
        fprintf(stderr, "Error: cannot read base\n");
        return BASE_ERR;
    }

    long bval;
    if (!parse_long(line, &bval)) {
        fprintf(stderr, "Error: invalid base\n");
        return BASE_ERR;
    }
    if (bval < 2 || bval > 36) {
        fprintf(stderr, "Error: base must be in [2..36]\n");
        return BASE_ERR;
    }
    int base = (int)bval;

    printf("Enter numbers in base %d, \"Stop\" to finish:\n", base);
    fflush(stdout);

    long long sum = 0;
    long long max_val = 0;
    long long max_abs = 0;
    int has_max = 0;
    int numbers_count = 0;

    while (fgets(line, sizeof(line), stdin)) {
        size_t len = strlen(line);
        while (len > 0 && (line[len - 1] == '\n' || line[len - 1] == '\r')) {
            line[--len] = '\0';
        }
        if (len == 0) continue;

        if (strcmp(line, "Stop") == 0) break;

        long long val;
        int s = parse_number(line, base, &val);
        if (s == OVERFLOW_ERR) {
            fprintf(stderr, "Warning: '%s' overflows long long, skipped\n", line);
            continue;
        }
        if (s != OK) {
            fprintf(stderr, "Warning: '%s' is not a valid number in base %d\n", line, base);
            continue;
        }

        if ((val > 0 && sum > LLONG_MAX - val) ||
            (val < 0 && sum < LLONG_MIN - val)) {
            fprintf(stderr, "Warning: sum overflow at '%s', skipped\n", line);
            continue;
        }
        sum += val;
        numbers_count++;

        long long abs_v = (val < 0) ? -val : val;
        if (!has_max || abs_v > max_abs) {
            max_abs = abs_v;
            max_val = val;
            has_max = 1;
        }
    }

    printf("\n");

    if (numbers_count == 0) {
        printf("No numbers entered\n");
        return OK;
    }

    int bases[] = {9, 18, 27, 36};

    printf(" Maximum by absolute value \n");
    printf("base 10: ");
    print_in_base(max_val, 10, stdout);
    printf("\n");
    for (int i = 0; i < 4; ++i) {
        printf("base %2d: ", bases[i]);
        print_in_base(max_val, bases[i], stdout);
        printf("\n");
    }

    printf("\n Sum \n");
    printf("base 10: ");
    print_in_base(sum, 10, stdout);
    printf("\n");
    for (int i = 0; i < 4; ++i) {
        printf("base %2d: ", bases[i]);
        print_in_base(sum, bases[i], stdout);
        printf("\n");
    }

    return OK;
}