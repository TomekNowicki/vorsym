#ifndef data_h
#define data_h

#include "../../vortex.h"

//zmienne globalne

#define STRINGS_CAPACITY 256

//nazwy plik�w z danymi i wynikowego
extern char inputFile[STRINGS_CAPACITY];
extern char outputFile[STRINGS_CAPACITY];

//ilo�� wir�w -rozmiar tablic: GA, VEL
extern unsigned_int NV;

//ilo�� krok�w do wykonania
//(1+NS) x NV to rozmiar tablicy STEP
extern unsigned_int NS;

//nat�enia wir�w (tablica jednowymiarowa)
extern real *GA;

//pr�dko�ci (tablica jednowymiarowa)
extern struct point *VEL;

//kolejne polo�enia wir�w (tablica dwuwymiarowa)
extern struct point *STEP;

//lepko�� - je�eli wynosi 0 to krok nie jest wykonywany
extern real v;

//krok czasowy symulacji
extern real dT;

//wymiar kwadratowej domeny
extern real D;

//
//struktury danych dodane go wersji quick
//

//podzia� boku domeny; ilo�� subdomen to NSd x NSd
extern unsigned_int NSd;

//tablica subdomen
extern struct subdomain *SB;

//pocz�tkowa pojemno��/skok dla tablicy subdomenowej
extern unsigned_int NVs;


#endif
