#ifndef domain_h
#define domain_h

#include "../../vortex.h"

//propagacja wir�w w kwadratowej domenie - pocz�tek uk�adu odniesienia
//w �rodku kwadratu


//sprawdza czy punkt znajduje si� wewn�trz domeny
inline int insideDomain(const struct point P);

//oblicza pr�dko�ci konwekcji dla po�o�e� wir�w w i-tym wierszu STEP
void calcVelocities(unsigned_int i);

//konwekcja wir�w z wiersza i do i+1 w STEP
void convectVortexes(unsigned_int i);

//dyfuzja z wiersza i do i+1 w STEP
void diffuseVortexes(unsigned_int i);

//przesuni�cie wir�w z realizacj� zawijania si� domeny
struct point newPosition(struct point P0, struct point dP);

//r�wnomierny generator liczb losowych z przedzia�u od 0 do 1
real rand_0_1(void);

//funkcje matematyczne pracuj�ce na typie real
inline real powr(real x, real y);
inline real sqrr(real x);
inline real sqrtr(real x);
inline real modfr(real x, real* y);
inline real sinr(real x);
inline real cosr(real x);
inline real logr(real x);

//
//funkcje dodane do wersji quick
//

inline real floorr(real x);

inline struct point multiComplex(struct point z1, struct point z2);
inline struct point divComplex(struct point z1, struct point z2);

//znajduje subdomen� dla podanego punktu
struct subdomain* whichSubdomain(struct point P);

//m�wi czy punkt jest w danej subdomenie
inline int isPointInSubdomain(struct subdomain* sub, struct point P);

//wstawia indeksy wir�w do odpowiednich subdomen
int initiateSubdomains(void);

//oblicza momenty wirowe dla subdomeny *sub
void calcSubdomainsMoments(struct subdomain* sub, unsigned_int n);

//oblicza pr�dko�ci konwekcji dla po�o�e� wir�w w i-tym wierszu STEP
//w subdomene *sub
void calcVelInSubdomain_inside(struct subdomain* sub, unsigned_int i);
void calcVelInSubdomain_outside(struct subdomain* sub, unsigned_int i);     

//konwekcja z wiersza i do i+1 w STEP w subdomenie *sub
void convectVortexexInSubdomain(const struct subdomain* const sub, unsigned_int i);

//dyfuzja z wiersza i do i+1 w STEP w subdomenie *sub
void diffuseVortexesInSubdomain(struct subdomain* sub, unsigned_int i);

//przekazuje do odpowiednich domen wiry kt�re uciek�y z subdomeny *sub
int transferVortexesFromSubdomain(struct subdomain* sub, unsigned_int i);


#endif
