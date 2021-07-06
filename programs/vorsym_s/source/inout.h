#ifndef inout_h
#define inout_h

#include "../../vortex.h"

//operacje plikowe, alokacja i zwalnianie pamieci

//sprawdza czy str1 koñczy sie na str2 (sprawdzanie rozszerzeñ plików)
int strEnd(const char* str1, const char* str2);

//zmienia rozszerzenie pliku na ext - nale¿y podaæ ".ext" jako parametr
//wczesniej uzyc strEnd, zeby sprwdzic czy zmiana bedzie ok
int changeExt(char* pathName, const char* ext);

//alokuje pamiec dla nv wirów i dla ns kroków;
int takeMemory(unsigned_int nv, unsigned_int ns);

//zwalnia pamiec
int freeMemory(void);

//wczytuje parametry programu do struktur danych
int readParameters(const int* argc, char* const argv[]);

//wczytuje poczatkowe polozenie wirow do wewnetrznych struktur danych
//format pliku wejsciowego rozpoznaje po rozszrzeniu *.vrt lub *.vrb
int readData(const char *pathName);

//czyta plik tekstowy z po³o¿eniami i mocami wirów
int readVrt(const char *pathName);

//czyta plik binarny do kontynuacji symulacji
int readVrb(const char *pathName);

//zapisuje wszystkie kroki i wiry do pliku .vrb
int writeVrb(const char *pathName); 

//zapisuje do pliku tekstowego wartosci odczytane z wewnetrznych struktur
int writeReport(const char *pathName);

#endif
