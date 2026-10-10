#include <errno.h>
#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define OK 0
#define USAGE_ERR 1
#define FLAG_INPUT_ERR 2
#define NUMBER_INPUT_ERR 3
#define RANGE_ERR 4

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

static int parse_double(const char *s, double *out) {
    if (!s || !*s || !out) return 0;

    char *end = NULL;
    double v = strtod(s, &end);

    if (end == s) return 0;
    if (*end != '\0') return 0;
    if (!isfinite(v)) return 0;

    *out = v;
    return 1;
}

static int dbl_cmp(double a, double b, double eps) {
    if (fabs(a - b) < eps) return 0;
    return (a > b) ? 1 : -1;
}

// -m
static int calc_multiplicity(long a, long b, int *out_is_multiple) {
    if (!out_is_multiple) return RANGE_ERR;
    if (a == 0 || b == 0) return RANGE_ERR;

    if (b == -1) {
        *out_is_multiple = 1;
        return OK;
    }

    *out_is_multiple = (a % b == 0);
    return OK;
}

static void print_multiplicity(long a, long b, int is_multiple) {
    if (is_multiple) printf("%ld is a multiple of %ld\n", a, b);
    else printf("%ld is not a multiple of %ld\n", a, b);
}

// -t
static int check_right_triangle(double eps, double a, double b, double c, int *out_is_right) {
    if (!out_is_right) return RANGE_ERR;
    if (eps <= 0.0) return RANGE_ERR;

    if (dbl_cmp(a, 0.0, eps) <= 0) return RANGE_ERR;
    if (dbl_cmp(b, 0.0, eps) <= 0) return RANGE_ERR;
    if (dbl_cmp(c, 0.0, eps) <= 0) return RANGE_ERR;

    double aa = a * a;
    double bb = b * b;
    double cc = c * c;

    if (dbl_cmp(aa + bb, cc, eps) == 0 || dbl_cmp(aa + cc, bb, eps) == 0 || dbl_cmp(bb + cc, aa, eps) == 0) {
        *out_is_right = 1;
    } else {
        *out_is_right = 0;
    }
    return OK;
}

static void print_right_triangle(int is_right) {
    if (is_right) {
        printf("right triangle\n");
    } else {
        printf("not right triangle\n");
    }
}

// -q
typedef struct {
    double a, b, c;
} Coefs;

static int coefs_equal(Coefs x, Coefs y, double eps) {
    return dbl_cmp(x.a, y.a, eps) == 0 && dbl_cmp(x.b, y.b, eps) == 0 && dbl_cmp(x.c, y.c, eps) == 0;
}

static int solve_quadratic(double a, double b, double c, double eps, int *out_n, double *out_x1, double *out_x2) {
    if (!out_n || !out_x1 || !out_x2) return RANGE_ERR;

    if (dbl_cmp(a, 0.0, eps) == 0) {
        if (dbl_cmp(b, 0.0, eps) == 0) {
            *out_n = 0;
            return OK;
        }

        *out_x1 = -c / b;
        *out_n = 1;
        return OK;
    }

    double disc = b * b - 4.0 * a * c;

    if (dbl_cmp(disc, 0.0, eps) < 0) {
        *out_n = 0;
    } else if (dbl_cmp(disc, 0.0, eps) == 0) {
        *out_x1 = -b / (2.0 * a);
        *out_n = 1;
    } else {
        double sd = sqrt(disc);
        *out_x1 = (-b - sd) / (2.0 * a);
        *out_x2 = (-b + sd) / (2.0 * a);
        *out_n = 2;
    }

    return OK;
}

static void print_quadratic(Coefs k, int n, double x1, double x2) {
    printf("a = %g, b = %g, c = %g:\n", k.a, k.b, k.c);
    if (n == 0) {
        printf("no real roots\n");
    } else if (n == 1) {
        printf("x = %.6f\n", x1);
    } else {
        printf("x1 = %.6f, x2 = %.6f\n", x1, x2);
    }
}

static int flag_q(double eps, const double coef[3]) {
    static const int perms[6][3] = {
        {0, 1, 2}, {0, 2, 1},
        {1, 0, 2}, {1, 2, 0},
        {2, 0, 1}, {2, 1, 0}
    };

    Coefs seen[6];
    size_t seen_count = 0;

    for (size_t p = 0; p < 6; p++) {
        Coefs cur;
        cur.a = coef[perms[p][0]];
        cur.b = coef[perms[p][1]];
        cur.c = coef[perms[p][2]];

        int dup = 0;
        for (size_t s = 0; s < seen_count; s++) {
            if (coefs_equal(cur, seen[s], eps)) {
                dup = 1;
                break;
            }
        }
        if (dup) continue;

        seen[seen_count++] = cur;

        int n_roots = 0;
        double x1 = 0, x2 = 0;
        int status = solve_quadratic(cur.a, cur.b, cur.c, eps, &n_roots, &x1, &x2);
        if (status != OK) return status;

        print_quadratic(cur, n_roots, x1, x2);
    }
    return OK;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <flag> ...\n", argv[0]);
        return USAGE_ERR;
    }

    char *flag = argv[1];

    if (flag[0] != '-' && flag[0] != '/') {
        fprintf(stderr, "Warning: flag should start with '-' or '/'\n");
        return FLAG_INPUT_ERR;
    }
    if (flag[0] == '/') flag[0] = '-';

    // -q
    if (strcmp(flag, "-q") == 0) {
        if (argc != 6) {
            fprintf(stderr, "Usage: %s -q <eps> <a> <b> <c>\n", argv[0]);
            return USAGE_ERR;
        }

        double eps, coef[3];
        if (!parse_double(argv[2], &eps)) {
            fprintf(stderr, "Warning: <eps> should be a number\n");
            return NUMBER_INPUT_ERR;
        }
        if (eps <= 0.0) {
            fprintf(stderr, "Warning: <eps> should be positive\n");
            return RANGE_ERR;
        }
        for (int i = 0; i < 3; i++) {
            if (!parse_double(argv[3 + i], &coef[i])) {
                fprintf(stderr, "Warning: coefficient should be a number\n");
                return NUMBER_INPUT_ERR;
            }
        }

        return flag_q(eps, coef);
    }

    // -m
    if (strcmp(flag, "-m") == 0) {
        if (argc != 4) {
            fprintf(stderr, "Usage: %s -m <a> <b>\n", argv[0]);
            return USAGE_ERR;
        }

        long a, b;
        if (!parse_long(argv[2], &a)) {
            fprintf(stderr, "Warning: <a> should be integer\n");
            return NUMBER_INPUT_ERR;
        }
        if (!parse_long(argv[3], &b)) {
            fprintf(stderr, "Warning: <b> should be integer\n");
            return NUMBER_INPUT_ERR;
        }

        int is_mult = 0;
        int status = calc_multiplicity(a, b, &is_mult);
        if (status == OK) print_multiplicity(a, b, is_mult);
        return status;
    }

    // -t
    if (strcmp(flag, "-t") == 0) {
        if (argc != 6) {
            fprintf(stderr, "Usage: %s -t <eps> <a> <b> <c>\n", argv[0]);
            return USAGE_ERR;
        }

        double eps, a, b, c;
        if (!parse_double(argv[2], &eps)) {
            fprintf(stderr, "Warning: <eps> should be a number\n");
            return NUMBER_INPUT_ERR;
        }
        if (!parse_double(argv[3], &a) ||
            !parse_double(argv[4], &b) ||
            !parse_double(argv[5], &c)) {
            fprintf(stderr, "Warning: side should be a number\n");
            return NUMBER_INPUT_ERR;
        }

        int is_right = 0;
        int status = check_right_triangle(eps, a, b, c, &is_right);
        if (status == OK) print_right_triangle(is_right);
        return status;
    }

    fprintf(stderr, "Warning: unknown flag\n");
    return FLAG_INPUT_ERR;
}