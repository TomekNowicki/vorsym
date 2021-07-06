#ifndef inout_h
#define inout_h

#include "../../vortex.h"
#include "data.h"

//operacje plikowe, alokacja i zwalnianie pamieci

//sprawdza czy str1 konczy sie na str2 (sprawdzanie rozszerzeñ plików)
int strEnd(const char* str1, const char* str2);

//zmienia rozszerzenie pliku na ext - nalezy podac ".ext" jako parametr
//wczesniej uzyc strEnd, zeby sprwdzic czy zmiana bedzie ok
int changeExt(char* pathName, const char* ext);

//alokuje pamiec dla nv wirów i dla ns kroków;
int takeMemory_0(unsigned_int nv, unsigned_int ns);

//zwalnia pamiec
int freeMemory_0(void);

//wczytuje parametry programu do struktur danych
int readParameters(const char* pathName);

//czyta plik tekstowy z polozeniami i mocami wirów
int readVrt(const char *pathName);

//zapisuje wszystkie kroki i wiry do pliku .vrb
//int writeVrb(const char *pathName); 

//zapisuje do pliku tekstowego wartosci odczytane z wewnetrznych struktur
int writeReport(const char *pathName, unsigned_int step);


//rezerwacja pamieci na subdomeny
int takeMemory_1(unsigned_int nsd, unsigned_int nvs, unsigned_int np);

//zwolnienie pamieci subdomen
int freeMemory_1(unsigned_int np, unsigned_int nsd);

//dodaje indeks wiru i do subdomeny
int addVortexToSubdomain(struct subdomain* S, unsigned_int i);


inline int increaseV_P(struct v_p_handle* h);
int addVortexToV_P(struct v_p_handle* h, unsigned_int i, struct point P);


//zapisuje raport z subdomen
//int writeReport2(const char *pathName);


//ustalenie ilosci wirow
int firstReadVrt(const char *pathName);

int createVrb(const char *pathName);

//...
unsigned_int countVortexesInProces(unsigned_int step);


#endif
