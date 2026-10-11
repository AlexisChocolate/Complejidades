/* =====================================================================
 *  Benchmarking de Complejidad Asintotica: Merge Sort vs Quick Sort
 *  Estructura: Lista Doblemente Enlazada (adaptado de la Practica 05)
 *  Autor: Condori Castillo Anderson Alexis
 *  Compilar:  gcc -O2 benchmark.c -o benchmark_c -lm
 *  Ejecutar:  ./benchmark_c        (Windows: .\benchmark_c.exe)
 * ===================================================================== */
#define _POSIX_C_SOURCE 199309L
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>

#ifdef _WIN32
#include <windows.h>
static double ahora_ms(void) {
    static LARGE_INTEGER f; LARGE_INTEGER c;
    if (!f.QuadPart) QueryPerformanceFrequency(&f);
    QueryPerformanceCounter(&c);
    return (double)c.QuadPart * 1000.0 / (double)f.QuadPart;
}
#else
#include <time.h>
static double ahora_ms(void) {
    struct timespec t; clock_gettime(CLOCK_MONOTONIC, &t);
    return t.tv_sec * 1e3 + t.tv_nsec / 1e6;
}
#endif

typedef struct Nodo { int dato; struct Nodo *siguiente, *anterior; } Nodo;
typedef struct { Nodo *inicio, *fin; long n; } ListaDoble;

static long long comparaciones = 0;

static void agregarFinal(ListaDoble* L, int d) {
    Nodo* nuevo = (Nodo*)malloc(sizeof(Nodo));
    nuevo->dato = d; nuevo->siguiente = NULL; nuevo->anterior = L->fin;
    if (L->fin) L->fin->siguiente = nuevo; else L->inicio = nuevo;
    L->fin = nuevo; L->n++;
}
static void liberar(ListaDoble* L) {
    while (L->inicio) { Nodo* t = L->inicio; L->inicio = t->siguiente; free(t); }
    L->fin = NULL; L->n = 0;
}
static int ordenada(const ListaDoble* L) {
    for (Nodo* p = L->inicio; p && p->siguiente; p = p->siguiente)
        if (p->dato > p->siguiente->dato || p->siguiente->anterior != p) return 0;
    return 1;
}

/* ------------------------- MERGE SORT ------------------------- */
static Nodo* dividir(Nodo* cabeza) {                 /* fast / slow pointer */
    Nodo *rapido = cabeza, *lento = cabeza;
    while (rapido->siguiente && rapido->siguiente->siguiente) {
        rapido = rapido->siguiente->siguiente; lento = lento->siguiente;
    }
    Nodo* mitad = lento->siguiente;
    lento->siguiente = NULL; mitad->anterior = NULL;
    return mitad;
}
static Nodo* merge(Nodo* a, Nodo* b) {               /* fusion iterativa */
    Nodo cab = {0, NULL, NULL}; Nodo* t = &cab;
    while (a && b) {
        comparaciones++;
        if (a->dato <= b->dato) { t->siguiente = a; a->anterior = t; a = a->siguiente; }
        else                    { t->siguiente = b; b->anterior = t; b = b->siguiente; }
        t = t->siguiente;
    }
    Nodo* resto = a ? a : b;
    t->siguiente = resto; if (resto) resto->anterior = t;
    Nodo* r = cab.siguiente; if (r) r->anterior = NULL;
    return r;
}
static Nodo* mergeSortRec(Nodo* h) {
    if (!h || !h->siguiente) return h;
    Nodo* mitad = dividir(h);
    return merge(mergeSortRec(h), mergeSortRec(mitad));
}
static void mergeSort(ListaDoble* L) {
    L->inicio = mergeSortRec(L->inicio);
    L->fin = L->inicio; while (L->fin && L->fin->siguiente) L->fin = L->fin->siguiente;
}

/* ------------------------- QUICK SORT ------------------------- */
static void intercambiar(Nodo* a, Nodo* b) { int t = a->dato; a->dato = b->dato; b->dato = t; }
static Nodo* particion(Nodo* bajo, Nodo* alto, long* k) {      /* Lomuto, pivote = ultimo */
    int pivote = alto->dato; Nodo* i = NULL; *k = 0;
    for (Nodo* j = bajo; j != alto; j = j->siguiente) {
        comparaciones++;
        if (j->dato <= pivote) { i = i ? i->siguiente : bajo; intercambiar(i, j); (*k)++; }
    }
    i = i ? i->siguiente : bajo; intercambiar(i, alto);
    return i;
}
static void quickSortRec(Nodo* bajo, Nodo* alto, long n) {
    while (n > 1) {                    /* recursion en la parte menor -> pila O(log n) */
        long k; Nodo* p = particion(bajo, alto, &k);
        long izq = k, der = n - k - 1;
        if (izq < der) { quickSortRec(bajo, p->anterior, izq); bajo = p->siguiente; n = der; }
        else           { quickSortRec(p->siguiente, alto, der); alto = p->anterior; n = izq; }
    }
}
static void quickSort(ListaDoble* L) { quickSortRec(L->inicio, L->fin, L->n); }

/* ---------------------- GENERACION DE DATOS ---------------------- */
static int cmpAsc(const void* a, const void* b)  { return (*(const int*)a > *(const int*)b) - (*(const int*)a < *(const int*)b); }
static int cmpDesc(const void* a, const void* b) { return cmpAsc(b, a); }
static int cmpDbl(const void* a, const void* b)  { double x = *(const double*)a, y = *(const double*)b; return (x > y) - (x < y); }

/* Generador LCG identico en C, C++ y Python -> mismos datos en los 3 lenguajes */
static int* generar(long n, const char* caso) {
    int* v = (int*)malloc(n * sizeof(int)); uint64_t x = 12345 + (uint64_t)n;
    for (long i = 0; i < n; i++) {
        x = (x * 1103515245ULL + 12345ULL) & 0x7FFFFFFFULL;
        v[i] = (int)((x >> 8) % 1000000);
    }
    if (!strcmp(caso, "ordenado")) qsort(v, n, sizeof(int), cmpAsc);
    if (!strcmp(caso, "inverso"))  qsort(v, n, sizeof(int), cmpDesc);
    return v;
}

static double medir(const int* v, long n, int alg, long long* comp, int* ok) {
    int reps = n <= 100000 ? 5 : 3; double t[5]; *ok = 1;
    for (int r = 0; r < reps; r++) {
        ListaDoble L = {NULL, NULL, 0};
        for (long i = 0; i < n; i++) agregarFinal(&L, v[i]);
        comparaciones = 0;
        double a = ahora_ms();
        if (alg == 0) mergeSort(&L); else quickSort(&L);
        t[r] = ahora_ms() - a;
        *comp = comparaciones; *ok = *ok && ordenada(&L);
        liberar(&L);
    }
    qsort(t, reps, sizeof(double), cmpDbl);
    return t[reps / 2];                                /* mediana */
}

int main(void) {
    const char* ALG[2]   = {"MergeSort", "QuickSort"};
    const char* CASOS[3] = {"aleatorio", "ordenado", "inverso"};
    FILE* csv = fopen("resultados_c.csv", "w");
    fprintf(csv, "lenguaje,algoritmo,caso,n,tiempo_ms,comparaciones\n");

    printf("==================================================================\n");
    printf("  BENCHMARK C   | Merge Sort vs Quick Sort | Lista Doble Enlazada\n");
    printf("==================================================================\n");
    for (int c = 0; c < 3; c++)
        for (int a = 0; a < 2; a++) {
            long nIni = c == 0 ? 1000 : 500, nMax = c == 0 ? 1024000 : 32000;
            printf("\n[%s - %s]\n", ALG[a], CASOS[c]);
            printf("%9s %12s %14s %10s %10s %4s\n", "n", "t (ms)", "comparac.", "C/nlog2n", "T2n/Tn", "OK");
            double prev = -1;
            for (long n = nIni; n <= nMax; n *= 2) {
                int* v = generar(n, CASOS[c]); long long comp; int ok;
                double ms = medir(v, n, a, &comp, &ok); free(v);
                double nlogn = n * log2((double)n);
                if (prev > 0) printf("%9ld %12.3f %14.0f %10.3f %10.2f %4s\n", n, ms, (double)comp, comp / nlogn, ms / prev, ok ? "si" : "NO");
                else          printf("%9ld %12.3f %14.0f %10.3f %10s %4s\n",   n, ms, (double)comp, comp / nlogn, "-", ok ? "si" : "NO");
                fprintf(csv, "C,%s,%s,%ld,%.6f,%.0f\n", ALG[a], CASOS[c], n, ms, (double)comp);
                prev = ms;
            }
        }
    fclose(csv);
    printf("\nResultados guardados en: resultados_c.csv\n");
    return 0;
}
