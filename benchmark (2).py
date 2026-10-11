# =====================================================================
#  Benchmarking de Complejidad Asintotica: Merge Sort vs Quick Sort
#  Estructura: Lista Doblemente Enlazada (adaptado de la Practica 05)
#  Autor: Condori Castillo Anderson Alexis
#  Ejecutar:  python benchmark.py
# =====================================================================
import gc
import math
import time

comparaciones = 0


class Nodo:
    __slots__ = ("dato", "siguiente", "anterior")

    def __init__(self, dato):
        self.dato = dato
        self.siguiente = None
        self.anterior = None


class ListaDoble:
    def __init__(self):
        self.inicio = self.fin = None
        self.n = 0

    def agregar_final(self, d):
        nuevo = Nodo(d)
        nuevo.anterior = self.fin
        if self.fin: self.fin.siguiente = nuevo
        else: self.inicio = nuevo
        self.fin = nuevo
        self.n += 1

    def ordenada(self):
        p = self.inicio
        while p and p.siguiente:
            if p.dato > p.siguiente.dato or p.siguiente.anterior is not p:
                return False
            p = p.siguiente
        return True

    # ------------------------- MERGE SORT -------------------------
    @staticmethod
    def _dividir(cabeza):                       # fast / slow pointer
        rapido = lento = cabeza
        while rapido.siguiente and rapido.siguiente.siguiente:
            rapido = rapido.siguiente.siguiente
            lento = lento.siguiente
        mitad = lento.siguiente
        lento.siguiente = None
        mitad.anterior = None
        return mitad

    @staticmethod
    def _merge(a, b):                           # fusion iterativa
        global comparaciones
        cab = Nodo(0)
        t = cab
        while a and b:
            comparaciones += 1
            if a.dato <= b.dato:
                t.siguiente = a; a.anterior = t; a = a.siguiente
            else:
                t.siguiente = b; b.anterior = t; b = b.siguiente
            t = t.siguiente
        resto = a if a else b
        t.siguiente = resto
        if resto: resto.anterior = t
        r = cab.siguiente
        if r: r.anterior = None
        return r

    def _merge_sort_rec(self, h):
        if not h or not h.siguiente:
            return h
        mitad = self._dividir(h)
        return self._merge(self._merge_sort_rec(h), self._merge_sort_rec(mitad))

    def merge_sort(self):
        self.inicio = self._merge_sort_rec(self.inicio)
        f = self.inicio
        while f and f.siguiente: f = f.siguiente
        self.fin = f

    # ------------------------- QUICK SORT -------------------------
    @staticmethod
    def _particion(bajo, alto):                 # Lomuto, pivote = ultimo
        global comparaciones
        pivote = alto.dato
        i, k, j = None, 0, bajo
        while j is not alto:
            comparaciones += 1
            if j.dato <= pivote:
                i = bajo if i is None else i.siguiente
                i.dato, j.dato = j.dato, i.dato
                k += 1
            j = j.siguiente
        i = bajo if i is None else i.siguiente
        i.dato, alto.dato = alto.dato, i.dato
        return i, k

    def _quick_sort_rec(self, bajo, alto, n):
        while n > 1:                            # recursion en la parte menor -> pila O(log n)
            p, k = self._particion(bajo, alto)
            izq, der = k, n - k - 1
            if izq < der:
                self._quick_sort_rec(bajo, p.anterior, izq); bajo, n = p.siguiente, der
            else:
                self._quick_sort_rec(p.siguiente, alto, der); alto, n = p.anterior, izq

    def quick_sort(self):
        self._quick_sort_rec(self.inicio, self.fin, self.n)


def generar(n, caso):
    """Generador LCG identico en C, C++ y Python -> mismos datos en los 3 lenguajes."""
    x, v = 12345 + n, []
    for _ in range(n):
        x = (x * 1103515245 + 12345) & 0x7FFFFFFF
        v.append((x >> 8) % 1000000)
    if caso == "ordenado": v.sort()
    if caso == "inverso": v.sort(reverse=True)
    return v


def medir(v, alg, reps=3):
    global comparaciones
    tiempos, ok = [], True
    for _ in range(reps):
        L = ListaDoble()
        for d in v: L.agregar_final(d)
        comparaciones = 0
        gc.disable()
        a = time.perf_counter()
        L.merge_sort() if alg == 0 else L.quick_sort()
        b = time.perf_counter()
        gc.enable()
        tiempos.append((b - a) * 1000)
        ok = ok and L.ordenada()
    tiempos.sort()
    return tiempos[len(tiempos) // 2], comparaciones, ok        # mediana


def main():
    ALG = ["MergeSort", "QuickSort"]
    ns_aleat = [1000 * 2 ** k for k in range(8)]     # 1 000 .. 128 000
    ns_peor = [500 * 2 ** k for k in range(4)]       #   500 ..   4 000
    bloques = [("aleatorio", ns_aleat), ("ordenado", ns_peor), ("inverso", ns_peor)]

    print("=" * 66)
    print("  BENCHMARK PYTHON | Merge Sort vs Quick Sort | Lista Doble Enlazada")
    print("=" * 66)
    with open("resultados_py.csv", "w") as csv:
        csv.write("lenguaje,algoritmo,caso,n,tiempo_ms,comparaciones\n")
        for caso, ns in bloques:
            for a in range(2):
                print(f"\n[{ALG[a]} - {caso}]")
                print(f"{'n':>9} {'t (ms)':>12} {'comparac.':>14} {'C/nlog2n':>10} {'T2n/Tn':>10} {'OK':>4}")
                prev = None
                for n in ns:
                    ms, comp, ok = medir(generar(n, caso), a)
                    nlogn = n * math.log2(n)
                    razon = f"{ms / prev:10.2f}" if prev else f"{'-':>10}"
                    print(f"{n:>9} {ms:>12.3f} {comp:>14} {comp / nlogn:>10.3f} {razon} {'si' if ok else 'NO':>4}", flush=True)
                    csv.write(f"Python,{ALG[a]},{caso},{n},{ms:.6f},{comp}\n")
                    prev = ms
    print("\nResultados guardados en: resultados_py.csv")


if __name__ == "__main__":
    main()
