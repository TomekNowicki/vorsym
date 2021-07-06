#include "data.h"
#include "../../vortex.h"
#include <stdlib.h>

//
//zmienne globalne - szerszy opis w pliku "data.h"

//parametry symulacji
char inputFile[STRINGS_CAPACITY]="";
char outputFile[STRINGS_CAPACITY]="";
unsigned_int NV=0, NS=0; 
real D=0, v=0, dT=0;

//dekompozycja zadania - PROCESY
unsigned_int NP=0, rowP=0, colP=0;
real Dp=0;
struct point O={0,0};

//dekompozycja zadania - SUBDOMENY
unsigned_int NSd=1;
struct subdomain *SB=NULL;
unsigned_int NVs=0;

//tablice 'obliczeniowe'
int* EX=NULL;
real *GA=NULL;
struct point *VEL=NULL;
struct point *STEP=NULL;

//wymiana informacji o momentach subdomenowych pomiedzy procesami
real *MS=NULL;
struct point *OS=NULL;

//wymiana wirow
unsigned_int *NVfP=NULL;
unsigned_int *NVtP=NULL;
struct v_p_handle *VfP=NULL;
struct v_p_handle *VtP=NULL;

unsigned_int V_Ps=0;

//
//funkcje sluzace do obslugi struktur danych

inline struct point* OS_get(unsigned_int r_proc, unsigned_int c_proc,
					 unsigned_int r_sub, unsigned_int c_sub){
	//zwraca wskaznik do subdomeny w wierszu r_sub i kolumnie c_sub
	//procesu z wiersza r_proc i koluny c_proc
	//czyta zmienne globalne: NP, NSd, OS[]
	//!ZALOZENIA: ..... 
	//
	unsigned_int offset_proc=(r_proc*NP+c_proc)*NSd*NSd,
				 offset_sub=(r_sub*NSd+c_sub);
	return OS+offset_proc+offset_sub;
}

inline real* MS_getStart(unsigned_int r_proc, unsigned_int c_proc,
					unsigned_int r_sub, unsigned_int c_sub, unsigned_int i){
	//zwraca wskaznik ...
	//nie modyfikuje zmiennych globalnych
	//czyta zmienne globalne NUMBER_OF_MOMENTS, NP, NSd, MS[]
	//
	unsigned_int ms_s=1+NUMBER_OF_MOMENTS*2,
				 ms_p=NSd*NSd*ms_s,
				 offset_proc=(r_proc*NP +c_proc)*ms_p,
				 offset_sub =(r_sub *NSd+c_sub )*ms_s;
	return MS+offset_proc+offset_sub+i;
}

inline real MS_getSum(unsigned_int r_proc, unsigned_int c_proc,
					  unsigned_int r_sub, unsigned_int c_sub){
	//zwraca sume wirowosci w subdomenie [r_sub, c_sub] procesu [r_proc, c_proc]
	//
	real G0=*MS_getStart(r_proc,c_proc,r_sub,c_sub,0);
	return G0;
}
 
inline void MS_getMoments(unsigned_int r_proc, unsigned_int c_proc,
					unsigned_int r_sub, unsigned_int c_sub, struct point* Z){
	//wypelna tablice Z[] wartosciami momentow subdomen
	//
	unsigned_int i, k;
	real *ms_tab=MS_getStart(r_proc,c_proc,r_sub,c_sub,0);
	Z[0].x=Z[0].y=0;
	for(k=0, i=1; i<=NUMBER_OF_MOMENTS; i++){
		Z[i].x=ms_tab[++k]; Z[i].y=ms_tab[++k];
	}
	return;
}

void updateSubdomainsMoments(void){
	//przepisuje wartosci momentow ze struktury SB[] do MS[] w miejsce
	//dla procesu z wiersza rowP i kolumny colP
	//modyfikuje zmienne globalne: MS[]
	//czyta zmienne globalne: NP,NSd,rowP,colP,NUMBER_OF_MOMENTS,SB[]
	//
	unsigned_int i,j,k,l; struct subdomain *sub=NULL; real *ms_tab=NULL;
	for(i=0; i<NSd; i++)
		for(j=0; j<NSd; j++){
			sub=SB+i*NSd+j;
			ms_tab=MS_getStart(rowP,colP,i,j,0);
			ms_tab[0]=sub->G0;
			for(k=0,l=1; l<=NUMBER_OF_MOMENTS; l++){
				ms_tab[++k]=sub->Z[l].x; ms_tab[++k]=sub->Z[l].y;
			}
		}
	//
	return;
}

//
