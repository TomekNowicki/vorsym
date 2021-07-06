#include "inout.h"

#include "../../vortex.h" 
#include "data.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>


//

int strEnd(const char* str1, const char* str2){
    //sprawdza czy str1 ko�czy si� na str2 (sprawdzanie rozszerze� plik�w)
    //nie u�ywa zmiennych globalnych
    //
    char* p=strstr(str1,str2);
    if(!p) return 0; //str2 nie wyst�puje w str1
    //wyst�puje - sprawdzam czy to jest koniec
    size_t n=0; while(*(p++)!='\0') n++;
    if(n==strlen(str2)) return 1; else return 0;
}

int changeExt(char* pathName, const char* ext){
    //zmienia rozszerzenie pliku na ext - nale�y poda� ".ext" jako parametr
    //wcze�niej u�yc strEnd, �eby sprwdzi� czy zmiana bedzie ok
    //nie u�ywa zmiennych globalnych
    //
    //ostatnie wystapienie kropki
    char *p_d=strrchr(pathName,'.'); //d-destination
    const char *p_s=ext; //s-source
    if(!p_d) return 0; //nie ma kropki - co� nie tak
    while(*p_d!='\0' || *p_s!='\0') *(p_d++)=*(p_s++);
    //oba powinny doj�� do konca jednoczesnie
    return 1;
}

int takeMemory(unsigned_int nv, unsigned_int ns){
    //alokuje pamiec dla nv wir�w i dla ns krok�w;
    //modyfikuje zmienne globalne: GA, VEL, STEP
    //zwraca 1 gdy wyst�pi� problem z alokacj� pamieci, 0 gdy wszystko ok
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
    free(GA); free(VEL); free(STEP);//uwolni pamie� je�eli nie s� to NULL-e
    GA=NULL; VEL=NULL; STEP=NULL;
    return 0;//nie spodziewam si� �adnego b��du
}

int readParameters(const int* argc, char* const argv[]){
    //wczytuje parametry programu do struktur danych w dw�ch wariantach
    //modyfikuje zmienne globalne: inputFile, outputFile, NS, D, v, dT    
    //
    //musz� by� co najmniej 4 parametry
    if(*argc<6) return 1;
    long temp_l=0; double temp_d=0;   
    //sprawdzam czy scie�ki do plik�w zmieszcz� si� w stringach
    if( (strlen(argv[1])+1)>STRINGS_CAPACITY &&
        (strlen(argv[2])+1)>STRINGS_CAPACITY 
                                                ) return 1;
    //mieszcz� si�, wiec kopiuj� nazwy plik�w
    strcpy(inputFile ,argv[1]); strcpy(outputFile,argv[2]);
    //czy nazwa pliku wej�ciowego ma odpowiednie rozszerzenie
    if(!strEnd(inputFile,TEXT_DATAFILE_EXT) &&
       !strEnd(inputFile, BIN_DATAFILE_EXT)     ) return 1;
    //czy nazwa pliku wyj�ciowego ma odpowiednie rozszerzenie
    if(!strEnd(outputFile,BIN_DATAFILE_EXT)) return 1;       
    //ilosc krok�w do wykonania - niezale�ne od rodzaju pliku 
    temp_l=atol(argv[3]); if(temp_l>=1) NS=temp_l; else return 1;
    //ilo�� przedzia��w na boku domeny
    temp_l=atol(argv[4]); if(temp_l>=1) NSd=temp_l; else return 1;
    //pocz�tkowa pojemno�� subdomeny
    temp_l=atol(argv[5]); if(temp_l>=1) NVs=temp_l; else NVs=0;
    //
    //je�eli jest to plik binarny to ju� wszystko mam
    //
    //je�eli jest to plik tekstowy to wczytuj� wiecej: D, v, dT
    if(strEnd(inputFile,TEXT_DATAFILE_EXT)){
        //
        //musi by� co najmniej 7 parametr�w
        if(*argc<9) return 1;          
        //wymiar kwadratowej domeny
        temp_d=atof(argv[6]); D=(real)temp_d;
        //lepko��
        temp_d=atof(argv[7]); v=(real)temp_d;
        //krok czasowy
        temp_d=atof(argv[8]); dT=(real)temp_d;
        //kontrola sensowno�ci danych
        if(D<=0 || v<0 || dT<=0) return 1;
        //je�eli v!=0 to ka�dy krok to dwa podkroki
        if(v>ZERO) NS*=2;  
    }
    //
    return 0;
}

int readData(const char *pathName){
    //wczytuje pocz�tkowe po�o�enie wir�w do wewn�trznych struktur danych
    //format pliku wej�ciowego rozpoznaje po rozszrzeniu *.vrt lub *.vrb
    //modyfikuje zmienne globalne: NV, NS, GA, STEP
    //
    if(strEnd(inputFile,TEXT_DATAFILE_EXT)) return readVrt(pathName);
    if(strEnd(inputFile,BIN_DATAFILE_EXT)) return readVrb(pathName);
    return 1;//rozszerzenie nazwy plkiu nie jest takie jak trzeba
}

int readVrt(const char *pathName){
    //czyta plik tekstowy z po�o�eniami i mocami wir�w
    //modyfikuje zmienne globalne: NV, GA, STEP
    //odczytuje zmienne globalne: NS, D
    //zwraca 1-problem z plikiem; 2-problem z pami�ci�; 0-wszystko OK
    //
    FILE *vrtF=fopen(pathName,"r+t"); if(vrtF==NULL) return 1;
    struct point P;
    //pierwsze przeczytanie pliku - liczenie ilo�ci wir�w w domenie
    float x,y,g;//? tutaj nie uda�o mi si� zrobi� real
    NV=0;
    while(fscanf(vrtF,"%f %f %f",&x,&y,&g)!=EOF){
        P.x=(real)x; P.y=(real)y;
        //sprawdzam czy punkt jest wewn�trz domeny obliczeniowej
        if(insideDomain(P)) NV++;
    }
    //wiem ile jest wir�w, w NS jest ile jest krok�w - rezerwuj� pamie�
    if(takeMemory(NV,NS)){fclose(vrtF); return 2;}
    //drugie czytanie pliku - wype�nianie struktury danych
    //zak�adam, �e plik nie zmieni� si�
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
    //po�o�ena wir�w
    //modyfikuje zmienne globalne: NV, D, v, dT,  GA, STEP
    //odczytuje zmienne globalne: NS
    //zwraca 1-problem z plikiem, 2-problem z pamiecia, 0-wszystko OK
    //
    FILE *vrbF=fopen(pathName,"r+b"); if(vrbF==NULL) return 1;
    //przeczytanie nag��wka pliku
    struct vrbHead vrbH;
    if(fread(&vrbH,VRBHEADSIZE,1,vrbF)!=1) goto fileError;
    //ilo�� wir�w, wymiar domeny, lepko��, krok czasowy
    NV=vrbH.NV; D=vrbH.D; v=vrbH.v; dT=vrbH.dT;
    //????? tutaj mo�e kontrola poprawno�ci danych
    
    //je�eli v!=0 to ka�dy krok to dwa podkroki
    if(v>ZERO) NS*=2;     
    //ilo�� krok�w w pliku *.vrb
    unsigned_int ns=vrbH.NS;
    //wiem ile jest wir�w, w NS jest ile jest krok�w - rezerwuj� pamie�
    if(takeMemory(NV,NS)){fclose(vrbF); return 2;}    
    //przeczytanie mocy wir�w
    if(fread(GA,REALSIZE,NV,vrbF)!=NV) goto fileError;
    //przeczytenie ostatnich po�o�e� wir�w
    //omini�cie wcze�niejszych po�o�e�
    long offset=NV*ns*POINTSIZE;//tutaj jest konwersja-mo�e si� nie zmie�ci�
    if(fseek(vrbF,offset,SEEK_CUR)) goto fileError;
    //przeczytanie ostatnich polo�e�
    if(fread(STEP,POINTSIZE,NV,vrbF)!=NV) goto fileError;
    //    
    fclose(vrbF); return 0;
    fileError: fclose(vrbF); freeMemory(); return 1;
}

int writeVrb(const char *pathName){
    //zapisuje wszystkie kroki i wiry do pliku .vrb
    //odczytuje zmienne globalne: NV, NS, GA, STEP
    //zwraca 1 gdy jest problem z operacj� dyskow�, 0 gdy wszystko OK
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
    //zapisuje do pliku tekstowego wartosci odczytane z wewn�trznych struktur
    //danych; odczytuje zmienne globalne: NV, GA, STEP
    //zwraca 1, gdy jest problem z operacj� dyskow�, 2 je�eli nie ma danych 
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
//funkcje dodane do wersji quick
//

int takeMemoryForSubdomains(void){
    //rezerwuje pami�� na subdomeny i ka�dej subodmienie tablic� wir�w
    //modyfikuje zmienne globalne: SB
    //odczytuje zmienne globalne: NSd, NVs
    //za�o�enia: NVs>=1
    //
    freeSubdomainsMemory();//je�eli co� ju� jest to wyczy��
    unsigned_int i, sd=NSd*NSd;//ilo�� subdomen
    //rezerwacja pami�ci na tabic� subdomen
    SB=(struct subdomain*)calloc(sd,SUBDOMAINSIZE);
    if(SB==NULL) return 1;//nie ma pami�ci nawet na tablic� subdomen
    memset(SB,0,sd*SUBDOMAINSIZE);//wype�nij to wszystko zerami
    for(i=0; i<sd; SB[i++].GAs=NULL);//subdomeny jeszcze nie maj� tablic
    //rezerwacja wewn�trznych tablic subdomen
    for(i=0; i<sd; i++){
        if((SB[i].GAs=(unsigned_int*)calloc(NVs,UNINTSIZE))==NULL) goto memError;
        memset(SB[i].GAs,0,NVs);//wype�nij wszystko zerami
        SB[i].NGs=NVs;//ustaw domenie jej pocz�tkowy rozmiar tablicy
        SB[i].last=0;//pierwsze wolne miejsce w tablicy
    }    
    return 0;
    memError: freeSubdomainsMemory(); return 1;
}

int freeSubdomainsMemory(void){
    //zwolnienie pamieci zarezerwowanej przez subdomeny
    //modyfikuje zmienne globalne: SB
    //czyta zmienne globalne: NSd, NVs
    //
    if(SB==NULL) return 1;//nic nie zwolni�, bo si� nie da�o
    else{
         //zwolnienie pamie� wewn�trznych tablic ka�dej subdomeny
         unsigned_int i, sd=NSd*NSd;       
         for(i=0; i<sd; i++) {
             free(SB[i].GAs);
             SB[i].GAs=NULL; SB[i].NGs=0; SB[i].last=0;//zupe�nie bez potrzeby
         }
    }         
    //zwolnienie tablicy subdomen
    free(SB); SB=NULL; return 0;
}

int writeReport2(const char *pathName){
    //jeszcze komentarz co to jest n
    
    //zapisuje do pliku tekstowego wartosci odczytane z wewn�trznych struktur
    //danych; odczytuje zmienne globalne: NSd, NV, GA, STEP, D, NSd
    //zwraca 1, gdy jest problem z operacj� dyskow�, 2 je�eli nie ma danych 
    //do zapisania, 0 gdy wszystko OK
    //    
    if(GA==NULL || STEP==NULL || SB==NULL) return 2;
    FILE *rptF; if((rptF=fopen(pathName,"w+t"))==NULL) return 1;    
    unsigned_int i, j, k, n, s=0; struct subdomain *sub=NULL;
    //
    for(i=0; i<NSd; i++){
        for(j=0; j<NSd; j++){
            //subdomena [i,j]
            sub=SB+i*NSd+j;//wska�nik do tej subdomeny
            //nag��wek subdomeny
            if(fprintf(rptF,"%d) subdomain [%d, %d]\n",(i*NSd+j),i,j)<0)
                goto fileError;
            if(fprintf(rptF,"O=(%f, %f) NGs=%d last=%d\n",
                       sub->O.x, sub->O.y, sub->NGs, sub->last)<0)
                goto fileError;
            //indeksy wir�w w subdomenie
            if(fprintf(rptF,"GAs[ ")<0) goto fileError;
            for(k=0; k<sub->last; k++)
                if(fprintf(rptF,"%d ",sub->GAs[k])<0) goto fileError;
            if(fprintf(rptF,"]\n")<0) goto fileError;
           //warto�ci moment�w w subdomenie
           if(fprintf(rptF,"G0=%f ",sub->G0)<0) goto fileError;
           for(k=1; k<=NUMBER_OF_MOMENTS; k++)
               if(fprintf(rptF,"Z%d=(%f, %f) ",k,sub->Z[k].x,sub->Z[k].y)<0) goto fileError;
           if(fprintf(rptF,"\n")<0) goto fileError;
           //wiry w subdomenie
           //
           //tutaj u�ywam wsp�rz�dnych z pierwszego kroku
           
           for(k=0; k<sub->last; k++){
               n=sub->GAs[k]; s++;
               if(fprintf(rptF,"%d)\tx=%f\ty=%f\tG=%f\tV=[%f, %f]\n",
                          k,(STEP+n)->x,(STEP+n)->y,GA[n],VEL[n].x,VEL[n].y)<0) goto fileError;
           }
           if(fprintf(rptF,"\n")<0) goto fileError;
        }
    }
    //ca�kowita liczba wir�w    
    if(fprintf(rptF,"\n--\ntotal number of vortexes: %d\n",s)<0) goto fileError;                
    close(rptF); return 0;
    fileError: close(rptF); remove(pathName); return 1;
}

int addVortex(struct subdomain* sub, unsigned_int i){
    //dodaje indeks wiru do subdomeny *sub, w razie potrzeby powi�ksza pojemno��
    //indeks odnosi si� do macierzy GA[], VEL[], STEP[]
    //czyta zmienn� globaln�: NVs
    //
    //sprawd� czy jest miejsce
    if(sub==NULL) return 1;
    if((sub->last)>=(sub->NGs)){//mo�e si� zdarzy� jedynie, �e jest r�wne
        //nie ma ju� miejsca - powieksz tablic�
        unsigned_int NGs_new=sub->NGs+NVs;//pojemno�� nowej tablicy
        unsigned_int *GAs_new=(unsigned_int*)calloc(NGs_new,UNINTSIZE);
        if(GAs_new==NULL) return 1;//brakuje pami�ci -wyskocz
        //przepisanie warto�ci do nowej tablicy
        unsigned_int j; for(j=0; j<sub->NGs; GAs_new[j]=sub->GAs[j++]);
        free(sub->GAs); sub->GAs=NULL;//zwolnienie pami�ci
        (sub->GAs)=GAs_new; (sub->NGs)=NGs_new;//nowa wi�ksza tablica
        //last pokazuje dobre miejsce
    }    
    //tablica ju� jest wystarczaj�co du�� - wstaw liczb� i przesu� wska�nik
    sub->GAs[sub->last++]=i;
    return 0;
}

//
