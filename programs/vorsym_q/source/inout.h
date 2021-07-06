#ifndef inout_h
#define inout_h

#include "../../vortex.h"

//operacje plikowe, alokacja i zwalnianie pamieci

//sprawdza czy str1 ko�czy si� na str2 (sprawdzanie rozszerze� plik�w)
int strEnd(const char* str1, const char* str2);

//zmienia rozszerzenie pliku na ext - nale�y poda� ".ext" jako parametr
//wcze�niej u�yc strEnd, �eby sprwdzi� czy zmiana bedzie ok
int changeExt(char* pathName, const char* ext);

//alokuje pamiec dla nv wir�w i dla ns krok�w;
int takeMemory(unsigned_int nv, unsigned_int ns);

//zwalnia pamiec
int freeMemory(void);

//wczytuje parametry programu do struktur danych
int readParameters(const int* argc, char* const argv[]);

//wczytuje pocz�tkowe po�o�enie wir�w do wewn�trznych struktur danych
//format pliku wej�ciowego rozpoznaje po rozszrzeniu *.vrt lub *.vrb
int readData(const char *pathName);

//czyta plik tekstowy z po�o�eniami i mocami wir�w
int readVrt(const char *pathName);

//czyta plik binarny do kontynuacji symulacji
int readVrb(const char *pathName);

//zapisuje wszystkie kroki i wiry do pliku .vrb
int writeVrb(const char *pathName); 

//zapisuje do pliku tekstowego wartosci odczytane z wewn�trznych struktur
int writeReport(const char *pathName);

//
//funkcje dodane do wersji quick
//

//rezerwacja pami�ci na subdomeny
int takeMemoryForSubdomains(void);

//zwolnienie pamieci subdomen
int freeSubdomainsMemory(void);

//dodaje indeks wiru i do subdomeny
int addVortex(struct subdomain* S, unsigned_int i);

//zapisuje raport z subdomen
int writeReport2(const char *pathName);


#endif
