#include "domain.h"
#include "../../vortex.h"
#include "data.h"
#include "inout.h"
#include <math.h>
#include <stdlib.h>

//!UWAGA: Problem wiru na boku kwadratu jest ciagle nierozwiazany
//moze zaliczyc po prosut dwa boki - wtedy ewentualnie strace wir ale nie bedzie bledu
//tzn nie zdubluja sie - trzeba zaliczy prawy i gorny tak ja w whichSubSq..
//tak jest w tej chwili

inline int insideSquare(real d, struct point center, struct point p){
	//boki kwadratu o dlugosci d i srodku center sa rownolegle do osi globalnego ukladu
	//wspolrzednych; funkcja zwraca 1 jezeli punkt p znajduje sie
	//wewnatrz kwadratu
	//nie uzywa zmiennych globalnych
	//
	real d2=d/2;
	p.x-=center.x; p.y-=center.y;//sprowadzenie punktu do lokalnego ukladu wspolrzednych
	if(p.x<=d2 && p.x>-d2 && p.y<=d2 && p.y>-d2 ) return 1;
	else return 0;	
}

inline struct point subSquareCenter(real d, struct point center,
                       unsigned_int n, unsigned_int r, unsigned_int c){
	//kwadrat o boku d ktorego srodek znajduje sie w punkcie center
	//dzieli na n*n subkwadratow i zwraca srodek subkwadratu
	//w wierszu r i kolumnie c (w globalnym ukladzie wspolrzednych)
	//nie uzywa zmiennych globalnych	
	//!ZALOZENIA: n>0
	real d_sub=d/n, d_sub_2=d_sub/2, d_2=d/2;
	struct point sub_center={-d_2+d_sub_2+c*d_sub,d_2-d_sub_2-r*d_sub};
	sub_center.x+=center.x; sub_center.y+=center.y;//transformacja do globalnego ukladu wspolrzednych
	return sub_center;
}

inline int whichSubSquare(real d, struct point center, unsigned_int n,
					 struct point p, unsigned_int *r, unsigned_int *c){
	//boki kwadratu o dlugosci d i srodku O sa rownolegle do osi
	//globalnego ukladu wspolrzednych; kwadrat jest podzielony na n*n
	//subkwadratow; funkcja okresla do ktorego subkwadratu nalezy punkt
	//P (dany w globalnym ukladzie wspolrzednych); 
	//nie uzywa zmiennych globalnych
	//!ZALOZENIE: n>0
	//
	*r=*c=0;
	if(!insideSquare(d,center,p)) return 0;//jezeli punkt jest poza wyskocz
	p.x-=center.x; p.y-=center.y;//sprowadzenie punktu do lokalnego ukladu kwadratu
	real ds=d/n, //dlugosc boku subdomeny
		 d2=d/2; //polowa dlugosci boku kwadratu
	//ustalam ktory wiersz (zadecyduje y)
	p.y+=d2; p.y=d-p.y;
	*r=(unsigned_int)floorr(p.y/ds);
	if(*r>(n-1)) *r=n-1;
	//ustalam ktore kolumna (zadecyduje x)
	p.x+=d2;
	*c=(unsigned_int)floorr(p.x/ds);
	if(*c>(n-1)) *c=n-1;
	return 0;//subkwadrat zostal znaleziony wyniki sa w *r *c
}

inline struct point simulationCenter(void){
	//zwraca srodek DOMENY_SYMULACJI (just for fun)
	//nie uzywa zmiennych globalnych
	//
	struct point center_sym={0,0};
	return center_sym;
}

inline struct point procesCenter(real d_sym, unsigned_int np, unsigned_int r_proc, unsigned_int c_proc){
	//zwraca srodek DOMENY_PROCESU z wiersza r i kolumy c
	//DOMENA_SYMULACJI ma wymiar d, np*np to ilosc procesow
	//nie uzywa zmiennych globalnych
	//!ZALOZENIA: d_sym>0; np>0; c,r <np
	//
	struct point center_proc=subSquareCenter(d_sym,simulationCenter(),np,r_proc,c_proc);
	return center_proc;
}

struct point subdomainCenter(real d_sym, unsigned_int np, unsigned_int nsd,
							 unsigned_int r_proc, unsigned_int c_proc,
							 unsigned_int r_sub, unsigned_int c_sub){
	//zwraca srodek SUBDOMENY z wiersza r_sub i kolumny c_sub
	//dla DOMENY_PROCESU z wiersza r_proc i kolummy c_proc
	//np*np -ilosc procesow, nsd*nsd -ilosc subdomen	
	//!ZALOZENIA: d_sym>0; np,nsd>0; r_proc,c_proc <np, r_sub,c_sub <nsd
	//
	struct point center_proc=subSquareCenter(d_sym,simulationCenter(),np,r_proc,c_proc);
	struct point center_sub =subSquareCenter(d_sym/np,center_proc,nsd,r_sub,c_sub);
	return center_sub;
}

inline int insideSimulationDomain(struct point P){
    //sprawdza czy punkt znajduje sie wewnatrz DOMENY_SYMULACJI
    //odczytuje zmienne globalne: D
    //
    return insideSquare(D, simulationCenter(),P);   
}

int whichProcess(struct point P, unsigned_int *r, unsigned_int *c){
	//...
	//
	if(whichSubSquare(D,simulationCenter(),NP,P,r,c)) return 1;
	return 0;
}

inline int insideProcesDomain(struct point P){
	//zwraca 1 jezeli punkt P jest w TEJ DOMENIE_PROCESU, 0 jezeli nie
	//czyta zmienne globalne: Dp, O
	//	
	unsigned_int r,c;
	if(whichProcess(P,&r,&c)) return 0;
	if(r==rowP && c==colP) return 1; else return 0;
}

struct subdomain* whichSubdomain(struct point P){
	//dotyczy TEGO PROCESU
    //znajduje subdomene dla podanego punktu P
    //zwraca wskaznik subdomeny do ktorej nalezy punkty P lub NULL jezeli
    //punkt lezy poza subdomenami
    //czyta zmienne globalne: Dp, O NSd, SB[]
    //
	unsigned_int r,c;
	if(whichSubSquare(Dp,O,NSd,P,&r,&c)) return NULL;
	return SB+r*NSd+c;
}

inline int isPointInSubdomain(struct subdomain* sub, struct point P){
	//dotyczyy TEGO PROCESU
    //mówi czy punkt P jest w subdomenie *sub
    //
    if(sub==whichSubdomain(P)) return 1; else return 0;
}

int initiateProcesDomain(int r, int c){
	//oblicza parametry DOMENY_PROCESU
	//r-wiersz, c-kolumna w macierzy procesow
	//modyfikuje zmienne globalne: rowP, colP, Dp, O
	//czyta zmienne globalne: D, NP
	//
	if(!NP) return 1;
	rowP=r; colP=c;//polorzenie w wirtualnej topologii
	Dp=D/NP;//dlugosc boku DOMENY_PROCESU
	O=procesCenter(D,NP,rowP,colP);//srodek domeny procesu
	return 0;
}

int initiateSubdomains(void){
    //wyznacza srodkowe punty subdomen, zeruje pozostale zmienne
    //przyporzadkowuje wiry do odpowiednich subdomen
    //modyfikuje zmienne globalne: SB[], OS[]
    //czyta zmienne globalne: D, Dp, NP, NSd, NV, rowP, colP, GA[], EX[]
    //
    unsigned_int i, j, k, l;//indeksy petli
    struct subdomain* sub=NULL;//tym bede pokazywal na subdomene
    //obliczenie srodkowego punktu subdomeny i zerowanie pozostalych pol
    for(i=0; i<NSd; i++){
        for(j=0; j<NSd; j++){
            sub=SB+i*NSd+j;//wskaznik do subdomeny [i,j]
            sub->O=subdomainCenter(D,NP,NSd,rowP,colP,i,j);
            //zerowanie pozostalych
            sub->G0=0;
            for(k=0; k<=NUMBER_OF_MOMENTS; k++) sub->Z[k].x=sub->Z[k].y=0;
            for(k=0; k<sub->NGs; sub->GAs[k++]=0) ;
        }
    }
    //wstawienie indeksów wirów do subdomen
    for(i=0; i<NV; i++){
    	if(!EX[i]) continue;//jezeli wir nie istnieje w DOMENIE_PROCESU to nic nie rob
        //zlokalizuj subdomene
        sub=whichSubdomain(STEP[i]);
        //wstaw wir do subdomeny         
        if(addVortexToSubdomain(sub,i)) return 1;
    }
    //wypelnienie OS[] - srodki subdomen DOMENY_SYMULACJI
    //petla po wszystkich procesach
    for(i=0; i<NP; i++)
		for(j=0; j<NP; j++){
			//dla procesu [i,j] petla po subdomenach
			for(k=0; k<NSd; k++)
				for(l=0; l<NSd; l++){
					*(OS_get(i,j,k,l))=subdomainCenter(D,NP,NSd,i,j,k,l);
				}
		}
	//
    return 0;
}

void calcMoments(unsigned_int i){
	//liczy momenty we wszystkich SUBDOMENACH procesu i przygotowuje
	//je do transferu
	//
	unsigned_int n;
	for(n=0; n<NSd*NSd; n++) calcMomentsInSubdomain(SB+n,0);
	updateSubdomainsMoments();
	return;
}

void calcMomentsInSubdomain(struct subdomain* sub, unsigned_int n){
    //oblicza momeny od 1 do NUMBER_OF_MOMENTS dla subodmeny *sub
    //dla wspolrzednych w n-tym wierszu STEP
    //czyta zmienne globalne: NV, STEP, GA
    //
    unsigned_int i, j;//indeksy petli
    //wyzerowanie starych wartosci niezaleznie czy sa wiry czy nie
    sub->G0=0; for(i=0; i<=NUMBER_OF_MOMENTS; i++) sub->Z[i].x=sub->Z[i].y=0;
    if(!sub->last) return;//jezeli nie ma w domenie wirów to wyskocz
    //
    //polozenie wiru wzgledem srodka subdomeny (zespolona) i jej kolejne potegi
    struct point z0, z;
    //n-ty wiersz ma macierzy STEP
    struct point* S=STEP+n*NV;
    //suma wirowosci w domenie oraz jej momenty - petla po wszystkich jej wirach
    for(i=0; i<sub->last; i++){
        //suma wirowosci
        sub->G0+=GA[sub->GAs[i]];   
        //wspolrzedna i-tego wiru wzgledem srodka domeny O
        //TO JUZ WYSTARCZY, BO WSPOLRZEDNA SRODKA SUBDOMENY JEST 
        //W GLOBALNYM TAK JAK WSPOLRZEDNA WIRU        
        z0.x=S[sub->GAs[i]].x-sub->O.x; z0.y=S[sub->GAs[i]].y-sub->O.y;
        z=z0;//pierwsza potega
        //liczniki momentów od 1 do NUMBER_OF_MOMENTS
        for(j=1; j<=NUMBER_OF_MOMENTS; j++){
            sub->Z[j].x+=GA[sub->GAs[i]]*z.x; sub->Z[j].y+=GA[sub->GAs[i]]*z.y;
            z=multiComplex(z,z0);//w nastepnym obiegu petli jedna potega wyzej
        }
    }
    //podzielenie przez sume wirowosci - policzenie momentów
    if(sub->G0) for(i=1; i<=NUMBER_OF_MOMENTS; i++) 
                    {sub->Z[i].x/=sub->G0; sub->Z[i].y/=sub->G0;}
    else for(i=1; i<=NUMBER_OF_MOMENTS; i++) sub->Z[i].x=sub->Z[i].y=0;
    //
    return;
}

void calcVelocities(unsigned_int i){
    //oblicza predkosci konwekcji dla polozen wirów w i-tym wierszu STEP[]
    //modyfikuje zmienne globalne: VEL[]
    //czyta zmienne globalne: NV, NSd, SB[], STEP[]
    //
    unsigned_int n, ns=NSd*NSd;
    //wyzeruj tablice predkosci
    for(n=0; n<NV; VEL[n].x=VEL[n++].y=0);
    //dodaj skladowe predkosci od bezposrednich oddzialywan wewnatrz subomen    
    for(n=0; n<ns; calcVelInSubdomain_inside(SB+n++,i)); 
    //dodaj skladowe predkosci od oddzialywan pozostalych subdomen PROCESU
    for(n=0; n<ns; calcVelInSubdomain_outside_local(SB+n++,i));
    //dodaj skladowe predkosci od oddzialywan subdomen INNYCH PROCESOW
    for(n=0; n<ns; calcVelInSubdomain_outside_global(SB+n++,i));
    return;
}

void calcVelInSubdomain_inside(struct subdomain* sub, unsigned_int i){
    //oblicza dla wirów w subdomenie *sub predkosci konwekcji wywolane
    //bezposrednim odzialywaniem wirów z tej samej subdomeny
    //wspolrzedne wirów w domenie *sub opisane sa w i-tym wierszy STEP[]     
    //wyliczone wartosci DODAJE do istnejacych wartosci w VEL[]
    //modyfikuje zmienne globalne: VEL[]
    //czyta zmienne globalne: STEP[]
    //     
    if(!sub->last) return;//jeZeli nie ma w domenie wirów, to wyskocz
    const struct point* const S=STEP+i*NV;//i-ty wiersz w STEP
    unsigned_int m, n;//indeksy petli
    real d,//kwadrat odleglosci pomiedzy wirami
         Vx,Vy,//predkosc w punkcie (sub->GAs[m]) od wiru z punkty (sub->GAs[n])
         ux,uy;//predkosc w (sub->GAs[m]) od chmury wirowej (akumulatory)
    //dla kazdego wiru w *sub
    for(m=0; m<sub->last; m++){
        ux=uy=0;//wyzeruj akumulator
        //policz wplyw od kazdego wiru z subdomeny *sub
        for(n=0; n<sub->last; n++){
            //wyznacz kwadrat odleglosci od 'm' do 'n'
             d=sqrr(S[sub->GAs[m]].x-S[sub->GAs[n]].x)+
               sqrr(S[sub->GAs[m]].y-S[sub->GAs[n]].y);
             if(d<ZERO) continue;//wiry sie pokrywaja - nie ma indukcji
             d=2*M_PI*d;//buduj dalej wyrazenie
             //policz predkosc od wiru 'n'
             Vx=GA[sub->GAs[n]]*((S[sub->GAs[m]].y-S[sub->GAs[n]].y)/d); 
             Vy=GA[sub->GAs[n]]*((S[sub->GAs[n]].x-S[sub->GAs[m]].x)/d);
             //wplyw od 'n'-tego wiru policzony - dosumuj do akumulatorów
             ux+=Vx; uy+=Vy;
        }
        //predkosc dla wiru 'm' juz jest policzona - dodajemy do tego co jest
        VEL[sub->GAs[m]].x+=ux; VEL[sub->GAs[m]].y+=uy;        
    }
    return;
}

void calcVelInSubdomain_outside_local(struct subdomain* sub0, unsigned_int i){
    //oblicza dla wirow w subdomenie *sub0 predkosci konwekcji wywolane
    //oddzialywaniem pozostalych subdomen TEGO SAMEGO PROCESU
    //wspolrzedne wirów w domenie *sub0 opisane sa w i-tym wierszy STEP[]
    //wyliczone wartosci DODAJE do istnejacych wartosci w VEL[]
    //modyfikuje zmienne globalne: VEL[]
    //czyta zmienne globalne: NSd; STEP[], SB[]
    //
    if(!sub0->last) return;//jezeli nie ma w domenie wirów, to wyskocz
    const struct point* const S=STEP+i*NV;//i-ty wiersz w STEP
    unsigned_int k,m,n,ns=NSd*NSd;//indeksy petli
    struct point z0, z,//odlegeosc wiru do srodka subdomeny, i jej potegi
                 temp;
    struct point Q,//predkosc w punkcie (sub->GAs[m]) od subdomeny *(SB+n)
                 q;//predkosci od wszystkich subdomen (akumulator)
    const struct subdomain* sub=NULL;//kolejna subdomena
    //dla kazdego wiru w *sub0
    for(m=0; m<sub0->last; m++){
        q.x=q.y=0;//wyzeruj akumulator
        //policz wplyw od pozostalych subdomen
        for(n=0; n<ns; n++){
            if((sub=SB+n)==sub0) continue;//to ta sama subdomena
            //odleglosc wiru 'm' od srodka subdomeny 'n'
            z0.x=S[sub0->GAs[m]].x-sub->O.x; z0.y=S[sub0->GAs[m]].y-sub->O.y;
            z=z0;
            //
            Q.x=1; Q.y=0;
            //suma po wszystkich momentach z domeny sub
            for(k=1; k<=NUMBER_OF_MOMENTS; k++){
                //
                temp=divComplex(sub->Z[k],z);//Z[k]/z^k
                Q.x+=temp.x; Q.y+=temp.y;
                z=multiComplex(z,z0);//kolejna potega odleglosci
            }
            temp.x=temp.y=(sub->G0)/(2*M_PI*(sqrr(z0.x)+sqrr(z0.y)));
            temp.x*=z0.y; temp.y*=z0.x;
            Q=multiComplex(temp,Q);
            Q.y=-Q.y;
            //wplyw od 'n'-tej subdomeny policzony - dosumuj do akumulatorów
            q.x+=Q.x; q.y+=Q.y;
        }
        //predkosc dla wiru 'm' juz jest policzona - dodajemy do tego co jest
        VEL[sub0->GAs[m]].x+=q.x; VEL[sub0->GAs[m]].y+=q.y;
    }
	//
    return;
}

void calcVelInSubdomain_outside_global(struct subdomain* sub0, unsigned_int i){
	//oblicza dla wirow w subdomenie *sub0 predkosci konwekcji wywolane
    //oddzialywaniem pozostalych subdomen INNYCH PROCESOW
    //wspolrzedne wirów w domenie *sub0 opisane sa w i-tym wierszy STEP[]
    //wyliczone wartosci DODAJE do istnejacych wartosci w VEL[]
    //modyfikuje zmienne globalne: VEL[]
    //czyta zmienne globalne: NP, rowP, colP, STEP[]
    //
    if(!sub0->last) return;//jezeli nie ma w domenie wirów, to wyskocz
    const struct point* const S=STEP+i*NV;//i-ty wiersz w STEP
    unsigned_int m, r_proc, c_proc, r_sub, c_sub, k;
    struct point z0, z,//odlegeosc wiru do srodka subdomeny, i jej potegi
                 temp;    
    struct point Q,//predkosc w punkcie (sub->GAs[m]) od pojedynczej SUBDOMENY
                 q;//predkosci od wszystkich SUBDOMEN (akumulator)
	struct point O_sub1;//srodek SUBDOEMNY
	real G0_sub1;//suma wirowosci w subdomenie
	struct point Z_sub1[NUMBER_OF_MOMENTS+1];//momenty SUBDOMENY
    //
    //dla kazdego wiru w *sub0
    for(m=0; m<sub0->last; m++){
        q.x=q.y=0;//wyzeruj akumulator dla 
        //policz wplyw od SUBDOMEN pozostalych procesow
   		//najpierw procesy
		for(r_proc=0; r_proc<NP; r_proc++)
			for(c_proc=0; c_proc<NP; c_proc++){
				if(r_proc==rowP && c_proc==colP) continue;//to ten sam proces
				//teraz subdomeny we wskazanym procesie
				for(r_sub=0; r_sub<NP; r_sub++)
					for(c_sub=0; c_sub<NP; c_sub++){
						//SUBDOMENA [r_sub, c_sub] PROCESU [r_proc, c_proc]
						//jej parametry
						O_sub1=*OS_get(r_proc,c_proc,r_sub,c_sub);//srodek tej subodmeny
						G0_sub1=MS_getSum(r_proc,c_proc,r_sub,c_sub);//jej suma wirowosci
						MS_getMoments(r_proc,c_proc,r_sub,c_sub,Z_sub1);//jej momenty
						//policz predkosc od tej subdomeny dla wiru (sub->GAs[m])
						//odlegosc wiru 'm' od srodka subdomeny 'n'
						z0.x=S[sub0->GAs[m]].x-O_sub1.x;
						z0.y=S[sub0->GAs[m]].y-O_sub1.y;
						z=z0;//pierwsza potega
						//
						Q.x=1; Q.y=0;
						//suma po wszystkich momentach z domeny sub1
						for(k=1; k<=NUMBER_OF_MOMENTS; k++){
							//
							temp=divComplex(Z_sub1[k],z);// Z[k]/z^k
							Q.x+=temp.x; Q.y+=temp.y;
							z=multiComplex(z,z0);//kolejna potega odleglosci
						}
						temp.x=temp.y=(G0_sub1)/(2*M_PI*(sqrr(z0.x)+sqrr(z0.y)));
						temp.x*=z0.y; temp.y*=z0.x;
						Q=multiComplex(temp,Q);
						Q.y=-Q.y;
						//dosumuj do akumulatora
						q.x+=Q.x; q.y+=Q.y;
					}
			}
		//predkosc w punkcie (sub->GAs[m]) od wszystkich pozaprocesowych
		//SUBDOMEN jest policzona - dodaj do tablicy
		VEL[sub0->GAs[m]].x+=q.x; VEL[sub0->GAs[m]].y+=q.y;
    }
	//
    return;
}

struct point newPosition(const struct point P0, const struct point dP){
    //realizuje zawijanie sie DOMENY_SYMULACJI
    //odczytuje zmienne globalne: D
    //
    struct point P1={P0.x+dP.x,P0.y+dP.y};//nowe polozenie
    while(!insideSimulationDomain(P1)){
       real D2=D/2;
       if(P1.x>D2) P1.x-=D; else if(P1.x<-D2) P1.x+=D;
       if(P1.y>D2) P1.y-=D; else if(P1.y<-D2) P1.y+=D;
    }
    return P1;
}

void convectVortexes(unsigned_int i){
     //przemieszcza wiry z polozen w i-tym wierszu STEP[] wykonujac konwekcje
     //nowe polozenie zapisuje w (i+1)-tym wierszu STEP[]
     //czyta zmienne globalne: NSd
     //
     unsigned_int n;
     for(n=0; n<NSd*NSd; convectVortexexInSubdomain(SB+n++,i));
     return;  
}

void convectVortexexInSubdomain(const struct subdomain* const sub, unsigned_int i){
     //przemieszcza wiry z subdomeny *sub z polozen w i-tym wierszu STEP[]
     //na podstawie predkosci zapisanych VEL[] (konwekcja), nowe polozenie 
     //zapisuje w (i+1)-tym wierszu STEP[]
     //modyfikuje zmienne globalne: STEP[]
     //odczytuje zmienne globalne: NV, dT, STEP[]
     //     
     if(!sub->last) return;//jezeli nie ma w domenie wirów, to wyskocz
     struct point dP;//wyliczone przesuniecie
     const struct point* const S0=STEP+i*NV;//i-ty wiersz w macierzy STEP
     struct point* const S1=STEP+(i+1)*NV;//(i+1)-ty wiersz w macierzy STEP
     //dla kazdego wiru wskazywanego przez macierz GAs[]
     unsigned_int j, n;
     for(n=0; n<sub->last; n++){
        j=sub->GAs[n];//indeks wiru, który bedzie przesuwany
        //policz przesuniecie na podstawie predkosci i czasu
        dP.x=VEL[j].x*dT; dP.y=VEL[j].y*dT;
        //zapisz nowe wspolrzedne
        S1[j]=newPosition(S0[j],dP);
     }                      
     return;
}

void diffuseVortexes(unsigned_int i){
     //przemieszcza wiry z polozen w i-tym wierszu STEP wykonujac ruchy Browna
     //w nowe polozenie zapisuje w (i+1)-tym wierszu STEP
     //dla polozenia w (i+1)-tym wierszu STEP dokonuje reorganizacji wirow
     //modyfikuje zmienne globalne: STEP[]
	 //
	 unsigned_int n;
	 for(n=0; n<NSd*NSd; diffuseVortexesInSubdomain(SB+n++,i))

     return;
}

void diffuseVortexesInSubdomain(struct subdomain* sub, unsigned_int i){
     //przemieszcza wiry z polozen w i-tym wierszu STEP wykonujac ruchy Browna
     //nowe polozenie zapisuje w (i+1)-tym wierszu STEP
     //modyfikuje zmienne globalne: STEP
     //odczytuje zmienne globalne: SB, STEP
     //
     if(!sub->last) return;//jezeli nie ma w domenie wirow, to wyskocz
     struct point dP;//wyliczone przesuniecie
     const struct point* const S0=STEP+i*NV;//i-ty wiersz w macierzy STEP
     struct point* const S1=STEP+(i+1)*NV;//(i+1)-ty wiersz w macierzy STEP    
     unsigned_int n,j; real th, r;
     for(n=0; n<sub->last; n++){
		 j=sub->GAs[n];//indeks wiru, który bedzie przesuwany
         th=2*M_PI*rand_0_1(); r=sqrtr(4*v*dT*logr(1/rand_0_1()));//kat i promien skoku w ruchu Browna
		 //przemieszczenie ruchu Browna
		 dP.x=r*cosr(th); dP.y=r*sinr(th);
         //zapisz nowe wspolrzedne
         S1[j]=newPosition(S0[j],dP);         
     }
     return;
}

int transferVortexes(unsigned_int i){	
    //dla polozenia w i-tym wierszu STEP[] dokonuje reorganizacji wirów
    //lub przygotowuje wiry do wyslania
	//czyta zmienne globalne: NP, NSd, SB[]
	//
	unsigned_int n;
	//reset struktur 'transferowych'
	for(n=0; n<NP*NP; n++){
		NVfP[n]=NVtP[n]=0;
		VfP[n].last=VtP[n].last=0;
		//ewentualnie jeszcze memset
	}
	//reorganizacja wirow
    for(n=0; n<NSd*NSd; n++)
		if(transferVortexesInSubdomain(SB+n,i)) return 1;
	//
	return 0;
}

int transferVortexesInSubdomain(struct subdomain* sub, unsigned_int i){
    //sprawdza czy wiry z macierzy sub->GAs[] sa ciagle w SUBDOMENIE *sub
    //bierze pod uwage wspolrzedne w i-tym wierszu macierzy STEP[]
    //jezeli nie to sprawdza czy jest w DOMENIE_PROCESU jezeli tak, to
    //zmiena domene wirowi, jezeli wyszedl poza domene procesu to
    //przygotowuje wir do transferu
    //modyfikuje zmienne globalne: SB[].GAs[], NVfP[](tylko dodaje), VfP[], EX[]
    //czyta zmienne globalne: NP, NV, STEP[]
    //   
    if(!sub->last) return 0;//jezeli nie ma wirow w subdomenie to wyskocz
	const struct point* const S=STEP+i*NV;//i-ty wiersz w macierzy STEP[]
	int* const E=EX+i*NV;//i-ty wiersz w macierzy EX[]
	//n pokazuje od konca wiry w sub->GAs[], j to indeks wiru
	unsigned_int j, n=sub->last, r, c, rc;

	
	do{
		j=sub->GAs[--n];
		E[j]=1;//jezeli zajdzie potrzeba to sie go usunie
	 	if(!isPointInSubdomain(sub,S[j])){
			//wyszedl poza SUBDOMENE - czy wyszedl poza DOMENE_PROCESU
			if(insideProcesDomain(S[j])){
				//jest w DOMENIE_PROCESU - zmien SUBDOMENE
				//znajdz odpowiednja SUBDOMENE i go tam dodaj
                if(addVortexToSubdomain(whichSubdomain(S[j]),j)) return 1;                
			}
			else{
				//wyszedl poza DOMENE_PROCESU - przygotuj do transferu
				//1. Okresl, ktory proces go przyjmie
				whichProcess(S[j],&r,&c); //! zwroci 1 gdy wir wyjdzie poza DOMENE_SYMULACJI
				rc=r*NP+c;                //! ale to nie powinno sie zdarzyc  
				//2. Zwieksz licznik w NVfP[]
				NVfP[rc]++;
				//3. dodaj go do VfP[] (Vortexes From Process)
				if(addVortexToV_P(VfP+rc,j,S[j])) return 1;
				E[j]=0;//usun go z tablicy egzystencji
			}
			//usun go z tablicy SUBDOMENY sub
			sub->GAs[n]= sub->GAs[--sub->last];
		}	 	
	}while(n>0);
	//
    return 0;//wszystko ok
}

int AddNewVortexes(unsigned_int i){
	//dodaje wiry znajdujace sie w strukturze VtP[] do odpowiedniej
	//SUBDOMENY PROCESU
	//w i-tym wierszu macierzy STEP[] i EX[]
	//
	struct point* const S=STEP+i*NV;//i-ty wiersz w macierzy STEP[]
	int* const E=EX+i*NV;//i-ty wiersz w macierzy EX[]
	unsigned_int j,n,m; struct point P;
	for(n=0; n<NP*NP; n++)
		for(m=0; m<VtP[n].last; m++){
			j=VtP[n].V[m].i;//globalny numer wiru
			P.x=VtP[n].V[m].x; P.y=VtP[n].V[m].y;//przepisz do punktu
			E[j]=1;//zapisz ze wir istnieje w procesie (w kroku i-tym)
			S[j]=P;//umiesc jego wspolrzedne (w kroku i-tym)
			if(addVortexToSubdomain(whichSubdomain(P),j)) return 1;	
		}
		
	return 0;	
}

real rand_0_1(void){
	//generator liczb losowych o rozkladzie równomiernym od 0 do 1
    static real p=0.5; //pierwsza wartosc to seed
    static const real shift=1.01316;
    real ipart;//miejsce na czesc calkowita
    //nastepne p
    p=powr(p+shift,5); p=modfr(p,&ipart); return p;
}

//funkcje matematyczne dzialajace na typie real

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

inline struct point multiComplex(struct point z1, struct point z2){
    //mnozenie liczb zespolonych z1*z2
    //
    struct point z0={z1.x*z2.x-z1.y*z2.y,z1.x*z2.y+z1.y*z2.x};
    return z0;
}

inline struct point divComplex(struct point z1, struct point z2){
    //dzielenie liczb zespolonych z1/z2;
    //!UWAGA: nie jest zabezpieczone przed dzieleniem przez zero
    //
    real d=z2.x*z2.x+z2.y*z2.y;
    struct point z0={(z1.x*z2.x+z1.y*z2.y)/d,(z2.x*z1.y-z1.x*z2.y)/d};
    return z0;
}

//
