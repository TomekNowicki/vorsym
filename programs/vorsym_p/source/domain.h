#ifndef domain_h
#define domain_h

#include "../../vortex.h"

//stwierdza czy punkt P znajduje sie wewnatrz kwadratu o boku d
//i srodku w O
inline int insideSquare(real d, struct point O, struct point P);

//zwraca wspolrzedne srodka subkwadratu
inline struct point subSquareCenter(real d, struct point center,
                       unsigned_int n, unsigned_int r, unsigned_int c);

//...
inline int whichSubSquare(real d, struct point O, unsigned_int n,
					 struct point P, unsigned_int *r, unsigned_int *c);

//zwraca wspolrzedne srodka DOMENY_SYMULACJI
inline struct point simulationCenter(void);

//zwraca wspolrzedne srodka DOMENY_PROCESU
inline struct point procesCenter(real d_sym, unsigned_int np,
							 unsigned_int r_proc, unsigned_int c_proc);


//zwraca wspolrzedne srodka SUBDOMENY
struct point subdomainCenter(real d_sym, unsigned_int np, unsigned_int nsd,
							 unsigned_int r_proc, unsigned_int c_proc,
							 unsigned_int r_sub, unsigned_int c_sub);

//zwraca 1 jezeli punkt P znajduje sie w DOMENIE_SYMULACJI
inline int insideSimulationDomain(struct point P);

//...
int whichProcess(struct point P, unsigned_int *r, unsigned_int *c);

//zwraca 1 jezeli punkt P znajduje sie w DOMENIE_PROCESU (tego)
inline int insideProcesDomain(struct point P);

//znajduje subdomene dla podanego punktu
struct subdomain* whichSubdomain(struct point P);

//mowi czy punkt jest w danej subdomenie
inline int isPointInSubdomain(struct subdomain* sub, struct point P);

//ustawia parametry DOMENY_PROCESU
int initiateProcesDomain(int r, int c);

//wstawia indeksy wirów do odpowiednich subdomen
int initiateSubdomains(void);

//oblicza momenty wirowe dla subdomeny *sub
void calcMomentsInSubdomain(struct subdomain* sub, unsigned_int n);

//oblicza predkosci konwekcji dla polozen wirow w i-tym wierszu STEP[]
void calcVelocities(unsigned_int i);

//oblicza predkosci konwekcji dla polozen wirow w i-tym wierszu STEP
//w subdomene *sub
void calcVelInSubdomain_inside(struct subdomain* sub, unsigned_int i);
void calcVelInSubdomain_outside_local(struct subdomain* sub, unsigned_int i);     
void calcVelInSubdomain_outside_global(struct subdomain* sub, unsigned_int i);

//realizuje zawijanie sie DOMENY_SYMULACJI
struct point newPosition(const struct point P0, const struct point dP);

//konwekcja wirow z wiersza i do i+1 w STEP[]
void convectVortexes(unsigned_int i);

//konwekcja z wiersza i do i+1 w STEP w subdomenie *sub
void convectVortexexInSubdomain(const struct subdomain* const sub, unsigned_int i);


//dyfuzja z wiersza i do i+1 w STEP
void diffuseVortexes(unsigned_int i);

//dyfuzja z wiersza i do i+1 w STEP w subdomenie *sub
void diffuseVortexesInSubdomain(struct subdomain* sub, unsigned_int i);

//...
int transferVortexes(unsigned_int i);

//...
int transferVortexesInSubdomain(struct subdomain* sub, unsigned_int i);

//równomierny generator liczb losowych z przedzialu od 0 do 1
real rand_0_1(void);

//funkcje matematyczne pracujace na typie real
inline real powr(real x, real y);
inline real sqrr(real x);
inline real sqrtr(real x);
inline real modfr(real x, real* y);
inline real sinr(real x);
inline real cosr(real x);
inline real logr(real x);
inline real floorr(real x);

//funkcje zmiennej zespolonej
inline struct point multiComplex(struct point z1, struct point z2);
inline struct point divComplex(struct point z1, struct point z2);

#endif
