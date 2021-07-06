#ifndef inout_h
#define inout_h

#include "../../vortex.h"

//operacje plikowe, alokacja i zwalnianie pamieci

//sprawdza czy str1 koñczy siê na str2 (sprawdzanie rozszerzeñ plików)
int strEnd(const char* str1, const char* str2);

//zmienia rozszerzenie pliku na ext - nale¿y podaæ ".ext" jako parametr
//wczeœniej u¿yc strEnd, ¿eby sprwdziæ czy zmiana bedzie ok
int changeExt(char* pathName, const char* ext);

//alokuje pamiec dla nv wirów i dla ns kroków;
int takeMemory(unsigned_int nv, unsigned_int ns);

//zwalnia pamiec
int freeMemory(void);

//wczytuje parametry programu do struktur danych
int readParameters(const int* argc, char* const argv[]);

//wczytuje pocz¹tkowe po³o¿enie wirów do wewnêtrznych struktur danych
//format pliku wejœciowego rozpoznaje po rozszrzeniu *.vrt lub *.vrb
int readData(const char *pathName);

//czyta plik tekstowy z po³o¿eniami i mocami wirów
int readVrt(const char *pathName);

//czyta plik binarny do kontynuacji symulacji
int readVrb(const char *pathName);

//zapisuje wszystkie kroki i wiry do pliku .vrb
int writeVrb(const char *pathName); 

//zapisuje do pliku tekstowego wartosci odczytane z wewnêtrznych struktur
int writeReport(const char *pathName);

//
//funkcje dodane do wersji quick
//

//rezerwacja pamiêci na subdomeny
int takeMemoryForSubdomains(void);

//zwolnienie pamieci subdomen
int freeSubdomainsMemory(void);

//dodaje indeks wiru i do subdomeny
int addVortex(struct subdomain* S, unsigned_int i);

//zapisuje raport z subdomen
int writeReport2(const char *pathName);


#endif
