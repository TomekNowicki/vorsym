#include "domain.h"

#include "../../vortex.h"
#include "data.h"
#include "inout.h"

#include <math.h>
#include <stdlib.h>


inline int insideDomain(const struct point P){
    //sprawdza czy punkt znajduje siê wewn¹trz kwadratowej domeny
    //odczytuje zmienn¹ globaln¹: D
    real D2=D/2;
    if(P.x<=D2 && P.x>=-D2 && P.y<=D2 && P.y>=-D2 ) return 1; else return 0;
}

void calcVelocities(unsigned_int i){
    //oblicza prêdkoœci konwekcji dla po³o¿eñ wirów w n-tym wierszu STEP[]
    //modyfikuje zmienne globalne: VEL[]
    //czyta zmienne globalne: NSd, STEP[]
    //
    unsigned_int n, ns=NSd*NSd;
    //wyzeruj tablicê prêdkoœci
    for(n=0; n<NV; VEL[n].x=VEL[n++].y=0);
    //policz momenty dla subdomen
    for(n=0; n<ns; calcSubdomainsMoments(SB+n++,i)) ;    
    //dodaj sk³adowe prêdkoœci od bezpoœrednich oddzia³ywañ wewn¹trz subomen    
    for(n=0; n<ns; calcVelInSubdomain_inside(SB+n++,i)); 
    //dodaj sk³adowe prêdkoœci od oddzia³ywañ pozosta³ych subdomen
    for(n=0; n<ns; calcVelInSubdomain_outside(SB+n++,i));      
    return;
}

void convectVortexes(unsigned_int i){
     //przemieszcza wiry z po³o¿eñ w i-tym wierszu STEP wykonuj¹æ konwekcjê
     //nowe po³o¿enie zapisuje w (i+1)-tym wierszu STEP
     //dla po³o¿enia w (i+1)-tym wierszu STEP dokonuje reorganizacji wirów
     //modyfikuje zmienne globalne: STEP[]
     //czyta zmienne globalne: NSd, STEP[]
     unsigned_int n, ns=NSd*NSd;
     for(n=0; n<ns; convectVortexexInSubdomain(SB+n++,i));
     for(n=0; n<ns; transferVortexesFromSubdomain(SB+n++,i+1));
     return;  
}

void diffuseVortexes(unsigned_int i){
     //przemieszcza wiry z po³o¿eñ w i-tym wierszu STEP wykonuj¹æ ruchy Browna
     //nowe po³o¿enie zapisuje w (i+1)-tym wierszu STEP
     //dla po³o¿enia w (i+1)-tym wierszu STEP dokonuje reorganizacji wirów     
     //modyfikuje zmienne globalne: STEP[]
     unsigned_int n, ns=NSd*NSd;
     for(n=0; n<ns; diffuseVortexesInSubdomain(SB+n++,i));
     for(n=0; n<ns; transferVortexesFromSubdomain(SB+n++,i+1));     
     return;
}

struct point newPosition(const struct point P0, const struct point dP){
    //realizuje zawijanie siê domeny
    //odczytuje zmienne globalne: D
    //
    struct point P1={P0.x+dP.x,P0.y+dP.y};//nowe po³o¿enie
    while(!insideDomain(P1)){
       real D2=D/2;       
       if(P1.x>D2) P1.x-=D; else if(P1.x<-D2) P1.x+=D;
       if(P1.y>D2) P1.y-=D; else if(P1.y<-D2) P1.y+=D;
    }

    return P1;
}

real rand_0_1(void){
    //generator liczb losowych o rozk³adzie równomiernym od 0 do 1
    static real p=0.5; //pierwsza wartoœæ to seed
    static const real shift=1.01316;
    real ipart;//miejsce na czêœæ ca³kowit¹
    //nastêpne p
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
    //mno¿enie liczb zespolonych z1*z2
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
    //znajduje subdomenê dla podanego punktu
    //zwraca wskaŸnik subdomeny do której nale¿y punkty P lub NULL je¿eli
    //punkt le¿y pozasubdomenami
    //czyta zmienne globalne: D, SB
    //
    //czy punkt jest w domenie obliczeniowej (o to powinien zadbaæ newPosition)
    real D2=D/2; if(P.x<-D2 || P.x>D2 || P.y<-D2 || P.y>D2) return NULL; 
    //punkt jest w domenie - teraz do której subdomeny nale¿y
    if(NSd<=0) return NULL;//to nie powinno siê zdarzyæ
    real dD=D/NSd; 
    
    //ustalam która kolumna - zadecyduje x
    unsigned_int j;
    P.x+=D2;
    j=(unsigned_int)floorr(P.x/dD);
    if(j>(NSd-1)) j=NSd-1;
        
    //ustalam który wiersz - zadecyduje y    
    unsigned_int i;
    P.y+=D2; P.y=D-P.y;
    i=(unsigned_int)floorr(P.y/dD);
    if(i>(NSd-1))i=NSd-1;
    return SB+i*NSd+j;
}

inline int isPointInSubdomain(struct subdomain* sub, struct point P){
    //mówi czy punkt P jest w subdomenie *sub
    //
    if(sub==whichSubdomain(P)) return 1; else return 0;
}

int initiateSubdomains(void){
    //wyznacza œrodkowe punty subdomen, zeruje pozosta³e zmienne
    //przyporz¹dkowuje wiry do odpowiednich subdomen
    //modyfikuje zmienne globalne: SB
    //czyta zmienne globalne: D, NSd, NV, GA
    //
    unsigned_int i, j, k;  real dD, D2=D/2; struct subdomain* sub=NULL;
    if(NSd<=0) return 1; else dD=D/NSd;
    //obliczenie œrodkowego punktu subdomeny, zerowanie pozosta³ych
    for(i=0; i<NSd; i++){
        for(j=0; j<NSd; j++){
            sub=SB+i*NSd+j;//wskaŸnik do tej subdomeny [i,j]
            sub->O.x=-D2+j*dD+dD/2; sub->O.y=D2-(i+1)*dD+dD/2;
            //! TO MO¯NA BY£OBY ZROBIÆ POPRZEZ DODAWANIE
            sub->G0=0;
            for(k=0; k<=NUMBER_OF_MOMENTS; k++) sub->Z[k].x=sub->Z[k].y=0;
            for(k=0; k<sub->NGs; sub->GAs[k++]=0) ;
        }
    }    
    //wstawienie indeksów wirów do subdomen
    j=0;//iloœæ wirów które nie zosta³y przyporz¹dkowane do ¿adnej domeny
    for(i=0; i<NV; i++){
        //zlokalizuj subdomenê
        sub=whichSubdomain(STEP[i]);
        //wstaw wir do subdomeny         
        if(sub==NULL) j++; else if(addVortex(sub,i)) return 1;
    }
    return (int)j;
}

void calcSubdomainsMoments(struct subdomain* sub, unsigned_int n){
    //oblicza momeny od 1 do NUMBER_OF_MOMENTS dla subodmeny *sub
    //dla wspó³rzêdnych w n-tym wierszu STEP
    //czyta zmienne globalne: NV, STEP, GA
    //
    unsigned_int i, j;//indeksy pêtli
    //wyzerowanie starych wartoœci niezale¿nie czy s¹ wiry czy nie
    sub->G0=0; for(i=0; i<=NUMBER_OF_MOMENTS; i++) sub->Z[i].x=sub->Z[i].y=0;
    if(!sub->last) return;//je¿eli nie ma w domenie wirów to wyskocz
    //
    //po³o¿enie wiru wzglêdem œrodka subdomeny (zespolona) i jej kolejne potêgi
    struct point z0, z;
    //n-ty wiersz ma macierzy STEP
    struct point* S=STEP+n*NV;
    //suma wirowoœci w domenie oraz jej momenty - pêtla po wszystkich jej wirach
    for(i=0; i<sub->last; i++){
        //suma wirowoœci
        sub->G0+=GA[sub->GAs[i]];   
        //wspó³rzêdna i-tego wiru wzglêdem œrodka domeny O
        z0.x=S[sub->GAs[i]].x-sub->O.x; z0.y=S[sub->GAs[i]].y-sub->O.y;
        z=z0;//pierwsza potêga
        //liczniki momentów od 1 do NUMBER_OF_MOMENTS
        for(j=1; j<=NUMBER_OF_MOMENTS; j++){
            sub->Z[j].x+=GA[sub->GAs[i]]*z.x; sub->Z[j].y+=GA[sub->GAs[i]]*z.y;
            z=multiComplex(z,z0);//w nastêpnym obiegu pêtli jedna potêga wy¿ej
        }
    }
    //podzielenie przez sumê wirowoœci - policzenie momentów
    if(sub->G0) for(i=1; i<=NUMBER_OF_MOMENTS; i++) 
                    {sub->Z[i].x/=sub->G0; sub->Z[i].y/=sub->G0;}
    else for(i=1; i<=NUMBER_OF_MOMENTS; i++) sub->Z[i].x=sub->Z[i].y=0;
    //
    return;
}

void calcVelInSubdomain_inside(struct subdomain* sub, unsigned_int i){
    //oblicza dla wirów w subdomenie *sub prêdkoœci konwekcji wywo³ane
    //bezpoœrednim odzia³ywaniem wirów z tej samej subdomeny
    //wspó³rzêdne wirów w domenie *sub opisane s¹ w i-tym wierszy STEP[]     
    //wyliczone wartoœci dodaje do istnej¹cych wartoœci w VEL[]
    //modyfikuje zmienne globalne: VEL[]
    //czyta zmienne globalne: STEP[]
    //     
    if(!sub->last) return;//je¿eli nie ma w domenie wirów, to wyskocz
    const struct point* const S=STEP+i*NV;//i-ty wiersz w STEP
    unsigned_int m, n;//indeksy pêtli
    real d,//kwadrat odleg³oœci pomiêdzy wirami
         Vx,Vy,//prêdkoœæ w punkcie (sub->GAs[m]) od wiru z punkty (sub->GAs[n])
         ux,uy;//prêdkoœæ w (sub->GAs[m]) od chmury wirowej (akumulatory)
    //dla ka¿dego wiru w *sub
    for(m=0; m<sub->last; m++){
        ux=uy=0;//wyzeruj akumulator
        //policz wp³yw od ka¿dego wiru z subdomeny *sub
        for(n=0; n<sub->last; n++){
            //wyznacz kwadrat odleg³oœci od 'm' do 'n'
             d=sqrr(S[sub->GAs[m]].x-S[sub->GAs[n]].x)+
               sqrr(S[sub->GAs[m]].y-S[sub->GAs[n]].y);
             if(d<ZERO) continue;//wiry siê pokrywaj¹ - nie ma indukcji
             d=2*M_PI*d;//buduj dalej wyra¿enie
             //policz prêdkoœæ od wiru 'n'
             Vx=GA[sub->GAs[n]]*((S[sub->GAs[m]].y-S[sub->GAs[n]].y)/d); 
             Vy=GA[sub->GAs[n]]*((S[sub->GAs[n]].x-S[sub->GAs[m]].x)/d);
             //wp³yw od 'n'-tego wiru policzony - dosumuj do akumulatorów
             ux+=Vx; uy+=Vy;
        }
        //prêdkoœæ dla wiru 'm' ju¿ jest policzona - dodajemy do tego co jest
        VEL[sub->GAs[m]].x+=ux; VEL[sub->GAs[m]].y+=uy;        
    }
    return;
}

void calcVelInSubdomain_outside(struct subdomain* sub0, unsigned_int i){
    //oblicza dla wirów w subdomenie *sub0 prêdkoœci konwekcji wywo³ane
    //oddzia³ywaniem pozosta³ych subdomen (wykorzystuje ich momenty)
    //wspó³rzêdne wirów w domenie *sub0 opisane s¹ w i-tym wierszy STEP[]
    //wyliczone wartoœci dodaje do istnej¹cych wartoœci w VEL[]
    //modyfikuje zmienne globalne: VEL[]
    //czyta zmienne globalne: NSc; STEP[], SB[]
    //za³o¿enie: momenty s¹ policzone
    //
    if(!sub0->last) return;//je¿eli nie ma w domenie wirów, to wyskocz
    const struct point* const S=STEP+i*NV;//i-ty wiersz w STEP
    unsigned_int k, m, n, ns=NSd*NSd;//indeksy pêtli
    struct point z0, z,//odleg³oœæ wiru do œrodka subdomeny, i jej potêgi
                 temp;
    struct point Q,//prêdkoœæ w punkcie (sub->GAs[m]) od subdomeny *(SB+n)
                 q;//prêdkoœæi od wszystkich subdomen
    const struct subdomain* sub=NULL;//kolejna subdomena
    //dla ka¿dego wiru w *sub0
    for(m=0; m<sub0->last; m++){
        q.x=q.y=0;//wyzeruj akumulator
        //policz wp³yw od pozosta³ych subdomen
        for(n=0; n<ns; n++){
            if((sub=SB+n)==sub0) continue;//to ta sama subdomena
            //odleg³oœæ wiru 'm' od œrodka subdomeny 'n'
            z0.x=S[sub0->GAs[m]].x-sub->O.x; z0.y=S[sub0->GAs[m]].y-sub->O.y;
            z=z0;
            //
            Q.x=1; Q.y=0;
            //suma po wszystkich momentach z domeny sub
            for(k=1; k<=NUMBER_OF_MOMENTS; k++){
                //
                temp=divComplex(sub->Z[k],z);//Z[k]/z^k
                Q.x+=temp.x; Q.y+=temp.y;
                z=multiComplex(z,z0);//kolejna potêga odleg³oœci
            }
            temp.x=temp.y=(sub->G0)/(2*M_PI*(sqrr(z0.x)+sqrr(z0.y)));
            temp.x*=z0.y; temp.y*=z0.x;
            Q=multiComplex(temp,Q);
            Q.y=-Q.y;
            //wp³yw od 'n'-tej subdomeny policzony - dosumuj do akumulatorów
            q.x+=Q.x; q.y+=Q.y;
        }
        //prêdkoœæ dla wiru 'm' ju¿ jest policzona - dodajemy do tego co jest
        VEL[sub0->GAs[m]].x+=q.x; VEL[sub0->GAs[m]].y+=q.y;
    }
   
    return;
}

void convectVortexexInSubdomain(const struct subdomain* const sub, unsigned_int i){
     //przemieszcza wiry z subdomeny *sub z po³o¿eñ w i-tym wierszu STEP[]
     //na podstawie prêdkoœci zapisanych VEL (konwekcja), nowe po³o¿enie 
     //zapisuje w (i+1)-tym wierszu STEP[]
     //modyfikuje zmienne globalne: STEP[]
     //odczytuje zmienne globalne: NV, dT, STEP[]
     //     
     if(!sub->last) return;//je¿eli nie ma w domenie wirów, to wyskocz
     struct point dP;
     const struct point* const S0=STEP+i*NV;//i-ty wiersz w macierzy STEP
     struct point* const S1=STEP+(i+1)*NV;//(i+1)-ty wiersz w macierzy STEP
     //dla ka¿dego wiru wskazywanego przez macierz GAs[]
     unsigned_int j, n;
     for(n=0; n<sub->last; n++){
        j=sub->GAs[n];//indeks wiru, który bêdzie przesuwany
        //policz przesuniêcie na podstawie prêdkoœci i czasu
        dP.x=VEL[j].x*dT; dP.y=VEL[j].y*dT;
        //zapisz nowe wspó³rzêdne (przyrosty pierszego rzêdu)
        S1[j]=newPosition(S0[j],dP);
     }                      
     return;
}

void diffuseVortexesInSubdomain(struct subdomain* sub, unsigned_int i){
     //przemieszcza wiry z po³o¿eñ w i-tym wierszu STEP wykonuj¹æ ruchy Browna
     //nowe po³o¿enie zapisuje w (i+1)-tym wierszu STEP
     //modyfikuje zmienne globalne: STEP
     //odczytuje zmienne globalne: SB, STEP
     //
     if(!sub->last) return;//je¿eli nie ma w domenie wirów, to wyskocz
     const struct point* const S0=STEP+i*NV;//i-ty wiersz w macierzy STEP
     struct point* const S1=STEP+(i+1)*NV;//(i+1)-ty wiersz w macierzy STEP    
     unsigned_int n; real th, r;
     for(n=0; n<sub->last; n++){
         //oblicz k¹t i promieñ skoku              
         th=2*M_PI*rand_0_1(); r=sqrtr(4*v*dT*logr(1/rand_0_1()));              
         S1[sub->GAs[n]].x=S0[sub->GAs[n]].x+r*cosr(th);
         S1[sub->GAs[n]].y=S0[sub->GAs[n]].y+r*sinr(th);
     }
     return;
}

int transferVortexesFromSubdomain(struct subdomain* sub, unsigned_int i){
     //sprawdza czy wiry z macierzy sub->GAs[] s¹ ci¹gle w subdomnie *sub
     //je¿eli nie to znajduje odpowiedni¹ subdomenê i wstawia jej te wiry
     //usuwa je ze swojej tablicy
     //bierze wspó³rzêdne z wiersza i w STEP[]
     //
     if(!sub->last) return 0;//je¿eli nie ma w domenie wirów to wyskocz
     const struct point* const S=STEP+i*NV;//i-ty wiersz w macierzy STEP     
     //n pokazuje od koñca wiry w sub->GAs[], j to indeks wiru
     unsigned_int j, n=sub->last;
     do{
         j=sub->GAs[--n];
         if(!isPointInSubdomain(sub,S[j])){//wyszed³ poza *sub
             //znajdŸ odpowiedni¹ subdomenê i go tam dodaj
             if(addVortex(whichSubdomain(S[j]),j)) return 1;
             //usuñ go z tablicy subdomeny *sub
             sub->GAs[n]= sub->GAs[--sub->last];
         }         
     }while(n>0);
     return 0;//wszystko ok
     
}

//
