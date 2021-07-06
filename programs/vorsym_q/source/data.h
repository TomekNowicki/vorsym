#ifndef data_h
#define data_h

#include "../../vortex.h"

//zmienne globalne

#define STRINGS_CAPACITY 256

//nazwy plików z danymi i wynikowego
extern char inputFile[STRINGS_CAPACITY];
extern char outputFile[STRINGS_CAPACITY];

//iloœæ wirów -rozmiar tablic: GA, VEL
extern unsigned_int NV;

//iloœæ kroków do wykonania
//(1+NS) x NV to rozmiar tablicy STEP
extern unsigned_int NS;

//natê¿enia wirów (tablica jednowymiarowa)
extern real *GA;

//prêdkoœci (tablica jednowymiarowa)
extern struct point *VEL;

//kolejne polo¿enia wirów (tablica dwuwymiarowa)
extern struct point *STEP;

//lepkoœæ - je¿eli wynosi 0 to krok nie jest wykonywany
extern real v;

//krok czasowy symulacji
extern real dT;

//wymiar kwadratowej domeny
extern real D;

//
//struktury danych dodane go wersji quick
//

//podzia³ boku domeny; iloœæ subdomen to NSd x NSd
extern unsigned_int NSd;

//tablica subdomen
extern struct subdomain *SB;

//pocz¹tkowa pojemnoœæ/skok dla tablicy subdomenowej
extern unsigned_int NVs;


#endif
