#ifndef vortex_h
#define vortex_h

#define ZERO 1E-10

#define TEXT_DATAFILE_EXT ".vrt"
#define BIN_DATAFILE_EXT ".vrb"
#define REPORT_FILE_EXT ".txt"

#define REALSIZE sizeof(real)
#define UNINTSIZE sizeof(unsigned_int)
#define POINTSIZE sizeof(struct point)
#define VORTEXSIZE sizeof(struct vortex)
#define SUBDOMAINSIZE sizeof(struct subdomain)
#define VRBHEADSIZE sizeof(struct vrbHead)

/* real and unsigned_int should be used in calculations
   locally different types may by used
*/

#define UNSIGNED_INT LONG //option are: INT LONG
#define REAL_TYPE DOUBLE //options are: FLOAT DOUBLE LONG_DOUBLE

#if (UNSIGNED_INT==INT)
	typedef unsigned int unsigned_int;
#elif (UNSIGNED_INT==LONG)
	typedef unsigned long int unsigned_int;
#else
	#define UNSIGNED_INT LONG
	typedef unsigned long int unsigned_int;
#endif

#if (REAL_TYPE==FLOAT)
    typedef float real;
#elif (REAL_TYPE==DOUBLE)
    typedef double real;
#elif (REL_TYPE==LONG_DOUBLE)
    typedef long double real;
#else
    #define REAL_TYPE DOUBLE
    typedef double real;
#endif

/*
   mathematical functions should be realized by conditiona compilation 
*/
    
struct point{ 
       real x,y;
};

struct vortex{
       struct point P;  /* vortex position */
       real G;          /* vortex strength */
};

#define NUMBER_OF_MOMENTS 5
//bede uzywal tej "zmiennej" tam gdzie bede liczyl momenty


struct subdomain{
       struct point O; /* the center of the subdomain */
       real G0; /*total vortex strenght (sum) */
       struct point Z[NUMBER_OF_MOMENTS+1]; /* moments of the discrete vortices
                                              the first is unused             */       
       unsigned_int *GAs;/*indices of vortexes that exist in this SUBDOMAIN */
       unsigned_int NGs;/* capacity of GAs[] */
       unsigned_int last;/* indicates the first free  space in GAs[] */
       
};

/* the vrb file: 
       1.header 
       2.vortexes strengths
       3.vortexes' shifts //dla 0 sa tylko poczatkowe
*/

struct vrbHead{
       unsigned_int NV;  /* number of vortexes  */
       unsigned_int NS;  /* number of shifts    */
       real v; /*viscosity*/
       real dT; /* simulation step duration */
       real D; /* the size of the square domain */
};

/*
  Values returned by functions
  0 - no error
  1 - problems with files
  2 - problems with memory
*/

#endif
