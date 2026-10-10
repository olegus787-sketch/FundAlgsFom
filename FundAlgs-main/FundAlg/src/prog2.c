#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define OK 0
#define USAGE_ERR 1
#define OVERFLOW 2
#define TOO_SMALL_PRECISION 3
#define NUMBER_INPUT_ERR 4

#define MAX_ITER 10000000L
#define MAX_GAMMA_M 25
#define MAX_GAMMA_T 1000000L

// парсер для точности
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

// e
int calc_e_series (double eps, double *out) {
    size_t n = 0;
    double sum = 0;
    double term = 1;

    while ((n < MAX_ITER) && (fabs(term) >= eps)) {
        if (n == 0) {
            sum += 1;
        }
        else {
            term /= n;
            sum += term;

        }
        n++;
    }

    if (!(n < MAX_ITER)) {
        return TOO_SMALL_PRECISION;
    }

    *out = sum;
    return OK;
}

int calc_e_limit (double eps, double *out) {
    size_t n = 1;
    double res = 0;
    double prev = eps + 1.0;

    while ((n < MAX_ITER) && (n * fabs(res - prev) >= eps)) {
        prev = res;
        res = pow(1.0 + 1.0/(double)n, (double)n);
        n++;
    }

    if(!(n < MAX_ITER)) {
        return TOO_SMALL_PRECISION;
    }

    *out = res;
    return OK;
}

int calc_e_equation(double eps, double *out) {
    double a = 2, b = 3;
    double func = 0, mid = (b + a) / 2;
    size_t n = 0;
    while ((n < MAX_ITER) && ((b - a) >= eps)) {
        func = log(mid) - 1;
        if (func > eps) b = mid;
        else if (func < -eps) a = mid;
        else break;
        mid = (b + a) / 2;
        n++;
    }

    if (!(n < MAX_ITER)) {
        return TOO_SMALL_PRECISION;
    }

    *out = mid;
    return OK;
}

// pi
int calc_pi_series(double eps, double *out) {
    size_t n = 1;
    double sum = 0;
    double term = 4;

    while ((n < MAX_ITER) && (fabs(term) >= eps)) {
        sum += term;
        term = -term * (2.0 * n - 1) / (2.0 * n + 1);
        n++;
    }

    if (!(n < MAX_ITER)) {
        return TOO_SMALL_PRECISION;
    }

    *out = sum;
    return OK;
}

int calc_pi_limit(double eps, double *out) {
    size_t n = 1;
    double res = 4;
    double prev = 0;

    while ((n < MAX_ITER) && (n * fabs(res - prev) >= eps)) {
        prev = res;
        res = res * (4.0 * n * (n + 1)) / ((2.0 * n + 1) * (2.0 * n + 1));
        n++;
    }

    if (!(n < MAX_ITER)) {
        return TOO_SMALL_PRECISION;
    }

    *out = res;
    return OK;
}

/* Уравнение cos(x) = -1 эквивалентно cos(x/2) = 0, но y = cos(x/2) меняет знак в этой точке, поэтому
использую именно его */
int calc_pi_equation(double eps, double *out) {
    double a = 3, b = 4;
    double func = 0, mid = (b + a) / 2;
    size_t n = 0;

    while ((n < MAX_ITER) && ((b - a) >= eps)) {
        func = cos(mid / 2);
        if (func > eps) a = mid;
        else if (func < -eps) b = mid;
        else break;
        mid = (b + a) / 2;
        n++;
    }

    if (!(n < MAX_ITER)) {
        return TOO_SMALL_PRECISION;
    }

    *out = mid;
    return OK;
}

// ln2
int calc_ln2_series(double eps, double *out) {
    size_t n = 1;
    double sum = 0;
    double term = 1;

    while ((n < MAX_ITER) && (fabs(term) >= eps)) {
        sum += term;
        term = -term * (double)n / (double)(n + 1);
        n++;
    }

    if (!(n < MAX_ITER)) {
        return TOO_SMALL_PRECISION;
    }

    *out = sum;
    return OK;
}

int calc_ln2_limit(double eps, double *out) {
    size_t n = 1;
    double res = 0;
    double prev = eps + 1.0;

    while ((n < MAX_ITER) && (n * fabs(res - prev) >= eps)) {
        prev = res;
        res = (double)n * (pow(2.0, 1.0 / (double)n) - 1.0);
        n++;
    }

    if (!(n < MAX_ITER)) {
        return TOO_SMALL_PRECISION;
    }

    *out = res;
    return OK;
}

int calc_ln2_equation(double eps, double *out) {
    double a = 0, b = 1;
    double func = 0, mid = (b + a) / 2;
    size_t n = 0;

    while ((n < MAX_ITER) && ((b - a) >= eps)) {
        func = exp(mid) - 2;
        if (func > eps) b = mid;
        else if (func < -eps) a = mid;
        else break;
        mid = (b + a) / 2;
        n++;
    }

    if (!(n < MAX_ITER)) {
        return TOO_SMALL_PRECISION;
    }

    *out = mid;
    return OK;
}

// sqrt2
int calc_sqrt2_product(double eps, double *out) {
    size_t k = 2;
    double prod = 1;
    double denom = 4;
    double term = 0;

    while ((k < MAX_ITER) && (fabs(term - 1.0) >= eps)) {
        term = pow(2.0, 1.0 / denom);
        prod *= term;
        denom *= 2;
        k++;
    }

    if (!(k < MAX_ITER)) {
        return TOO_SMALL_PRECISION;
    }

    *out = prod;
    return OK;
}

int calc_sqrt2_limit(double eps, double *out) {
    size_t n = 0;
    double res = -0.5;
    double prev = eps + 1.0;

    while ((n < MAX_ITER) && (fabs(res - prev) >= eps)) {
        prev = res;
        res = res - res * res / 2.0 + 1.0;
        n++;
    }

    if (!(n < MAX_ITER)) {
        return TOO_SMALL_PRECISION;
    }

    *out = res;
    return OK;
}

int calc_sqrt2_equation(double eps, double *out) {
    double a = 1, b = 2;
    double func = 0, mid = (b + a) / 2;
    size_t n = 0;

    while ((n < MAX_ITER) && ((b - a) >= eps)) {
        func = mid * mid - 2;
        if (func > eps) b = mid;
        else if (func < -eps) a = mid;
        else break;
        mid = (b + a) / 2;
        n++;
    }

    if (!(n < MAX_ITER)) {
        return TOO_SMALL_PRECISION;
    }

    *out = mid;
    return OK;
}

// gamma
int calc_gamma_series(double eps, double *out) {
    const double pi = acos(-1.0);

    double sum = 7.0 / 6.0;
    size_t n = 2;
    size_t limit = (size_t)(2.0 / eps);

    while (n < limit && n < MAX_ITER) {
        double na = (double)(n * n);
        double nb = (double)((n + 1) * (n + 1) - 1);

        double inv_a = 1.0 / na;
        double inv_b = 1.0 / nb;

        double harmonic = log(nb / na)
                        + 0.5 * (inv_a + inv_b)
                        + (inv_a * inv_a - inv_b * inv_b) / 12.0;

        double block = (nb - na + 1.0) / (na) - harmonic;
        sum += block;
        n++;
    }

    if (n >= MAX_ITER) return TOO_SMALL_PRECISION;

    *out = -pi * pi / 6.0 + sum;
    return OK;
}

int calc_gamma_limit(double eps, double *out) {
    size_t m = 1;
    double res = 0;
    double prev = eps + 1.0;
    size_t n = 0;

    while ((n < MAX_ITER) && (m < MAX_GAMMA_M) &&
           (fabs(res - prev) >= eps)) {
        prev = res;
        m++;

        double sum = 0;
        double binom = 1;
        double ln_fact = 0;

        for (size_t k = 1; k <= m; k++) {
            binom = binom * (double)(m - k + 1) / (double)k;
            ln_fact += log((double)k);
            double term = binom * ln_fact / (double)k;
            if (k % 2 == 1) sum -= term;
            else            sum += term;
        }

        res = sum;
        n++;
    }

    if (m >= MAX_GAMMA_M || !(n < MAX_ITER)) {
        return TOO_SMALL_PRECISION;
    }

    *out = res;
    return OK;
}

int compute_mertens_limit(double eps, double *out) {
    size_t t = 100;
    double res = 0;
    double prev = eps + 1.0;
    size_t n = 0;

    while ((n < MAX_ITER) && (t < MAX_GAMMA_T) && (fabs(res - prev) >= eps)) {
        prev = res;

        char *is_prime = calloc(t + 1, sizeof(char));
        if (!is_prime) return OVERFLOW;

        for (size_t i = 2; i <= t; i++) is_prime[i] = 1;
        for (size_t i = 2; i * i <= t; i++) {
            if (is_prime[i])
                for (size_t j = i * i; j <= t; j += i)
                    is_prime[j] = 0;
        }

        double prod = 1.0;
        for (size_t p = 2; p <= t; p++) {
            if (is_prime[p])
                prod *= (double)(p - 1) / (double)p;
        }
        free(is_prime);

        res = log((double)t) * prod;
        t *= 2;
        n++;
    }

    if ((t >= MAX_GAMMA_T) || !(n < MAX_ITER)) 
        return TOO_SMALL_PRECISION;

    *out = res;
    return OK;
}

int calc_gamma_equation(double eps, double *out) {
    double L;
    int status = compute_mertens_limit(eps, &L);
    if (status != OK) return status;

    double a = 0.0, b = 1.0;
    double mid = (a + b) / 2.0;
    size_t n = 0;

    while ((n < MAX_ITER) && ((b - a) >= eps)) {
        double func = exp(-mid) - L;
        if (func > eps) a = mid;
        else if (func < -eps) b = mid;
        else break;
        mid = (a + b) / 2.0;
        n++;
    }

    if (!(n < MAX_ITER)) return TOO_SMALL_PRECISION;

    *out = mid;
    return OK;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <eps>\n", argv[0]);
        return USAGE_ERR;
    }

    double eps;
    if (!parse_double(argv[1], &eps)) {
        fprintf(stderr, "Warning: <eps> should be a number\n");
        return NUMBER_INPUT_ERR;
    }
    if (eps <= 0.0) {
        fprintf(stderr, "Warning: <eps> should be positive\n");
        return USAGE_ERR;
    }

    double v;
    int s;

    // e
    printf("e:\n");

    v = 0.0;
    s = calc_e_series(eps, &v);
    if (s == OK) printf("series: %.15f\n", v);
    else printf("series: error %d\n", s);

    v = 0.0;
    s = calc_e_limit(eps, &v);
    if (s == OK) printf("limit: %.15f\n", v);
    else printf("limit: error %d\n", s);

    v = 0.0;
    s = calc_e_equation(eps, &v);
    if (s == OK) printf("equation: %.15f\n", v);
    else printf("equation: error %d\n", s);

    // pi
    printf("\npi:\n");

    v = 0.0;
    s = calc_pi_series(eps, &v);
    if (s == OK) printf("series: %.15f\n", v);
    else printf("series: error %d\n", s);

    v = 0.0;
    s = calc_pi_limit(eps, &v);
    if (s == OK) printf("limit: %.15f\n", v);
    else printf("limit: error %d\n", s);

    v = 0.0;
    s = calc_pi_equation(eps, &v);
    if (s == OK) printf("equation: %.15f\n", v);
    else printf("equation: error %d\n", s);

    // ln2
    printf("\nln2:\n");

    v = 0.0;
    s = calc_ln2_series(eps, &v);
    if (s == OK) printf("series: %.15f\n", v);
    else printf("series: error %d\n", s);

    v = 0.0;
    s = calc_ln2_limit(eps, &v);
    if (s == OK) printf("limit: %.15f\n", v);
    else printf("limit: error %d\n", s);

    v = 0.0;
    s = calc_ln2_equation(eps, &v);
    if (s == OK) printf("equation: %.15f\n", v);
    else printf("equation: error %d\n", s);

    // sqrt2
    printf("\nsqrt2:\n");

    v = 0.0;
    s = calc_sqrt2_product(eps, &v);
    if (s == OK) printf("product: %.15f\n", v);
    else printf("product: error %d\n", s);

    v = 0.0;
    s = calc_sqrt2_limit(eps, &v);
    if (s == OK) printf("limit: %.15f\n", v);
    else printf("limit: error %d\n", s);

    v = 0.0;
    s = calc_sqrt2_equation(eps, &v);
    if (s == OK) printf("equation: %.15f\n", v);
    else printf("equation: error %d\n", s);

    // gamma
    printf("\ngamma:\n");

    v = 0.0;
    s = calc_gamma_limit(eps, &v);
    if (s == OK) printf("limit: %.15f\n", v);
    else printf("limit: error %d\n", s);

    v = 0.0;
    s = calc_gamma_series(eps, &v);
    if (s == OK) printf("series: %.15f\n", v);
    else printf("series: error %d\n", s);

    v = 0.0;
    s = calc_gamma_equation(eps, &v);
    if (s == OK) printf("equation: %.15f\n", v);
    else printf("equation: error %d\n", s);

    return OK;
}