// =====================================================================
//  Benchmarking de Complejidad Asintotica: Merge Sort vs Quick Sort
//  Estructura: Lista Doblemente Enlazada (adaptado de la Practica 05)
//  Autor: Condori Castillo Anderson Alexis
//  Compilar:  g++ -O2 -std=c++17 benchmark.cpp -o benchmark_cpp
//  Ejecutar:  ./benchmark_cpp      (Windows: .\benchmark_cpp.exe)
// =====================================================================
#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <string>
using namespace std;

struct Nodo { int dato; Nodo* siguiente; Nodo* anterior; };

static long long comparaciones = 0;   // contador independiente de la maquina

class ListaDoble {
public:
    Nodo* inicio = nullptr; Nodo* fin = nullptr; long n = 0;

    ~ListaDoble() { while (inicio) { Nodo* t = inicio; inicio = inicio->siguiente; delete t; } }

    void agregarFinal(int d) {
        Nodo* nuevo = new Nodo{d, nullptr, fin};
        if (fin) fin->siguiente = nuevo; else inicio = nuevo;
        fin = nuevo; n++;
    }
    bool ordenada() const {   // verifica orden y enlaces "anterior"
        for (Nodo* p = inicio; p && p->siguiente; p = p->siguiente)
            if (p->dato > p->siguiente->dato || p->siguiente->anterior != p) return false;
        return true;
    }

    // ------------------------- MERGE SORT -------------------------
    void mergeSort() {
        inicio = mergeSortRec(inicio);
        fin = inicio; while (fin && fin->siguiente) fin = fin->siguiente;
    }
    // ------------------------- QUICK SORT -------------------------
    void quickSort() { quickSortRec(inicio, fin, n); }

private:
    static Nodo* dividir(Nodo* cabeza) {               // fast / slow pointer
        Nodo *rapido = cabeza, *lento = cabeza;
        while (rapido->siguiente && rapido->siguiente->siguiente) {
            rapido = rapido->siguiente->siguiente; lento = lento->siguiente;
        }
        Nodo* mitad = lento->siguiente;
        lento->siguiente = nullptr; mitad->anterior = nullptr;
        return mitad;
    }
    static Nodo* merge(Nodo* a, Nodo* b) {             // fusion iterativa
        Nodo cab{0, nullptr, nullptr}; Nodo* t = &cab;
        while (a && b) {
            comparaciones++;
            if (a->dato <= b->dato) { t->siguiente = a; a->anterior = t; a = a->siguiente; }
            else                    { t->siguiente = b; b->anterior = t; b = b->siguiente; }
            t = t->siguiente;
        }
        Nodo* resto = a ? a : b;
        t->siguiente = resto; if (resto) resto->anterior = t;
        Nodo* r = cab.siguiente; if (r) r->anterior = nullptr;
        return r;
    }
    static Nodo* mergeSortRec(Nodo* h) {
        if (!h || !h->siguiente) return h;
        Nodo* mitad = dividir(h);
        return merge(mergeSortRec(h), mergeSortRec(mitad));
    }
    static Nodo* particion(Nodo* bajo, Nodo* alto, long& k) {   // Lomuto, pivote = ultimo
        int pivote = alto->dato; Nodo* i = nullptr; k = 0;
        for (Nodo* j = bajo; j != alto; j = j->siguiente) {
            comparaciones++;
            if (j->dato <= pivote) { i = i ? i->siguiente : bajo; swap(i->dato, j->dato); k++; }
        }
        i = i ? i->siguiente : bajo; swap(i->dato, alto->dato);
        return i;
    }
    static void quickSortRec(Nodo* bajo, Nodo* alto, long n) {
        while (n > 1) {                 // recursion en la parte menor -> pila O(log n)
            long k; Nodo* p = particion(bajo, alto, k);
            long izq = k, der = n - k - 1;
            if (izq < der) { quickSortRec(bajo, p->anterior, izq); bajo = p->siguiente; n = der; }
            else           { quickSortRec(p->siguiente, alto, der); alto = p->anterior; n = izq; }
        }
    }
};

// Generador LCG identico en C, C++ y Python -> mismos datos en los 3 lenguajes
static vector<int> generar(long n, const string& caso) {
    vector<int> v(n); uint64_t x = 12345 + (uint64_t)n;
    for (long i = 0; i < n; i++) {
        x = (x * 1103515245ULL + 12345ULL) & 0x7FFFFFFFULL;
        v[i] = (int)((x >> 8) % 1000000);
    }
    if (caso == "ordenado") sort(v.begin(), v.end());
    if (caso == "inverso")  sort(v.rbegin(), v.rend());
    return v;
}

struct Res { double ms; long long comp; bool ok; };

static Res medir(const vector<int>& v, int alg) {
    int reps = v.size() <= 100000 ? 5 : 3;          // mediana de varias corridas
    vector<double> t; long long comp = 0; bool ok = true;
    for (int r = 0; r < reps; r++) {
        ListaDoble L; for (int d : v) L.agregarFinal(d);
        comparaciones = 0;
        auto a = chrono::steady_clock::now();
        if (alg == 0) L.mergeSort(); else L.quickSort();
        auto b = chrono::steady_clock::now();
        t.push_back(chrono::duration<double, milli>(b - a).count());
        comp = comparaciones; ok = ok && L.ordenada();
    }
    sort(t.begin(), t.end());
    return {t[t.size() / 2], comp, ok};
}

int main() {
    const char* ALG[2] = {"MergeSort", "QuickSort"};
    vector<long> nsAleat, nsPeor;
    for (long n = 1000; n <= 1024000; n *= 2) nsAleat.push_back(n);
    for (long n = 500;  n <= 32000;   n *= 2) nsPeor.push_back(n);
    struct Bloque { string caso; vector<long> ns; };
    vector<Bloque> bloques = {{"aleatorio", nsAleat}, {"ordenado", nsPeor}, {"inverso", nsPeor}};

    ofstream csv("resultados_cpp.csv");
    csv << "lenguaje,algoritmo,caso,n,tiempo_ms,comparaciones\n";

    printf("==================================================================\n");
    printf("  BENCHMARK C++ | Merge Sort vs Quick Sort | Lista Doble Enlazada\n");
    printf("==================================================================\n");
    for (auto& B : bloques)
        for (int a = 0; a < 2; a++) {
            printf("\n[%s - %s]\n", ALG[a], B.caso.c_str());
            printf("%9s %12s %14s %10s %10s %4s\n", "n", "t (ms)", "comparac.", "C/nlog2n", "T2n/Tn", "OK");
            double prev = -1;
            for (long n : B.ns) {
                Res r = medir(generar(n, B.caso), a);
                double nlogn = n * log2((double)n);
                if (prev > 0) printf("%9ld %12.3f %14lld %10.3f %10.2f %4s\n", n, r.ms, r.comp, r.comp / nlogn, r.ms / prev, r.ok ? "si" : "NO");
                else          printf("%9ld %12.3f %14lld %10.3f %10s %4s\n",   n, r.ms, r.comp, r.comp / nlogn, "-", r.ok ? "si" : "NO");
                csv << "C++," << ALG[a] << "," << B.caso << "," << n << "," << r.ms << "," << r.comp << "\n";
                prev = r.ms;
            }
        }
    printf("\nResultados guardados en: resultados_cpp.csv\n");
    return 0;
}
