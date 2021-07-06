#ifndef domain_h
#define domain_h

#include "../../vortex.h"

//sprawdza czy punkt znajduje siê wewn¹trz domeny
inline int insideDomain(struct point P);

//oblicza predkosci konwekcji dla polozen wirow w i-tym wierszu STEP
void calcVelocities(unsigned_int i);

//konwekcja wirow z wiersza i do i+1 w STEP
void convectVortexes(unsigned_int i);

//dyfuzja z wiersza i do i+1 w STEP
void diffuseVortexes(unsigned_int i);

//przesuniecie wirow z realizacja zawijania sie domeny
struct point newPosition(struct point P0, struct point dP);

//rownomierny generator liczb losowych z przedzialu od 0 do 1
real rand_0_1(void);

//funkcje matematyczne pracujace na typie real
inline real powr(real x, real y);
inline real sqrr(real x);
inline real sqrtr(real x);
inline real modfr(real x, real* y);
inline real sinr(real x);
inline real cosr(real x);
inline real logr(real x);

#endif
