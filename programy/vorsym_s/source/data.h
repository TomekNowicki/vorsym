#ifndef data_h
#define data_h

#include "../../vortex.h"

//zmienne globalne

#define STRINGS_CAPACITY 256

//nazwy plików z danymi i wynikowego
extern char inputFile[STRINGS_CAPACITY];
extern char outputFile[STRINGS_CAPACITY];

//ilosc wirow -rozmiar tablic: GA[], VEL[]
extern unsigned_int NV;

//ilosc krokow do wykonania
//(1+NS) x NV to rozmiar tablicy STEP
extern unsigned_int NS;

//natezenia wirow (tablica jednowymiarowa)
extern real *GA;

//predkosci (tablica jednowymiarowa)
extern struct point *VEL;

//kolejne polozenia wirow (tablica dwuwymiarowa)
extern struct point *STEP;

//lepkosc - jezeli wynosi 0 to krok nie jest wykonywany
extern real v;

//krok czasowy symulacji
extern real dT;

//wymiar kwadratowej domeny
extern real D;

#endif
