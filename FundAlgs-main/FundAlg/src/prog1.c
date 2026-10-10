#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define OK 0
#define USAGE_ERR 1
#define FLAG_INPUT_ERR 2
#define NUMBER_INPUT_ERR 3
#define RANGE_ERR 4
#define OVERFLOW_ERR 5

#define MAX_MULTIPLES 100 // -h
#define MAX_HEX_DIGITS 64 // -s
#define MAX_EXP 10 // -e
#define MAX_BASE 10 // -e

// парсер для строкового представления числа
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

// flag -h
int calc_multiples(long x, long *arr, size_t arr_size, size_t *out_count) {
    if (!arr || !out_count) {
        return RANGE_ERR;
    }
    if (x <= 0) {
        return RANGE_ERR;
    }

    size_t count = 0;
    for (long i = 1; i <= 100 && count < arr_size; ++i) {
        if (i % x == 0) {
            arr[count++] = i;
        }
    }
    *out_count = count;
    return OK;
}

void print_multiples(const long *arr, size_t count) {
    if (count == 0) {
        printf("No such numbers\n");
        return;
    }
    for (size_t i = 0; i < count; ++i) {
        printf("%ld\n", arr[i]);
    }
}

// flag -p
typedef enum {
    PRIME_NEITHER,
    PRIME_YES,
    PRIME_NO
} PrimeState;

int check_prime(long x, PrimeState *out) {
    if (!out) {
        return RANGE_ERR;
    }
    if (x < 0) {
        return RANGE_ERR;
    }
    if (x == 0 || x == 1) {
        *out = PRIME_NEITHER;
        return OK;
    }

    bool prime = true;
    for (long i = 2; i <= x / i; ++i) {
        if (x % i == 0) {
            prime = false;
            break;
        }
    }
    *out = prime ? PRIME_YES : PRIME_NO;
    return OK;
}

void print_prime(PrimeState state) {
    switch (state) {
        case PRIME_YES:
            printf("Prime number\n");
            break;
        case PRIME_NO:
            printf("Composite number\n");
            break;
        default:
            printf("Neither prime nor composite\n");
            break;
    }
}

// -s
int calc_hex_digits(long x, char *digits, size_t buf_size, size_t *out_count, bool *out_negative) {
    if (!digits || !out_count || !out_negative) {
        return RANGE_ERR;
    }

    const char NS16[] = "0123456789ABCDEF";
    size_t count = 0;
    *out_negative = false;

    if (x == 0) {
        if (buf_size < 1) {
            return RANGE_ERR;
        }
        digits[count++] = '0';
        *out_count = count;
        return OK;
    }

    unsigned long temp;
    if (x < 0) {
        *out_negative = true;
        temp = (unsigned long)(-(x + 1)) + 1;
    } else {
        temp = (unsigned long)x;
    }

    while (temp != 0) {
        if (count >= buf_size) {
            return RANGE_ERR;
        }
        digits[count++] = NS16[temp % 16];
        temp /= 16;
    }

    *out_count = count;
    return OK;
}

void print_hex_digits(const char *digits, size_t count, bool negative) {
    if (negative) {
        printf("-");
    }
    for (size_t i = count; i > 0; --i) {
        printf("%c", digits[i - 1]);
        if (i > 1) {
            printf(" ");
        }
    }
    printf("\n");
}

// -e
int calc_powers(long x, unsigned long long table[MAX_EXP][MAX_BASE], size_t *out_rows) {
    if (!table || !out_rows) {
        return RANGE_ERR;
    }
    if (x < 1 || x > MAX_EXP) {
        return RANGE_ERR;
    }

    for (long exp = 1; exp <= x; ++exp) {
        for (long base = 1; base <= MAX_BASE; ++base) {
            unsigned long long res = 1;
            for (long k = 0; k < exp; ++k) {
                if (res > ULLONG_MAX / (unsigned long long)base) {
                    return OVERFLOW_ERR;
                }
                res *= (unsigned long long)base;
            }
            table[exp - 1][base - 1] = res;
        }
    }

    *out_rows = (size_t)x;
    return OK;
}

void print_powers(unsigned long long table[MAX_EXP][MAX_BASE], size_t rows) {
    printf("Exponent\tBase\tResult\n");
    for (size_t e = 0; e < rows; ++e) {
        for (size_t b = 0; b < MAX_BASE; ++b) {
            printf("%zu\t\t%zu\t%llu\n", e + 1, b + 1, table[e][b]);
        }
    }
}

// -a
int calc_sum(long x, unsigned long long *out) {
    if (!out) {
        return RANGE_ERR;
    }
    if (x < 1) {
        return RANGE_ERR;
    }

    unsigned long long n = (unsigned long long)x;
    if (n > ULLONG_MAX / (n + 1)) {
        return OVERFLOW_ERR;
    }
    *out = n * (n + 1) / 2;
    return OK;
}

void print_sum(long x, unsigned long long res) {
    printf("Sum 1 .. %ld = %llu\n", x, res);
}

// -f
int calc_factorial(long x, unsigned long long *out) {
    if (!out) {
        return RANGE_ERR;
    }
    if (x < 0) {
        return RANGE_ERR;
    }
    if (x > 20) {
        return OVERFLOW_ERR;
    }

    unsigned long long res = 1;
    for (long i = 2; i <= x; ++i) {
        if (res > ULLONG_MAX / (unsigned long long)i) {
            return OVERFLOW_ERR;
        }
        res *= (unsigned long long)i;
    }
    *out = res;
    return OK;
}

void print_factorial(long x, unsigned long long res) {
    printf("%ld! = %llu\n", x, res);
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <x> <flag>\n", argv[0]);
        return USAGE_ERR;
    }

    char *str_x = argv[1];
    char *flag = argv[2];

    long x;
    if (!parse_long(str_x, &x)) {
        fprintf(stderr, "Warning: <x> should be a number\n");
        return NUMBER_INPUT_ERR;
    }

    if (strlen(flag) != 2) {
        fprintf(stderr, "Warning: flag should be 2-character\n");
        return FLAG_INPUT_ERR;
    }
    if (flag[0] != '-' && flag[0] != '/') {
        fprintf(stderr, "Warning: flag should have '-' or '/'\n");
        return FLAG_INPUT_ERR;
    }
    if (flag[0] == '/') {
        flag[0] = '-';
    }

    int status = OK;

    if (strcmp(flag, "-h") == 0) {
        long arr[MAX_MULTIPLES];
        size_t count = 0;
        status = calc_multiples(x, arr, MAX_MULTIPLES, &count);
        if (status == OK) {
            print_multiples(arr, count);
        }
    }
    else if (strcmp(flag, "-p") == 0) {
        PrimeState state = PRIME_NEITHER;
        status = check_prime(x, &state);
        if (status == OK) {
            print_prime(state);
        }
    }
    else if (strcmp(flag, "-s") == 0) {
        char digits[MAX_HEX_DIGITS];
        size_t count = 0;
        bool negative = false;
        status = calc_hex_digits(x, digits, MAX_HEX_DIGITS,
                                 &count, &negative);
        if (status == OK) {
            print_hex_digits(digits, count, negative);
        }
    }
    else if (strcmp(flag, "-e") == 0) {
        unsigned long long table[MAX_EXP][MAX_BASE];
        size_t rows = 0;
        status = calc_powers(x, table, &rows);
        if (status == OK) {
            print_powers(table, rows);
        }
    }
    else if (strcmp(flag, "-a") == 0) {
        unsigned long long res = 0;
        status = calc_sum(x, &res);
        if (status == OK) {
            print_sum(x, res);
        }
    }
    else if (strcmp(flag, "-f") == 0) {
        unsigned long long res = 0;
        status = calc_factorial(x, &res);
        if (status == OK) {
            print_factorial(x, res);
        }
    }
    else {
        fprintf(stderr, "Warning: unknown flag\n");
        return FLAG_INPUT_ERR;
    }

    switch (status) {
        case OK:
            break;
        case RANGE_ERR:
            fprintf(stderr, "Error: number out of range for this flag\n");
            break;
        case OVERFLOW_ERR:
            fprintf(stderr, "Error: result overflow\n");
            break;
        default:
            fprintf(stderr, "Error: unknown error\n");
            break;
    }

    return status;
}