#include "inout.h"

#include "../../vortex.h" 
#include "data.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int strEnd(const char* str1, const char* str2){
    //sprawdza czy str1 konczy sie na str2 (sprawdzanie rozszerzen plikow)
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

int takeMemory(unsigned_int nv, unsigned_int ns){
    //alokuje pamiec dla nv wirów i dla ns krokow;
    //modyfikuje zmienne globalne: GA, VEL, STEP
    //zwraca 1 gdy wystapil problem z alokacja pamieci, 0 gdy wszystko ok
    //
    freeMemory();//tak na wszelki wypadek
    unsigned_int nvs1=nv*(ns+1);
    GA=(real*)calloc(nv,REALSIZE);
    VEL=(struct point*)calloc(nv,POINTSIZE);
    STEP=(struct point*)calloc(nvs1,POINTSIZE);
    if(GA==NULL || VEL==NULL || STEP==NULL) goto memError;
    memset(GA,0,nv*REALSIZE);
    memset(VEL,0,nv*POINTSIZE);
    memset(STEP,0,nvs1*POINTSIZE);
    return 0;
    memError: freeMemory(); return 1;
}

int freeMemory(void){
    //zwalnia pamiec; modyfikuje zmienne globalne: GA, VEL, STEP
    free(GA); free(VEL); free(STEP);//uwolni pamieæ jezeli nie sa to NULL-e
    GA=NULL; VEL=NULL; STEP=NULL;
    return 0;//nie spodziewam sie zadnego bledu
}

int readParameters(const int* argc, char* const argv[]){
    //wczytuje parametry programu do struktur danych w dwóch wariantach
    //modyfikuje zmienne globalne: inputFile, outputFile, NS, D, v, dT    
    //
    //musza byc co najmniej 4 parametry
    if(*argc<4) return 1;
    long temp_l=0; double temp_d=0;   
    //sprawdzam czy sciezki do plików zmieszcza sie w stringach
    if( (strlen(argv[1])+1)>STRINGS_CAPACITY &&
        (strlen(argv[2])+1)>STRINGS_CAPACITY 
                                                ) return 1;
    //mieszcza sie, wiec kopiuje nazwy plików
    strcpy(inputFile ,argv[1]); strcpy(outputFile,argv[2]);
    //czy nazwa pliku wejociowego ma odpowiednie rozszerzenie
    if(!strEnd(inputFile,TEXT_DATAFILE_EXT) &&
       !strEnd(inputFile, BIN_DATAFILE_EXT)     ) return 1;
    //czy nazwa pliku wyjsciowego ma odpowiednie rozszerzenie
    if(!strEnd(outputFile,BIN_DATAFILE_EXT)) return 1;       
    //ilosc kroków do wykonania - niezalezne od rodzaju pliku 
    temp_l=atol(argv[3]); if(temp_l>=1) NS=temp_l; else return 1;
    //
    //je¿eli jest to plik binarny to ju¿ wszystko mam
    //
    //je¿eli jest to plik tekstowy to wczytujê wiecej: D, v, dT
    if(strEnd(inputFile,TEXT_DATAFILE_EXT)){
        //
        //musi byc co najmniej 7 parametrów
        if(*argc<7) return 1;          
        //wymiar kwadratowej domeny
        temp_d=atof(argv[4]); D=(real)temp_d;
        //lepkosc
        temp_d=atof(argv[5]); v=(real)temp_d;
        //krok czasowy
        temp_d=atof(argv[6]); dT=(real)temp_d;
        //kontrola sensownosci danych
        if(D<=0 || v<0 || dT<=0) return 1;
        //jezeli v!=0 to kazdy krok to dwa podkroki
        if(v>ZERO) NS*=2;  
    }
    //
    return 0;
}

int readData(const char *pathName){
    //wczytuje poczatkowe polozenie wirów do wewnetrznych struktur danych
    //format pliku wejsciowego rozpoznaje po rozszrzeniu *.vrt lub *.vrb
    //modyfikuje zmienne globalne: NV, NS, GA, STEP
    //
    if(strEnd(inputFile,TEXT_DATAFILE_EXT)) return readVrt(pathName);
    if(strEnd(inputFile,BIN_DATAFILE_EXT)) return readVrb(pathName);
    return 1;//rozszerzenie nazwy plkiu nie jest takie jak trzeba
}

int readVrt(const char *pathName){
    //czyta plik tekstowy z polozeniami i mocami wirów
    //modyfikuje zmienne globalne: NV, GA, STEP
    //odczytuje zmienne globalne: NS, D
    //zwraca 1-problem z plikiem; 2-problem z pamiecia; 0-wszystko OK
    //
    FILE *vrtF=fopen(pathName,"r+t"); if(vrtF==NULL) return 1;
    struct point P;
    //pierwsze przeczytanie pliku - liczenie ilosci wirow w domenie
    float x,y,g;
    NV=0;
    while(fscanf(vrtF,"%f %f %f",&x,&y,&g)!=EOF){
        P.x=(real)x; P.y=(real)y;
        //sprawdzam czy punkt jest wewnatrz domeny obliczeniowej
        if(insideDomain(P)) NV++;
    }
    //wiem ile jest wirów, w NS jest ile jest krokow - rezerwuje pamiec
    if(takeMemory(NV,NS)){fclose(vrtF); return 2;}
    //drugie czytanie pliku - wypelnianie struktury danych
    //zakladam, ze plik nie zmienil sie
    rewind(vrtF);
    unsigned_int i=0;
    while(fscanf(vrtF,"%f %f %f",&x,&y,&g)!=EOF){
        P.x=(real)x; P.y=(real)y;
        if(insideDomain(P)){GA[i]=g; *(STEP+i)=P; i++;}
    }
    //
    fclose(vrtF); return 0;
}

int readVrb(const char *pathName){
    //czyta plik binarny do kontynuacji symulacji, tzn. wczytuje ostatnie
    //polozena wirow
    //modyfikuje zmienne globalne: NV, D, v, dT,  GA, STEP
    //odczytuje zmienne globalne: NS
    //zwraca 1-problem z plikiem, 2-problem z pamiecia, 0-wszystko OK
    //
    FILE *vrbF=fopen(pathName,"r+b"); if(vrbF==NULL) return 1;
    //przeczytanie nag³ówka pliku
    struct vrbHead vrbH;
    if(fread(&vrbH,VRBHEADSIZE,1,vrbF)!=1) goto fileError;
    //ilosc wirow, wymiar domeny, lepkosc, krok czasowy
    NV=vrbH.NV; D=vrbH.D; v=vrbH.v; dT=vrbH.dT;
    //jezeli v!=0 to kazdy krok to dwa podkroki
    if(v>ZERO) NS*=2;     
    //ilosc kroków w pliku *.vrb
    unsigned_int ns=vrbH.NS;
    //wiem ile jest wirów, w NS jest ile jest krokow - rezerwuje pamiec
    if(takeMemory(NV,NS)){fclose(vrbF); return 2;}    
    //przeczytanie mocy wirów
    if(fread(GA,REALSIZE,NV,vrbF)!=NV) goto fileError;
    //przeczytenie ostatnich polozen wirow
    //ominiecie wczesniejszych polozen
    long offset=NV*ns*POINTSIZE;//tutaj jest konwersja-moze sie nie zmiescic
    if(fseek(vrbF,offset,SEEK_CUR)) goto fileError;
    //przeczytanie ostatnich polozen
    if(fread(STEP,POINTSIZE,NV,vrbF)!=NV) goto fileError;
    //    
    fclose(vrbF); return 0;
    fileError: fclose(vrbF); freeMemory(); return 1;
}

int writeVrb(const char *pathName){
    //zapisuje wszystkie kroki i wiry do pliku .vrb
    //odczytuje zmienne globalne: NV, NS, GA, STEP
    //zwraca 1 gdy jest problem z operacja dyskow¹, 0 gdy wszystko OK
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

int writeReport(const char *pathName){
    //zapisuje do pliku tekstowego wartosci odczytane z wewnetrznych struktur
    //danych; odczytuje zmienne globalne: NV, GA, STEP
    //zwraca 1, gdy jest problem z operacja dyskowa, 2 jezeli nie ma danych 
    //do zapisania, 0 gdy wszystko OK
    //    
    if(GA==NULL || STEP==NULL) return 2;
    unsigned_int i; FILE *rptF; if((rptF=fopen(pathName,"w+t"))==NULL) return 1;    
    for(i=0; i<NV; i++){
        if(fprintf(rptF,"%d)\tx=%f\ty=%f\tG=%f\n",i,(STEP+i)->x,(STEP+i)->y,GA[i])<0)
        goto fileError;
    }
    close(rptF); return 0;
    fileError: close(rptF); remove(pathName); return 1;
}

//
