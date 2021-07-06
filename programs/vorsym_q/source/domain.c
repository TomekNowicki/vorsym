#include "domain.h"

#include "../../vortex.h"
#include "data.h"
#include "inout.h"

#include <math.h>
#include <stdlib.h>


inline int insideDomain(const struct point P){
    //sprawdza czy punkt znajduje si� wewn�trz kwadratowej domeny
    //odczytuje zmienn� globaln�: D
    real D2=D/2;
    if(P.x<=D2 && P.x>=-D2 && P.y<=D2 && P.y>=-D2 ) return 1; else return 0;
}

void calcVelocities(unsigned_int i){
    //oblicza pr�dko�ci konwekcji dla po�o�e� wir�w w n-tym wierszu STEP[]
    //modyfikuje zmienne globalne: VEL[]
    //czyta zmienne globalne: NSd, STEP[]
    //
    unsigned_int n, ns=NSd*NSd;
    //wyzeruj tablic� pr�dko�ci
    for(n=0; n<NV; VEL[n].x=VEL[n++].y=0);
    //policz momenty dla subdomen
    for(n=0; n<ns; calcSubdomainsMoments(SB+n++,i)) ;    
    //dodaj sk�adowe pr�dko�ci od bezpo�rednich oddzia�ywa� wewn�trz subomen    
    for(n=0; n<ns; calcVelInSubdomain_inside(SB+n++,i)); 
    //dodaj sk�adowe pr�dko�ci od oddzia�ywa� pozosta�ych subdomen
    for(n=0; n<ns; calcVelInSubdomain_outside(SB+n++,i));      
    return;
}

void convectVortexes(unsigned_int i){
     //przemieszcza wiry z po�o�e� w i-tym wierszu STEP wykonuj�� konwekcj�
     //nowe po�o�enie zapisuje w (i+1)-tym wierszu STEP
     //dla po�o�enia w (i+1)-tym wierszu STEP dokonuje reorganizacji wir�w
     //modyfikuje zmienne globalne: STEP[]
     //czyta zmienne globalne: NSd, STEP[]
     unsigned_int n, ns=NSd*NSd;
     for(n=0; n<ns; convectVortexexInSubdomain(SB+n++,i));
     for(n=0; n<ns; transferVortexesFromSubdomain(SB+n++,i+1));
     return;  
}

void diffuseVortexes(unsigned_int i){
     //przemieszcza wiry z po�o�e� w i-tym wierszu STEP wykonuj�� ruchy Browna
     //nowe po�o�enie zapisuje w (i+1)-tym wierszu STEP
     //dla po�o�enia w (i+1)-tym wierszu STEP dokonuje reorganizacji wir�w     
     //modyfikuje zmienne globalne: STEP[]
     unsigned_int n, ns=NSd*NSd;
     for(n=0; n<ns; diffuseVortexesInSubdomain(SB+n++,i));
     for(n=0; n<ns; transferVortexesFromSubdomain(SB+n++,i+1));     
     return;
}

struct point newPosition(const struct point P0, const struct point dP){
    //realizuje zawijanie si� domeny
    //odczytuje zmienne globalne: D
    //
    struct point P1={P0.x+dP.x,P0.y+dP.y};//nowe po�o�enie
    while(!insideDomain(P1)){
       real D2=D/2;       
       if(P1.x>D2) P1.x-=D; else if(P1.x<-D2) P1.x+=D;
       if(P1.y>D2) P1.y-=D; else if(P1.y<-D2) P1.y+=D;
    }

    return P1;
}

real rand_0_1(void){
    //generator liczb losowych o rozk�adzie r�wnomiernym od 0 do 1
    static real p=0.5; //pierwsza warto�� to seed
    static const real shift=1.01316;
    real ipart;//miejsce na cz�� ca�kowit�
    //nast�pne p
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

inline real floorr(real x){
    #if (REAL_TYPE==FLOAT)
     return floorf(x);
    #elif (REAL_TYPE==LONG_DOUBLE)
     return floorl(x);
    #else
     return floor(x);
    #endif
       
}

//
//funkcje dodane do wersji quick
//

inline struct point multiComplex(struct point z1, struct point z2){
    //mno�enie liczb zespolonych z1*z2
    struct point z0={z1.x*z2.x-z1.y*z2.y,z1.x*z2.y+z1.y*z2.x};
    return z0;
}

inline struct point divComplex(struct point z1, struct point z2){
    //dzielenie liczb zespolonych z1/z2;
    //UWAGA: nie jest zabezpieczone przed dzieleniem przez zero
    real d=z2.x*z2.x+z2.y*z2.y;
    struct point z0={(z1.x*z2.x+z1.y*z2.y)/d,(z2.x*z1.y-z1.x*z2.y)/d};
    return z0;
}

struct subdomain* whichSubdomain(struct point P){
    //znajduje subdomen� dla podanego punktu
    //zwraca wska�nik subdomeny do kt�rej nale�y punkty P lub NULL je�eli
    //punkt le�y pozasubdomenami
    //czyta zmienne globalne: D, SB
    //
    //czy punkt jest w domenie obliczeniowej (o to powinien zadba� newPosition)
    real D2=D/2; if(P.x<-D2 || P.x>D2 || P.y<-D2 || P.y>D2) return NULL; 
    //punkt jest w domenie - teraz do kt�rej subdomeny nale�y
    if(NSd<=0) return NULL;//to nie powinno si� zdarzy�
    real dD=D/NSd; 
    
    //ustalam kt�ra kolumna - zadecyduje x
    unsigned_int j;
    P.x+=D2;
    j=(unsigned_int)floorr(P.x/dD);
    if(j>(NSd-1)) j=NSd-1;
        
    //ustalam kt�ry wiersz - zadecyduje y    
    unsigned_int i;
    P.y+=D2; P.y=D-P.y;
    i=(unsigned_int)floorr(P.y/dD);
    if(i>(NSd-1))i=NSd-1;
    return SB+i*NSd+j;
}

inline int isPointInSubdomain(struct subdomain* sub, struct point P){
    //m�wi czy punkt P jest w subdomenie *sub
    //
    if(sub==whichSubdomain(P)) return 1; else return 0;
}

int initiateSubdomains(void){
    //wyznacza �rodkowe punty subdomen, zeruje pozosta�e zmienne
    //przyporz�dkowuje wiry do odpowiednich subdomen
    //modyfikuje zmienne globalne: SB
    //czyta zmienne globalne: D, NSd, NV, GA
    //
    unsigned_int i, j, k;  real dD, D2=D/2; struct subdomain* sub=NULL;
    if(NSd<=0) return 1; else dD=D/NSd;
    //obliczenie �rodkowego punktu subdomeny, zerowanie pozosta�ych
    for(i=0; i<NSd; i++){
        for(j=0; j<NSd; j++){
            sub=SB+i*NSd+j;//wska�nik do tej subdomeny [i,j]
            sub->O.x=-D2+j*dD+dD/2; sub->O.y=D2-(i+1)*dD+dD/2;
            //! TO MO�NA BY�OBY ZROBI� POPRZEZ DODAWANIE
            sub->G0=0;
            for(k=0; k<=NUMBER_OF_MOMENTS; k++) sub->Z[k].x=sub->Z[k].y=0;
            for(k=0; k<sub->NGs; sub->GAs[k++]=0) ;
        }
    }    
    //wstawienie indeks�w wir�w do subdomen
    j=0;//ilo�� wir�w kt�re nie zosta�y przyporz�dkowane do �adnej domeny
    for(i=0; i<NV; i++){
        //zlokalizuj subdomen�
        sub=whichSubdomain(STEP[i]);
        //wstaw wir do subdomeny         
        if(sub==NULL) j++; else if(addVortex(sub,i)) return 1;
    }
    return (int)j;
}

void calcSubdomainsMoments(struct subdomain* sub, unsigned_int n){
    //oblicza momeny od 1 do NUMBER_OF_MOMENTS dla subodmeny *sub
    //dla wsp�rz�dnych w n-tym wierszu STEP
    //czyta zmienne globalne: NV, STEP, GA
    //
    unsigned_int i, j;//indeksy p�tli
    //wyzerowanie starych warto�ci niezale�nie czy s� wiry czy nie
    sub->G0=0; for(i=0; i<=NUMBER_OF_MOMENTS; i++) sub->Z[i].x=sub->Z[i].y=0;
    if(!sub->last) return;//je�eli nie ma w domenie wir�w to wyskocz
    //
    //po�o�enie wiru wzgl�dem �rodka subdomeny (zespolona) i jej kolejne pot�gi
    struct point z0, z;
    //n-ty wiersz ma macierzy STEP
    struct point* S=STEP+n*NV;
    //suma wirowo�ci w domenie oraz jej momenty - p�tla po wszystkich jej wirach
    for(i=0; i<sub->last; i++){
        //suma wirowo�ci
        sub->G0+=GA[sub->GAs[i]];   
        //wsp�rz�dna i-tego wiru wzgl�dem �rodka domeny O
        z0.x=S[sub->GAs[i]].x-sub->O.x; z0.y=S[sub->GAs[i]].y-sub->O.y;
        z=z0;//pierwsza pot�ga
        //liczniki moment�w od 1 do NUMBER_OF_MOMENTS
        for(j=1; j<=NUMBER_OF_MOMENTS; j++){
            sub->Z[j].x+=GA[sub->GAs[i]]*z.x; sub->Z[j].y+=GA[sub->GAs[i]]*z.y;
            z=multiComplex(z,z0);//w nast�pnym obiegu p�tli jedna pot�ga wy�ej
        }
    }
    //podzielenie przez sum� wirowo�ci - policzenie moment�w
    if(sub->G0) for(i=1; i<=NUMBER_OF_MOMENTS; i++) 
                    {sub->Z[i].x/=sub->G0; sub->Z[i].y/=sub->G0;}
    else for(i=1; i<=NUMBER_OF_MOMENTS; i++) sub->Z[i].x=sub->Z[i].y=0;
    //
    return;
}

void calcVelInSubdomain_inside(struct subdomain* sub, unsigned_int i){
    //oblicza dla wir�w w subdomenie *sub pr�dko�ci konwekcji wywo�ane
    //bezpo�rednim odzia�ywaniem wir�w z tej samej subdomeny
    //wsp�rz�dne wir�w w domenie *sub opisane s� w i-tym wierszy STEP[]     
    //wyliczone warto�ci dodaje do istnej�cych warto�ci w VEL[]
    //modyfikuje zmienne globalne: VEL[]
    //czyta zmienne globalne: STEP[]
    //     
    if(!sub->last) return;//je�eli nie ma w domenie wir�w, to wyskocz
    const struct point* const S=STEP+i*NV;//i-ty wiersz w STEP
    unsigned_int m, n;//indeksy p�tli
    real d,//kwadrat odleg�o�ci pomi�dzy wirami
         Vx,Vy,//pr�dko�� w punkcie (sub->GAs[m]) od wiru z punkty (sub->GAs[n])
         ux,uy;//pr�dko�� w (sub->GAs[m]) od chmury wirowej (akumulatory)
    //dla ka�dego wiru w *sub
    for(m=0; m<sub->last; m++){
        ux=uy=0;//wyzeruj akumulator
        //policz wp�yw od ka�dego wiru z subdomeny *sub
        for(n=0; n<sub->last; n++){
            //wyznacz kwadrat odleg�o�ci od 'm' do 'n'
             d=sqrr(S[sub->GAs[m]].x-S[sub->GAs[n]].x)+
               sqrr(S[sub->GAs[m]].y-S[sub->GAs[n]].y);
             if(d<ZERO) continue;//wiry si� pokrywaj� - nie ma indukcji
             d=2*M_PI*d;//buduj dalej wyra�enie
             //policz pr�dko�� od wiru 'n'
             Vx=GA[sub->GAs[n]]*((S[sub->GAs[m]].y-S[sub->GAs[n]].y)/d); 
             Vy=GA[sub->GAs[n]]*((S[sub->GAs[n]].x-S[sub->GAs[m]].x)/d);
             //wp�yw od 'n'-tego wiru policzony - dosumuj do akumulator�w
             ux+=Vx; uy+=Vy;
        }
        //pr�dko�� dla wiru 'm' ju� jest policzona - dodajemy do tego co jest
        VEL[sub->GAs[m]].x+=ux; VEL[sub->GAs[m]].y+=uy;        
    }
    return;
}

void calcVelInSubdomain_outside(struct subdomain* sub0, unsigned_int i){
    //oblicza dla wir�w w subdomenie *sub0 pr�dko�ci konwekcji wywo�ane
    //oddzia�ywaniem pozosta�ych subdomen (wykorzystuje ich momenty)
    //wsp�rz�dne wir�w w domenie *sub0 opisane s� w i-tym wierszy STEP[]
    //wyliczone warto�ci dodaje do istnej�cych warto�ci w VEL[]
    //modyfikuje zmienne globalne: VEL[]
    //czyta zmienne globalne: NSc; STEP[], SB[]
    //za�o�enie: momenty s� policzone
    //
    if(!sub0->last) return;//je�eli nie ma w domenie wir�w, to wyskocz
    const struct point* const S=STEP+i*NV;//i-ty wiersz w STEP
    unsigned_int k, m, n, ns=NSd*NSd;//indeksy p�tli
    struct point z0, z,//odleg�o�� wiru do �rodka subdomeny, i jej pot�gi
                 temp;
    struct point Q,//pr�dko�� w punkcie (sub->GAs[m]) od subdomeny *(SB+n)
                 q;//pr�dko��i od wszystkich subdomen
    const struct subdomain* sub=NULL;//kolejna subdomena
    //dla ka�dego wiru w *sub0
    for(m=0; m<sub0->last; m++){
        q.x=q.y=0;//wyzeruj akumulator
        //policz wp�yw od pozosta�ych subdomen
        for(n=0; n<ns; n++){
            if((sub=SB+n)==sub0) continue;//to ta sama subdomena
            //odleg�o�� wiru 'm' od �rodka subdomeny 'n'
            z0.x=S[sub0->GAs[m]].x-sub->O.x; z0.y=S[sub0->GAs[m]].y-sub->O.y;
            z=z0;
            //
            Q.x=1; Q.y=0;
            //suma po wszystkich momentach z domeny sub
            for(k=1; k<=NUMBER_OF_MOMENTS; k++){
                //
                temp=divComplex(sub->Z[k],z);//Z[k]/z^k
                Q.x+=temp.x; Q.y+=temp.y;
                z=multiComplex(z,z0);//kolejna pot�ga odleg�o�ci
            }
            temp.x=temp.y=(sub->G0)/(2*M_PI*(sqrr(z0.x)+sqrr(z0.y)));
            temp.x*=z0.y; temp.y*=z0.x;
            Q=multiComplex(temp,Q);
            Q.y=-Q.y;
            //wp�yw od 'n'-tej subdomeny policzony - dosumuj do akumulator�w
            q.x+=Q.x; q.y+=Q.y;
        }
        //pr�dko�� dla wiru 'm' ju� jest policzona - dodajemy do tego co jest
        VEL[sub0->GAs[m]].x+=q.x; VEL[sub0->GAs[m]].y+=q.y;
    }
   
    return;
}

void convectVortexexInSubdomain(const struct subdomain* const sub, unsigned_int i){
     //przemieszcza wiry z subdomeny *sub z po�o�e� w i-tym wierszu STEP[]
     //na podstawie pr�dko�ci zapisanych VEL (konwekcja), nowe po�o�enie 
     //zapisuje w (i+1)-tym wierszu STEP[]
     //modyfikuje zmienne globalne: STEP[]
     //odczytuje zmienne globalne: NV, dT, STEP[]
     //     
     if(!sub->last) return;//je�eli nie ma w domenie wir�w, to wyskocz
     struct point dP;
     const struct point* const S0=STEP+i*NV;//i-ty wiersz w macierzy STEP
     struct point* const S1=STEP+(i+1)*NV;//(i+1)-ty wiersz w macierzy STEP
     //dla ka�dego wiru wskazywanego przez macierz GAs[]
     unsigned_int j, n;
     for(n=0; n<sub->last; n++){
        j=sub->GAs[n];//indeks wiru, kt�ry b�dzie przesuwany
        //policz przesuni�cie na podstawie pr�dko�ci i czasu
        dP.x=VEL[j].x*dT; dP.y=VEL[j].y*dT;
        //zapisz nowe wsp�rz�dne (przyrosty pierszego rz�du)
        S1[j]=newPosition(S0[j],dP);
     }                      
     return;
}

void diffuseVortexesInSubdomain(struct subdomain* sub, unsigned_int i){
     //przemieszcza wiry z po�o�e� w i-tym wierszu STEP wykonuj�� ruchy Browna
     //nowe po�o�enie zapisuje w (i+1)-tym wierszu STEP
     //modyfikuje zmienne globalne: STEP
     //odczytuje zmienne globalne: SB, STEP
     //
     if(!sub->last) return;//je�eli nie ma w domenie wir�w, to wyskocz
     const struct point* const S0=STEP+i*NV;//i-ty wiersz w macierzy STEP
     struct point* const S1=STEP+(i+1)*NV;//(i+1)-ty wiersz w macierzy STEP    
     unsigned_int n; real th, r;
     for(n=0; n<sub->last; n++){
         //oblicz k�t i promie� skoku              
         th=2*M_PI*rand_0_1(); r=sqrtr(4*v*dT*logr(1/rand_0_1()));              
         S1[sub->GAs[n]].x=S0[sub->GAs[n]].x+r*cosr(th);
         S1[sub->GAs[n]].y=S0[sub->GAs[n]].y+r*sinr(th);
     }
     return;
}

int transferVortexesFromSubdomain(struct subdomain* sub, unsigned_int i){
     //sprawdza czy wiry z macierzy sub->GAs[] s� ci�gle w subdomnie *sub
     //je�eli nie to znajduje odpowiedni� subdomen� i wstawia jej te wiry
     //usuwa je ze swojej tablicy
     //bierze wsp�rz�dne z wiersza i w STEP[]
     //
     if(!sub->last) return 0;//je�eli nie ma w domenie wir�w to wyskocz
     const struct point* const S=STEP+i*NV;//i-ty wiersz w macierzy STEP     
     //n pokazuje od ko�ca wiry w sub->GAs[], j to indeks wiru
     unsigned_int j, n=sub->last;
     do{
         j=sub->GAs[--n];
         if(!isPointInSubdomain(sub,S[j])){//wyszed� poza *sub
             //znajd� odpowiedni� subdomen� i go tam dodaj
             if(addVortex(whichSubdomain(S[j]),j)) return 1;
             //usu� go z tablicy subdomeny *sub
             sub->GAs[n]= sub->GAs[--sub->last];
         }         
     }while(n>0);
     return 0;//wszystko ok
     
}

//
