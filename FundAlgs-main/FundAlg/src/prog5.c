#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#define OK 0
#define USAGE_ERR 1
#define NUMBER_INPUT_ERR 2
#define RANGE_ERR 3
#define NO_CONVERGENCE_ERR 4

#define MAX_ITER 10000000L   
#define MAX_ITER_INT 1000000L    
#define INIT_N 2

// парсер для вещественных чисел
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

// сравнение вещественных чисел
static int dbl_cmp(double a, double b, double eps) {
    if (fabs(a - b) < eps) return 0;
    return (a > b) ? 1 : -1;
}

// a1
static int calc_sum_a(double x, double eps, double *out) {
    if (!out) return RANGE_ERR;
    if (eps <= 0.0) return RANGE_ERR;

    double sum  = 1.0;
    double term = 1.0;
    size_t n = 0;

    while ((n < MAX_ITER) && (fabs(term) >= eps)) {
        n++;
        term = term * x / (double)n;
        sum += term;
        if (!isfinite(term) || !isfinite(sum)) {
            return NO_CONVERGENCE_ERR;
        }
    }

    if (n >= MAX_ITER) return NO_CONVERGENCE_ERR;
    *out = sum;
    return OK;
}

// b1
static int calc_sum_b(double x, double eps, double *out) {
    if (!out) return RANGE_ERR;
    if (eps <= 0.0) return RANGE_ERR;

    double sum  = 1.0;
    double term = 1.0;
    size_t n = 0;

    while ((n < MAX_ITER) && (fabs(term) >= eps)) {
        n++;
        term = -term * x * x / ((2.0 * n - 1.0) * (2.0 * n));
        sum += term;
        if (!isfinite(term) || !isfinite(sum)) {
            return NO_CONVERGENCE_ERR;
        }
    }

    if (n >= MAX_ITER) return NO_CONVERGENCE_ERR;
    *out = sum;
    return OK;
}

// c1
static int calc_sum_c(double x, double eps, double *out) {
    if (!out) return RANGE_ERR;
    if (eps <= 0.0) return RANGE_ERR;

    double sum = 1.0;
    double term = 1.0;
    size_t n = 0;

    while ((n < MAX_ITER) && (fabs(term) >= eps)) {
        n++;
        double nf = (double)n;
        term = term * 27.0 * nf * nf * nf * x * x / ((3.0 * nf - 2.0) * (3.0 * nf - 1.0) * (3.0 * nf));
        sum += term;
        if (!isfinite(term) || !isfinite(sum)) {
            return NO_CONVERGENCE_ERR;
        }
    }

    if (n >= MAX_ITER) return NO_CONVERGENCE_ERR;
    *out = sum;
    return OK;
}

// d1
static int calc_sum_d(double x, double eps, double *out) {
    if (!out) return RANGE_ERR;
    if (eps <= 0.0) return RANGE_ERR;

    double term = -x * x / 2.0;
    double sum  = term;
    size_t n = 1;

    while ((n < MAX_ITER) && (fabs(term) >= eps)) {
        n++;
        term = -term * x * x * (2.0 * n - 1.0) / (2.0 * n);
        sum += term;
        if (!isfinite(term) || !isfinite(sum)) {
            return NO_CONVERGENCE_ERR;
        }
    }

    if (n >= MAX_ITER) return NO_CONVERGENCE_ERR;
    *out = sum;
    return OK;
}

static double simpson(double (*f)(double), double a, double b, size_t n) {
    double h = (b - a) / (double)n;
    double s = f(a) + f(b);

    for (size_t i = 1; i < n; i++) {
        double x = a + (double)i * h;
        double y = f(x);
        s += (i % 2 == 0) ? 2.0 * y : 4.0 * y;
    }
    return s * h / 3.0;
}

static int integrate(double (*f)(double), double a, double b, double eps, double *out) {
    if (!f || !out) return RANGE_ERR;
    if (eps <= 0.0) return RANGE_ERR;

    size_t n = INIT_N;
    double prev = simpson(f, a, b, n);

    for (long i = 0; i < MAX_ITER_INT; i++) {
        n *= 2;
        double cur = simpson(f, a, b, n);
        if (fabs(cur - prev) < eps) {
            *out = cur;
            return OK;
        }
        prev = cur;
    }
    return NO_CONVERGENCE_ERR;
}

// a2
static double f_a(double x) {
    if (dbl_cmp(x, 0.0, 1e-6) == 0) return 1.0;
    return log(1.0 + x) / x;
}

// b2
static double f_b(double x) {
    return exp(-x * x / 2.0);
}

// c2
static double f_c(double x) {
    if (dbl_cmp(x, 0.0, 1e-6) == 0) return 0.0;
    return -4.0 * x * log(x);
}

// d2
static double f_d(double x) {
    if (dbl_cmp(x, 0.0, 1e-6) == 0) return 1.0;
    return pow(x, x);
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <eps> <x>\n", argv[0]);
        return USAGE_ERR;
    }

    double eps, x;
    if (!parse_double(argv[1], &eps)) {
        fprintf(stderr, "Warning: <eps> should be a number\n");
        return NUMBER_INPUT_ERR;
    }
    if (!parse_double(argv[2], &x)) {
        fprintf(stderr, "Warning: <x> should be a number\n");
        return NUMBER_INPUT_ERR;
    }
    if (eps <= 0.0) {
        fprintf(stderr, "Warning: <eps> should be positive\n");
        return RANGE_ERR;
    }

    printf("eps = %g, x = %g\n", eps, x);

    double v;
    int s;

    printf("\n 5.1: sums \n");

    v = 0.0;
    s = calc_sum_a(x, eps, &v);
    if (s == OK) printf("a) sum x^n/n! = %.15f\n", v);
    else printf("a) error %d\n", s);

    v = 0.0;
    s = calc_sum_b(x, eps, &v);
    if (s == OK) printf("b) sum (-1)^n x^(2n)/(2n)! = %.15f\n", v);
    else printf("b) error %d\n", s);

    v = 0.0;
    s = calc_sum_c(x, eps, &v);
    if (s == OK) printf("c) sum 3^(3n)(n!)^3 x^(2n)/(3n)! = %.15f\n", v);
    else printf("c) error %d\n", s);

    v = 0.0;
    s = calc_sum_d(x, eps, &v);
    if (s == OK) printf("d) sum (-1)^n (2n-1)!! x^(2n)/(2n)!! = %.15f\n", v);
    else printf("d) error %d\n", s);

    printf("\n 5.2: integrals \n");

    v = 0.0;
    s = integrate(f_a, 0.0, 1.0, eps, &v);
    if (s == OK) printf("a) int_0^1 ln(1+x)/x dx = %.15f\n", v);
    else printf("a) error %d\n", s);

    v = 0.0;
    s = integrate(f_b, 0.0, 1.0, eps, &v);
    if (s == OK) printf("b) int_0^1 e^(-x^2/2) dx = %.15f\n", v);
    else printf("b) error %d\n", s);

    v = 0.0;
    s = integrate(f_c, 0.0, 1.0, eps, &v);
    if (s == OK) printf("c) int_0^1 ln(1/(1-x)) dx = %.15f\n", v);
    else printf("c) error %d\n", s);

    v = 0.0;
    s = integrate(f_d, 0.0, 1.0, eps, &v);
    if (s == OK) printf("d) int_0^1 x^x dx = %.15f\n", v);
    else printf("d) error %d\n", s);

    return OK;
}