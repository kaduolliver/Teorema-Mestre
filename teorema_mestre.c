/* Analise experimental de algoritmos recursivos - C11, GCC e POSIX.
 * Uso: ./tempo problema expoente amostras dados.csv
 * n = 4^expoente no P1; n = 2^expoente no P2 e no P3.
 * Compilar com -DINSTRUMENT para registrar contagens, sem tempo.
 */
#define _POSIX_C_SOURCE 200809L
#include <errno.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define REC __attribute__((noinline, noipa))
static uint64_t operacoes;
#ifdef INSTRUMENT
#define CONTA() (++operacoes)
#else
#define CONTA() ((void)0)
#endif

/* P1: maior elemento. Quatro chamadas de tamanho n/4.
 * Conta uma visita por chamada. Precondicao: n = 4^k. */
REC uint32_t maior4(const uint32_t *v, size_t n) {
    CONTA();
    if (n == 1) return v[0];
    size_t m = n / 4;
    uint32_t a = maior4(v, m), b = maior4(v + m, m);
    uint32_t c = maior4(v + 2*m, m), d = maior4(v + 3*m, m);
    uint32_t maior = a > b ? a : b;
    if (c > maior) maior = c;
    if (d > maior) maior = d;
    return maior;
}

/* P2: busca binaria. Escolhe uma metade e desce ate n = 1.
 * Vetor ordenado; n = 2^k. Retorna 1 se encontrou, senao 0.
 * Esta variante inclui o meio na metade direita, sem subtrair 1.
 * Conta uma visita por chamada. */
REC int busca(const uint32_t *v, size_t n, uint32_t alvo) {
    CONTA();
    if (n == 1) return v[0] == alvo;
    size_t m = n / 2;
    if (alvo < v[m]) return busca(v, m, alvo);
    return busca(v + m, m, alvo);
}

/* P3: soma. A primeira metade e recursiva; a segunda usa um laco.
 * Conta uma leitura de elemento por folha ou iteracao. n = 2^k. */
REC uint64_t soma(const uint32_t *v, size_t n) {
    if (n == 1) { CONTA(); return v[0]; }
    size_t m = n / 2;
    uint64_t total = soma(v, m);
    for (size_t i = m; i < n; ++i) { total += v[i]; CONTA(); }
    return total;
}

static void erro(const char *msg) {
    fprintf(stderr, "%s\n", msg); exit(EXIT_FAILURE);
}
static int numero(const char *s, int min, int max) {
    char *fim; errno = 0;
    long x = strtol(s, &fim, 10);
    if (errno || fim == s || *fim || x < min || x > max)
        erro("Argumento numerico invalido");
    return (int)x;
}
static size_t potencia(size_t b, int k) {
    size_t n = 1;
    while (k-- > 0) n *= b;
    return n;
}
static uint64_t executar(int p, const uint32_t *v, size_t n) {
    if (p == 1) return maior4(v, n);
    if (p == 2) return (uint64_t)busca(v, n, (uint32_t)n);
    return soma(v, n);
}
#ifndef INSTRUMENT
static double agora(void) {
    struct timespec t;
    if (clock_gettime(CLOCK_MONOTONIC, &t)) erro("Falha no relogio");
    return (double)t.tv_sec + (double)t.tv_nsec * 1e-9;
}
/* Mede o lote inteiro com apenas duas leituras de relogio.
 * Acumular e verificar a saida preserva os resultados do lote.
 * noipa impede memoizacao/eliminacao das chamadas repetidas. */
static double lote(int p, const uint32_t *v, size_t n, size_t b,
                   uint64_t *checagem) {
    uint64_t total = 0;
    double inicio = agora();
    for (size_t r = 0; r < b; ++r) total += executar(p, v, n);
    double dt = agora() - inicio;
    *checagem = total;
    return dt;
}
#endif

int main(int argc, char **argv) {
    if (argc != 5) erro("Uso: programa problema expoente amostras arquivo.csv");
    int p = numero(argv[1], 1, 3);
    int k = numero(argv[2], 0, p == 1 ? 10 : (p == 2 ? 25 : 20));
    int amostras = numero(argv[3], 1, 99);
    size_t n = potencia(p == 1 ? 4 : 2, k);
    uint32_t *v = malloc(n * sizeof(*v));
    if (!v) erro("Falha de memoria");
    /* Dados deterministas e reproduziveis: 0,1,...,n-1. Alvo n ausente. */
    for (size_t i = 0; i < n; ++i) v[i] = (uint32_t)i;
    uint64_t esperado = p == 1 ? n-1 :
                        (p == 2 ? 0 : (uint64_t)n*(n-1)/2);
    FILE *f = fopen(argv[4], "a+");
    if (!f || fseek(f, 0, SEEK_END)) erro("Falha ao abrir CSV");
    if (ftell(f) == 0)
        fprintf(f, "problema,n,expoente,modo,amostra,lote,segundos,"
                   "operacoes,resultado,valido\n");
    uint64_t resultado = executar(p, v, n);
    if (resultado != esperado) erro("Resultado incorreto");
#ifdef INSTRUMENT
    (void)amostras;
    uint64_t formula = p == 1 ? (4*(uint64_t)n-1)/3 :
                       (p == 2 ? (uint64_t)k+1 : n);
    if (operacoes != formula) erro("Contagem incorreta");
    fprintf(f, "%d,%zu,%d,contagem,0,1,,%" PRIu64 ",%" PRIu64 ",1\n",
            p, n, k, operacoes, resultado);
#else
    uint64_t checagem;
    size_t b = 1, limite = 1048576;
    double dt;
    /* Aquece e aumenta o lote ate 5 ms, ou ate o limite. */
    do {
        dt = lote(p, v, n, b, &checagem);
        if (dt >= .005 || b == limite) break;
        b *= 2;
    } while (1);
    double estimado = dt > 0 ? .020*b/dt : (double)limite;
    b = estimado >= limite ? limite : (size_t)estimado + 1;
    for (int a = 1; a <= amostras; ++a) {
        dt = lote(p, v, n, b, &checagem);
        /* Os limites de entrada mantem a soma do lote em uint64_t. */
        if (checagem != esperado*b) erro("Resultado do lote incorreto");
        fprintf(f, "%d,%zu,%d,tempo,%d,%zu,%.12g,,%" PRIu64 ",1\n",
                p, n, k, a, b, dt/b, resultado);
    }
    (void)operacoes;
#endif
    if (ferror(f) || fclose(f)) erro("Falha ao gravar CSV");
    free(v);
    printf("P%d n=%zu resultado=%" PRIu64 " OK\n", p, n, resultado);
    return 0;
}
