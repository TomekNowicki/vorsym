#include "data.h"

#include "../../vortex.h"

#include <stdlib.h> //NULL

//zmienne globalne - opis w pliku "data.h"

char inputFile[STRINGS_CAPACITY]="";
char outputFile[STRINGS_CAPACITY]="";
unsigned_int NV=0; 
unsigned_int NS=0;
real *GA=NULL;
struct point *VEL=NULL;
struct point *STEP=NULL;
real v=0;
real dT=0;
real D;
//
