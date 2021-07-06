#ifndef data_h
#define data_h

#include "../../vortex.h"

// terminologia dekompozycji:
// DOMENA_SYMULACJI SIMULATION_DOMAIN
// DOMENA_PROCESU PROCESS_DOMAIN
// SUBDOMENA SUBDOMAIN

//wszystkie punkty sa podawane we wspolrzednych DOMENY_SYMULACJI

//przesylany wir
struct vortex_trans{
	//
	unsigned_int i;//pozycja wiru
	real x, y;//pozycja wiru
};

//wezel wskazujacy na tablice VfP[]
struct v_p_handle{
	unsigned_int cap, last;
	struct vortex_trans *V;
};

#define VORTEXTRANSSIZE sizeof(struct vortex_trans)
#define VPHANDLESIZE sizeof(struct v_p_handle)

#define STRINGS_CAPACITY 256

//nazwy plików z danymi i wynikowego
extern char inputFile[STRINGS_CAPACITY];
extern char outputFile[STRINGS_CAPACITY];

//ilosc wirów w CALEJ_DOMENIE -rozmiar tablic: GA[], VEL[]
//!UWAGA: zmienna bierze udzial w wymianie miedzy procesami
extern unsigned_int NV;

//ilosc kroków do wykonania - (1+NS) x NV to rozmiar tablicy STEP
//!UWAGA: zmienna bierze udzial w wymianie miedzy procesami
extern unsigned_int NS;

//wymiar DOMENY_SYMULACJI
//!UWAGA: zmienna bierze udzial w wymianie miedzy procesami
extern real D;

//lepkosc - jezeli wynosi 0 to krok dyfuzyjny nie jest wykonywany
//!UWAGA: zmienna bierze udzial w wymianie miedzy procesami
extern real v;

//krok czasowy symulacji
//!UWAGA: zmienna bierze udzial w wymianie miedzy procesami
extern real dT;

//ilosc procesow
//!UWAGA: zmienna bierze udzial w wymianie miedzy procesami
extern unsigned_int NP;

//pozycja procesu w wirtualnej topologii
extern unsigned_int rowP, colP;

//rozmiar domeny dla procesu
extern real Dp;

//srodkowy punkt DOMENY_PROCESU
extern struct point O;

//podzial boku domeny; ilosc subdomen to NSd x NSd
//!UWAGA: zmienna bierze udzial w wymianie miedzy procesami
extern unsigned_int NSd;

//tablica subdomen jednego procesu
extern struct subdomain *SB;

//poczatkowa pojemnosc/skok dla tablicy subdomenowej SB[].GAs[]
//!UWAGA: zmienna bierze udzial w wymianie miedzy procesami
extern unsigned_int NVs;

//tablica egzystencji danego wiru w procesie (tablica dwuwymiarowa)
extern int* EX;

//natezenia wirów (tablica jednowymiarowa)
extern real *GA;

//predkosci (tablica jednowymiarowa)
extern struct point *VEL;

//kolejne polozenia wirów (tablica dwuwymiarowa)
extern struct point *STEP;

//tablica momentow subdomenowych dla calego zadania - obszar ciagly
//!UWAGA: zmienna bierze udzial w wymianie miedzy procesami
extern real *MS;

//srodki subdomen DOMEMENY_SYMULACJI
extern struct point *OS;

//tablica ktora mowi procesowi ile WYSLE innym procesom 
//!UWAGA: zmienna bierze udzial w wymianie miedzy procesami
extern unsigned_int *NVfP;

//tablica ktora mowi procesowi ile wirow otrzyma od innych procesow
//!UWAGA: zmienna bierze udzial w wymianie miedzy procesami
extern unsigned_int *NVtP;

//tablica wskaznikow do tablic na wiry DO WYSLANIA 'f' - from
//!UWAGA: zmienna bierze udzial w wymianie miedzy procesami
extern struct v_p_handle *VfP;

//tablica wskaznikow do tablic na wiry DO ODEBRANIA 't' - to
//!UWAGA: zmienna bierze udzial w wymianie miedzy procesami
extern struct v_p_handle *VtP;

//poczatkowa pojemnosc/skok dla tablicy VfP[] oraz VtP[]
//!UWAGA: zmienna bierze udzial w wymianie miedzy procesami
extern unsigned_int V_Ps;

//
//funkcje sluzace do obslugi struktur danych

//
inline struct point* OS_get(unsigned_int r_proc, unsigned_int c_proc,
					 unsigned_int r_sub, unsigned_int c_sub);

//
inline real* MS_getStart(unsigned_int r_proc, unsigned_int c_proc,
					unsigned_int r_sub, unsigned_int c_sub, unsigned_int i); 
					
//
inline real MS_getSum(unsigned_int r_proc, unsigned_int c_proc,
					  unsigned_int r_sub, unsigned_int c_sub);					

//
inline void MS_getMoments(unsigned_int r_proc, unsigned_int c_proc,
					unsigned_int r_sub, unsigned_int c_sub, struct point* Z);
					
//przepisanie momentow z SB[] do MS[]
void updateSubdomainsMoments(void);

#endif
