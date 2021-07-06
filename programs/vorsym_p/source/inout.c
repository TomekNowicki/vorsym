#include "inout.h"
#include "../../vortex.h" 
#include "data.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int strEnd(const char* str1, const char* str2){
    //sprawdza czy str1 konczy sie na str2 (sprawdzanie rozszerzen plików)
    //nie uzywa zmiennych globalnych
    //
    char* p=strstr(str1,str2);
    if(!p) return 0; //str2 nie wystepuje w str1
    //wystepuje - sprawdzam czy to jest koniec
    size_t n=0; while(*(p++)!='\0') n++;
    if(n==strlen(str2)) return 1; else return 0;
}

int changeExt(char* pathName, const char* ext){
    //zmienia rozszerzenie pliku na ext - nalezy podac ".ext" jako parametr
    //wczesniej uzyc strEnd, zeby sprwdzic czy zmiana bedzie ok
    //nie uzywa zmiennych globalnych
    //
    //ostatnie wystapienie kropki
    char *p_d=strrchr(pathName,'.'); //d-destination
    const char *p_s=ext; //s-source
    if(!p_d) return 0; //nie ma kropki - cos nie tak
    while(*p_d!='\0' || *p_s!='\0') *(p_d++)=*(p_s++);
    //oba powinny dojsc do konca jednoczesnie
    return 1;
}

int takeMemory_0(unsigned_int nv, unsigned_int ns){
    //alokuje pamiec dla nv wirów i dla ns kroków
    //alokuje tablice GA[], VEL[], STEP[], EX[] (zmienne globalne)
    //zwraca 1 gdy wystapil problem z alokacja pamieci, 0 - ok
    //
    //!zalozenie: GA, VEL, STEP, EX ==NULL
    unsigned_int nvs1=nv*(ns+1);//rozmiar tablicy STEP[] i EX[]
    GA=(real*)calloc(nv,REALSIZE);
    VEL=(struct point*)calloc(nv,POINTSIZE);
    STEP=(struct point*)calloc(nvs1,POINTSIZE);
    EX=(int*)calloc(nvs1,sizeof(int));
    if(GA==NULL || VEL==NULL || STEP==NULL || EX==NULL) goto memError;
    memset(GA,0,nv*REALSIZE);
    memset(VEL,0,nv*POINTSIZE);
    memset(STEP,0,nvs1*POINTSIZE);
    memset(EX,0,nvs1*sizeof(int));
    return 0;
    memError: freeMemory_0(); return 1;
}

int freeMemory_0(void){
    //zwalnia pamiec globalnych tablic
    free(GA); free(VEL); free(STEP); free(EX);
    GA=NULL; VEL=NULL; STEP=NULL; EX=NULL;
    return 0;//nie spodziewam sie zadnego bledu
}

int takeMemory_1(unsigned_int nsd, unsigned_int nvs, unsigned_int np){
	// (...)
	//
    unsigned_int i, tab_size=NSd*NSd;
    //alokacja tablicy SB[]
    SB=(struct subdomain*)calloc(tab_size,SUBDOMAINSIZE);
    if(SB==NULL) return 1;//nie ma pamieci nawet na tablice subdomen
    memset(SB,0,tab_size*SUBDOMAINSIZE);//wypelnij to wszystko zerami
    for(i=0; i<tab_size; SB[i++].GAs=NULL);//subdomeny jeszcze nie maja tablic
    //alokacja wewnetrznych tablic SB->GAs[]
    for(i=0; i<tab_size; i++){
        if((SB[i].GAs=(unsigned_int*)calloc(NVs,UNINTSIZE))==NULL) goto memError;
        memset(SB[i].GAs,0,NVs);//wypelnij wszystko zerami
        SB[i].NGs=NVs;//ustaw domenie jej poczatkowy rozmiar tablicy
        SB[i].last=0;//pierwsze wolne miejsce w tablicy
    }    
    //alokacja tablicy MS[]
    tab_size=np*np*nsd*nsd*(1+NUMBER_OF_MOMENTS*2);
    MS=(real*)calloc(tab_size,REALSIZE);
    if(MS==NULL) goto memError;//nie udalo sie zglos blad
    memset(MS,0,tab_size*REALSIZE);
    //alokacja tablicy OS[]
    tab_size=np*np*nsd*nsd;
    OS=(struct point*)calloc(tab_size,POINTSIZE);
    if(OS==NULL) goto memError;//nie udalo sie zglos blad
    memset(OS,0,tab_size*POINTSIZE);
    //alokacja NVfP[]
    tab_size=np*np;
    NVfP=(unsigned_int*)calloc(tab_size,UNINTSIZE);
    if(NVfP==NULL) goto memError;
    memset(NVfP,0,tab_size*UNINTSIZE);
    //alokacja NVtP[]
    //tab_size bez zmian
    NVtP=(unsigned_int*)calloc(tab_size,UNINTSIZE);
    if(NVtP==NULL) goto memError;
    memset(NVtP,0,tab_size*UNINTSIZE);    
    //alokacja VfP[]
    //tab_size bez zmian
    VfP=(struct v_p_handle*)calloc(tab_size,VPHANDLESIZE);
    if(VfP==NULL) goto memError;
    for(i=0; i<tab_size; i++){VfP[i].last=0; VfP[i].cap=V_Ps; VfP[i].V=NULL;}
    for(i=0; i<tab_size; i++){
		VfP[i].V=(struct vortex_trans*)calloc(V_Ps,VORTEXTRANSSIZE);
		if(VfP[i].V==NULL) goto memError;
		memset(VfP[i].V,0,V_Ps*VORTEXTRANSSIZE);
    }
    //alokajca VtP[]
    //tab_size bez zmian
    VtP=(struct v_p_handle*)calloc(tab_size,VPHANDLESIZE);
    if(VtP==NULL) goto memError;
    for(i=0; i<tab_size; i++){VtP[i].last=0; VtP[i].cap=V_Ps; VtP[i].V=NULL;}
    for(i=0; i<tab_size; i++){
		VtP[i].V=(struct vortex_trans*)calloc(V_Ps,VORTEXTRANSSIZE);
		if(VtP[i].V==NULL) goto memError;
		memset(VtP[i].V,0,V_Ps*VORTEXTRANSSIZE);
    }    
	//
    return 0;//wszystko sie udalo
    memError: freeMemory_1(np,nsd); return 1;//blad
}

int freeMemory_1(unsigned_int np,unsigned_int nsd){
	//zwolniene pamieci globalnych tablic
	//
    unsigned_int i=0;
    if(SB!=NULL) for(i=0; i<nsd*nsd; free(SB[i++].GAs)); free(SB); SB=NULL; 
    free(MS); MS=NULL;
    free(OS); OS=NULL;
    free(NVfP); NVfP=NULL;
    free(NVtP); NVtP=NULL;
    if(VfP!=NULL) for(i=0; i<np*np; free(VfP[i++].V)); free(VfP); VfP=NULL;
    if(VtP!=NULL) for(i=0; i<np*np; free(VtP[i++].V)); free(VtP); VtP=NULL; 
	//	
    return 0;
}

int readParameters(const char* pathName){
    //wczytuje parametry wywolana z pliku o podanej nazwie
    //modyfikuje zmienne globalne: inputFile[], outputFile[], NS, NP NSd, NVs, V_Ps, D, v, dt    
    //!UWAGA: nie ma sprawdzania poprawnosci
    //
    FILE* prmF=fopen(pathName,"r+t");
    if(prmF==NULL) return 1;
    float D_temp=0, v_temp=0, dT_temp=0;
    if(fscanf(prmF,"%s%s%d%d%d%d%d%f%f%f",
				  inputFile,
				  outputFile,
				  &NS,
				  &NP,
				  &NSd,
				  &NVs,
				  &V_Ps,
				  &D_temp,
				  &v_temp,
				  &dT_temp
			 )==EOF) goto fileError;
	//
	D=(real)D_temp;
	v=(real)v_temp;
	dT=(real)dT_temp;
	//jezeli v!=0 to kazdy krok to dwa podkroki
    if(v>ZERO) NS*=2;      
	//
    fclose(prmF); return 0;
    fileError: fclose(prmF); return 1;
}

int firstReadVrt(const char *pathName){
	//czyta startowy plik tekstowy z polozeniami i mocami wirów
	//w celu ustalenia ilosci wirwo w domenie czyli NV
    //modyfikuje zmienne globalne: NV
    //zwraca 1-problem z plikiem; 0-wszystko OK
    //!UWAGA: nie ma sprawdzania poprawnosci
    //
    FILE *vrtF=fopen(pathName,"r+t"); if(vrtF==NULL) return 1;
    struct point P; float x,y,g;
    NV=0;//wyzerowanie akumulatora
    //przeczytanie pliku - liczenie ilosci wirów w domenie
    while(fscanf(vrtF,"%f %f %f",&x,&y,&g)!=EOF){
        P.x=(real)x; P.y=(real)y;
        //sprawdzam czy punkt jest wewnatrz domeny obliczeniowej
        if(insideSimulationDomain(P)) NV++;
    }
    //
    fclose(vrtF); return 0;
}

int readVrt(const char *pathName){
    //czyta plik tekstowy z polozeniami i mocami wirów
    //modyfikuje zmienne globalne: GA, STEP, EX
    //zwraca 1-problem z plikiem; 0-wszystko OK
    //!UWAGA: nie ma sprawdzania poprawnosci    
    //
    FILE *vrtF=fopen(pathName,"r+t"); if(vrtF==NULL) return 1;
    struct point P; float x,y,g;
    unsigned_int i=0;//odliczanie wiru
    while(fscanf(vrtF,"%f %f %f",&x,&y,&g)!=EOF){
        //
        P.x=(real)x; P.y=(real)y;
        //sprawdzam czy punkt jest wewnatrz CALEJ_DOMENY
        if(insideSimulationDomain(P)) GA[i]=g; else continue;
		//sprawdzam czy punkt jest wewnatrz DOMENY_PROCESU
        if(insideProcesDomain(P)) {EX[i]=1; STEP[i]=P;} //pierwszy wiersz w EX[] i STEP[]
        i++;
    }
    //
    fclose(vrtF); return 0;
}

int createVrb(const char *pathName){
	//tworzy plik wynikowy z odpowiednim naglowkiem i mocami wirow
	//ale z zerowymi krokami
	//czyta zmienne globalne: NV, NS, v, dT, D, GA[]
	FILE *vrbF=fopen(pathName, "w+b"); if(vrbF==NULL) return 1; 		
	struct vrbHead vrbH ={NV,NS,v,dT,D};
	if(fwrite(&vrbH,VRBHEADSIZE,1,vrbF)!=1)  goto fileError;
	if(fwrite(GA,REALSIZE,NV,vrbF)!=NV) goto fileError;
	//"puste" kroki	
	unsigned_int i; struct point P_temp={0,0};
	for(i=0; i<NV*(NS+1); i++)
		if(fwrite(&P_temp,POINTSIZE,1,vrbF)!=1) goto fileError;
	//
	fclose(vrbF); return 0;
    fileError: fclose(vrbF); remove(pathName); return 1;
}

int writeVortexes(const char *pathName){
	//
	//
	FILE *vrbF=fopen(pathName, "r+b"); if(vrbF==NULL) return 1;
	unsigned_int i, offset=VRBHEADSIZE+NV*REALSIZE;
	if(fseek(vrbF,offset,SEEK_SET)) goto fileError;
    for(i=0; i<NV*(NS+1); i++){
		//fwrite(&P,POINTSIZE,1,vrbF);
		if(EX[i]){ if(fwrite(&STEP[i],POINTSIZE,1,vrbF)!=1) goto fileError;}
		else{if(fseek(vrbF,POINTSIZE,SEEK_CUR)) goto fileError;}
    }
	//
	fclose(vrbF); return 0;
    fileError: fclose(vrbF); remove(pathName); return 1;
}

/* int writeVrb(const char *pathName){
    //zapisuje wszystkie kroki i wiry do pliku .vrb
    //odczytuje zmienne globalne: NV, NS, GA, STEP
    //zwraca 1 gdy jest problem z operacj¹ dyskow¹, 0 gdy wszystko OK
    //    
    FILE *vrbF=fopen(pathName,"w+b"); if(vrbF==NULL) return 1;    
    struct vrbHead vrbH ={NV,NS,v,dT,D};
    if(fwrite(&vrbH,VRBHEADSIZE,1,vrbF)!=1)  goto fileError;
    if(fwrite(GA,REALSIZE,NV,vrbF)!=NV) goto fileError;
    unsigned_int nvs1=NV*(NS+1);
    if(fwrite(STEP,POINTSIZE,nvs1,vrbF)!=nvs1) goto fileError;
    fclose(vrbF); return 0;
    fileError: fclose(vrbF); remove(pathName); return 1;
}
*/


int writeReport(const char *pathName, unsigned_int step){
	//...
	//kazdy program odzielny plik, dotyczy wierszy 'step' macierzy
	//STEP[] i EX[]
    //
           
    FILE *rptF; if((rptF=fopen(pathName,"w+t"))==NULL) return 1;    
	unsigned_int i,j,k, //indeksy petli
				 n, //indeks wiru
				 sum; //akumulator
	
	const struct point* const S=STEP+NV*step;//wiersz nr 'step' w STEP[]
	const int* const E=EX+NV*step;//wiersz nr 'step' w EX[]
	struct subdomain *sub=NULL;//do pokazywania na odpowiednia subdomena
	
	//informacje o DOMENIE_PROCESU
	if(fprintf(rptF,"PROCESS [%d, %d] O=(%f, %f) NSd=%d\n",rowP,colP,O.x,O.y,NSd)<0) goto fileError;
    if(fprintf(rptF,"total number of vortexes: %d\n\n",countVortexesInProces(step))<0) goto fileError;
    for(i=0; i<NV; i++) if(E[i]) if(fprintf(rptF,"%d ",i)<0) goto fileError;
    
    //informacje o SUBDOMENACH
    if(fprintf(rptF,"\n\n%dx%d=%d SUBDOMAINS\n",NSd,NSd, NSd*NSd)<0) goto fileError;
	sum=0;
    for(i=0; i<NSd; i++){
        for(j=0; j<NSd; j++){
            sub=SB+i*NSd+j;//wskaznik do subdomeny [i,j]
            //naglowek subdomeny
            if(fprintf(rptF,"\n%d) subdomain [%d, %d] ",(i*NSd+j),i,j)<0) goto fileError;
            if(fprintf(rptF,"O=(%f, %f) NGs=%d last=%d number of vortexes: %d\n",
					   sub->O.x,sub->O.y,sub->NGs,sub->last,sub->last)<0) goto fileError;
            //indeksy wirow w subdomenie
            if(fprintf(rptF,"GAs[ ")<0) goto fileError;
            for(k=0; k<sub->last; k++) if(fprintf(rptF,"%d ",sub->GAs[k])<0) goto fileError;
            if(fprintf(rptF,"]\n")<0) goto fileError;
			//wartosci momentow w subdomenie
			if(fprintf(rptF,"G0=%f ",sub->G0)<0) goto fileError;
			for(k=1; k<=NUMBER_OF_MOMENTS; k++)
				if(fprintf(rptF,"Z%d=(%f, %f) ",k,sub->Z[k].x,sub->Z[k].y)<0) goto fileError;
			if(fprintf(rptF,"\n")<0) goto fileError;
            //wiry w subdomenie
            for(k=0; k<sub->last; k++){
               n=sub->GAs[k]; sum++;
               if(fprintf(rptF,"%d)%d\tx=%f\ty=%f\tG=%f\tV=[%f, %f]\n",
						  k,n,S[n].x,S[n].y,GA[n],VEL[n].x,VEL[n].y)<0) goto fileError;
            }            
        }
    }
    //calkowita liczba wirów    
    if(fprintf(rptF,"\ntotal number of vortexes in SUBDOMAINS: %d\n",sum)<0) goto fileError;                
    
    /*
    //teraz MS[liczba po liczbie];
	fprintf(rptF,"\n-------------MS\n");    
    for(i=0; i<NP*NP*NSd*NSd*(1+NUMBER_OF_MOMENTS*2); i++) fprintf(rptF,"%f\n",MS[i]);
    
    
    //teraz OS[liczba po liczbie];
	fprintf(rptF,"\n-------------OS\n");
    for(i=0; i<NP*NP*NSd*NSd; i++) fprintf(rptF,"(%f,%f)\n",OS[i].x,OS[i].y);
    
    */
    
    //teraz NVfP[] liczba po liczbie
    
    
    
    	
    fprintf(rptF,"\n------NVfP\n");
    for(i=0; i<NP*NP; i++) fprintf(rptF,"%d   ",NVfP[i]);
    fprintf(rptF,"\n");
    
    //teraz NVtP[] liczba po liczbie
    fprintf(rptF,"\n-------------NVtP\n");
    for(i=0; i<NP*NP; i++) fprintf(rptF,"%d   ",NVtP[i]);
    fprintf(rptF,"\n\n");
    
    //teraz VfP[]
    fprintf(rptF,"vfP[]\n");
    for(i=0; i<NP*NP; i++){
		fprintf(rptF,"%d) cap=%d last=%d\n",i,VfP[i].cap,VfP[i].last);
		for(j=0; j<VfP[i].last; j++)
			fprintf(rptF,"\t%d) %d (%f,%f) \n",
				    j, VfP[i].V[j].i, VfP[i].V[j].x,VfP[i].V[j].y );
    }
    
    
    fprintf(rptF,"\n\n");
    //teraz VfP[]
    fprintf(rptF,"vtP[]\n");
    for(i=0; i<NP*NP; i++){
		fprintf(rptF,"%d) cap=%d last=%d\n",i,VtP[i].cap,VtP[i].last);
		for(j=0; j<VtP[i].last; j++)
			fprintf(rptF,"\t%d) %d (%f,%f) \n",
			        j, VtP[i].V[j].i, VtP[i].V[j].x,VtP[i].V[j].y );
    }    
    
    
    close(rptF); return 0;
    fileError: close(rptF); remove(pathName); return 1;    
}


int addVortexToSubdomain(struct subdomain* sub, unsigned_int i){
    //dodaje indeks wiru do subdomeny *sub, w razie potrzeby powieksza pojemnosc
    //indeks odnosi sie do macierzy GA[], VEL[], STEP[]
    //czyta zmienna globalna: NVs
    //
    if(sub==NULL) return 1;//nie wskazano dobrej domeny
    //sprawdz czy jest miejsce
    if((sub->last)>=(sub->NGs)){//moze sie zdarzc jedyne, ze jest rowne
        //nie ma juz miejsca - powieksz tablice
        unsigned_int NGs_new=sub->NGs+NVs;//pojemnosc nowej tablicy
        unsigned_int *GAs_new=(unsigned_int*)calloc(NGs_new,UNINTSIZE);
        if(GAs_new==NULL) return 1;//brakuje pamieci -wyskocz
        //przepisanie wartosci do nowej tablicy
        unsigned_int j; for(j=0; j<sub->NGs; GAs_new[j]=sub->GAs[j++]);
        free(sub->GAs); sub->GAs=NULL;//zwolnienie pamieci
        (sub->GAs)=GAs_new; (sub->NGs)=NGs_new;//nowa wieksza tablica
        //last pokazuje dobre miejsce
    }    
    //tablica juz jest wystarczajaco duza - wstaw liczbe i przesuc wskaznik
    sub->GAs[sub->last++]=i;
    return 0;
}

inline int increaseV_P(struct v_p_handle* h){
	//zwieksza pojemnosc tablicy pokazywanej przez uchwyt h
	//czyta zmienne globalne: V_Ps
	//
	unsigned_int cap_new=h->cap+V_Ps;//pojemnosc nowej tablicy
	struct vortex_trans *V_new =(struct vortex_trans*)calloc(cap_new,VPHANDLESIZE);
	if(V_new==NULL) return 1;
	unsigned_int j; for(j=0; j<h->last; V_new[j]=h->V[j++]);
	free(h->V);h->V=V_new; h->cap=cap_new;
	//last pokazuje dobre miejsce
	return 0;
}

int addVortexToV_P(struct v_p_handle* h, unsigned_int i, struct point P){
	//i-numer wiru, P- wspolrzedne wiru
	//
	if(h==NULL) return 1;//nie wskazano dobrego uchwytu
	//sprawdz czy jest miejsce, jezeli nie ma, to powieksz
	if((h->last)>=(h->cap)) increaseV_P(h);	
	//tablica jest wystarczajaco duza- wstaw i przesun wskaznik
	h->V[h->last].i=i; h->V[h->last].x=P.x; h->V[h->last].y=P.y;
	h->last++;
	return 0;
}


int checkSizeV_P(struct v_p_handle* H, unsigned_int* N, unsigned_int n){
	//przygotowuje tablice na przyjcecie wirow
	//
	unsigned_int i;
	for(i=0; i<n; i++){
		while(H[i].cap<N[i])
			if(increaseV_P(&H[i])) return 1;
		H[i].last=N[i];
	}
	return 0;
}

unsigned_int countVortexesInProces(unsigned_int step){
	//liczy ile jest wirow w procesie w wiersu nr 'step' macierzy EX[]
	//
	const int* const E=EX+NV*step;
	unsigned_int i, s=0;
	for(i=0; i<NV; i++) if(E[i]) s++;
	return s;
}

//
