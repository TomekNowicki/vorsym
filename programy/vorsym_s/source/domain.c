#include "domain.h"

#include "../../vortex.h"
#include "data.h"

#include <math.h>

inline int insideDomain(struct point P){
    //sprawdza czy punkt znajduje sie wewnatrz kwadratowej domeny
    //odczytuje zmienna globalna D
    real D2=D/2;
    if(P.x<=D2 && P.x>=-D2 && P.y<=D2 && P.y>=-D2 ) return 1; else return 0;
}

void calcVelocities(unsigned_int i){
     //oblicza predkosci konwekcji dla polozen wirów w i-tym wierszu STEP
     //modyfikuje zmienne globalne: VEL
     //odczytuje zmienne globalne: NV, GA, STEP
     //
     struct point *S=STEP+i*NV;//i-ty wiersz w STEP - jednowymiarowa macierz
     real  ux,uy, //skladowe predkosci w punkcjie m od chmury wirowej
           Vx,Vy, //skladowe predkosci w punkcjie m od wiru w punkcie n
           d;     //kwadrat odleglosci pomiedzy wirem w punkcie m, a wirem w n
     //
     unsigned_int m,n;
     //dla kazdego wiru od m==0 od m==NV-1
     for(m=0; m<NV; m++){
         //wyznacz skladowe predkosci poprzez sumowanie
         ux=uy=0;//to sa akumulatory
         for(n=0; n<NV; n++){
             //wyznacz kwadrat odleglosci     
             //d=(S[m].x-S[n].x)*(S[m].x-S[n].x)+(S[m].y-S[n].y)*(S[m].y-S[n].y);
             d=sqrr(S[m].x-S[n].x)+sqrr(S[m].y-S[n].y);
             if(d<ZERO) continue;//wiry sie pokrywaja - nie ma indukcji
             d=2*M_PI*d;
             //
             Vx=(S[m].y-S[n].y)/d;
             Vy=(S[n].x-S[m].x)/d;
             //dosumuj
             ux+=GA[n]*Vx; uy+=GA[n]*Vy;
         }
         //wyliczona predkosc zapisz do macierzy VEL
         VEL[m].x=ux; VEL[m].y=uy;
     }
     return;
}

void convectVortexes(unsigned_int i){
     //przemieszcza wiry z polozen w i-tym wierszu STEP
     //na podstawie predkosci zapisanych VEL (konwekcja)
     //nowe po³o¿enie zapisuje w (i+1)-tym wierszu STEP
     //modyfikuje zmienne globalne: STEP
     //odczytuje zmienne globalne: NV, dT, STEP
     //
     struct point *S0=STEP+i*NV,//i-ty wiersz w macierzy STEP
                  *S1=STEP+(i+1)*NV;//(i+1)-ty wiersz w macierzy STEP
     struct point dP;
     //dla ka¿dego wiru
     unsigned_int n;
     for(n=0; n<NV; n++){
        //policz przesuniecie na podstawie predkosci i czasu
        dP.x=VEL[n].x*dT; dP.y=VEL[n].y*dT;
        //zapisz nowe wspolrzedne (przyrosty pierszego rzedu)
        S1[n]=newPosition(S0[n],dP);
     }         
     return;
}

void diffuseVortexes(unsigned_int i){
     //przemieszcza wiry z polozen w i-tym wierszu STEP wykonujac ruchy Browna
     //nowe po³o¿enie zapisuje w (i+1)-tym wierszu STEP
     //modyfikuje zmienne globalne: STEP
     //odczytuje zmienne globalne: NV, STEP
     //
     struct point *S0=STEP+i*NV,//i-ty wiersz w macierzy STEP
                  *S1=STEP+(i+1)*NV;//(i+1)-ty wiersz w macierzy STEP
     real th, r;//kat skoku i promien skoku
     unsigned_int n;
     //dla kazdego wiru
     for(n=0; n<NV; n++){
         //oblicz kat i promien skoku
         th=2*M_PI*rand_0_1(); r=sqrtr(4*v*dT*logr(1/rand_0_1()));
         //zapisz nowe wspolrzedne
         S1[n].x=S0[n].x+r*cosr(th);
         S1[n].y=S0[n].y+r*sinr(th);
     }                   
     return;
}

struct point newPosition(const struct point P0, const struct point dP){
    //realizuje zawijanie sie domeny
    //odczytuje zmienne globalne: D
    //
    struct point P1={P0.x+dP.x,P0.y+dP.y};//nowe polozenie

    //!to nie jest najlepsze rozwiazanie - moze sie zapetlic
    while(!insideDomain(P1)){
       real D2=D/2;       
       if(P1.x>D2) P1.x-=D; else if(P1.x<-D2) P1.x+=D;
       if(P1.y>D2) P1.y-=D; else if(P1.y<-D2) P1.y+=D;
    }
    return P1;
}

real rand_0_1(void){
    //generator liczb losowych o rozkladzie rownomiernym od 0 do 1
    static real p=0.5; //pierwsza wartosc to seed
    static const real shift=1.01316;
    real ipart;//miejsce na czesc calkowita
    //nastepne p
    p=powr(p+shift,5); p=modfr(p,&ipart);
    return p;
}

inline real powr(real x, real y){
    #if (REAL_TYPE==FLOAT)
     return powf(x,y);
    #elif (REAL_TYPE==LONG_DOUBLE)
     return powl(x,y);
    #else
     return pow(x,y);
    #endif        
}

inline real sqrr(real x){
    return powr(x,2);       
}

inline real sqrtr(real x){
    return pow(x,0.5);
}

inline real modfr(real x, real* y){
    #if (REAL_TYPE==FLOAT)
     return modff(x,y);
    #elif (REAL_TYPE==LONG_DOUBLE)
     return modfl(x,y);
    #else
     return modf(x,y);
    #endif
}

inline real sinr(real x){
    #if (REAL_TYPE==FLOAT)
     return sinf(x);
    #elif (REAL_TYPE==LONG_DOUBLE)
     return sinl(x);
    #else
     return sin(x);
    #endif
}

inline real cosr(real x){
    #if (REAL_TYPE==FLOAT)
     return cosf(x);
    #elif (REAL_TYPE==LONG_DOUBLE)
     return cosl(x);
    #else
     return cos(x);
    #endif
}

inline real logr(real x){
    #if (REAL_TYPE==FLOAT)
     return logf(x);
    #elif (REAL_TYPE==LONG_DOUBLE)
     return logl(x);
    #else
     return log(x);
    #endif
}

//
