#ifndef EngineH
#define EngineH

#include "../../vortex.h"


//zawartosc pliku .vrb
extern vrbHead vrbH; //naglowek
extern real  *G; //moce wirow
extern point *STEP; //pozycje wirow


int takeMemory(unsigned_int nv, unsigned_int ns);
void freeMemory(void);
int readVrb(const char* pathName);


int resetImage(int size);
int saveImage(const char* pathName);

int drawVortexesForStep(unsigned_int step);


#endif
