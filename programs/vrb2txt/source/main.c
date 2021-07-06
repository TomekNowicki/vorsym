#include "../../vortex.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#define VERSE_BREAK 3
#define OUTPUT_FILE_EXT ".txt" 

int readVrb(const char* pathName);
int writeTxt(const char* pathName);

//zmienne globalne

//zawartosc pliku *.vrb
struct vrbHead vrbH; //the header of the .vrb file
real  *G=NULL; //vortexes' strengths
struct point *S=NULL; //vortexes positions

//uzycie:
// vrb2txt input.vrb [output] (ignoruje reszte paramtrow)
//jezeli nie jest dana nazwa pliku wynikowego to do nazwy pliku
//wejsciowego jest dodawane rozszerzenie OUTPUT_FILE_EXT

int main(int argc, char *argv[])
{ 

	printf("\nvrb2txt\n\n");

    //sprawdzenie czy jest chociaz jeden parametr wywolania
    if(argc<2){printf("!error: parameters\n"); return 1;}
    
    //okreslenie dlugosci nazwy pliku wynikowego
    size_t string_size=256;
    if(argc>=2) string_size=strlen(argv[1]);
    if(argc>=3 && strlen(argv[2])>string_size) string_size=strlen(argv[2]);
    string_size+=strlen(OUTPUT_FILE_EXT);
    char outFileName[string_size];
    
    printf("input file name: %s  (wait...)\n",argv[1]);
    
    //czytanie pliku z danymi
    if(readVrb(argv[1])){printf("!error: input file\n"); return 1;}
    
    //komunikat na ekran
    printf( "number of vortexes: %d\n"
			"number of shifts: %d\n"
			"viscosity: %f\n"
			"dT=%f\n",
			vrbH.NV,vrbH.NS,vrbH.v,vrbH.dT
		  );
			

    //nazwa pliku wynikowego
    if(argc>=3) strcpy(outFileName,argv[2]);
    else{strcpy(outFileName,argv[1]); strcat(outFileName,OUTPUT_FILE_EXT);}

    //zapisanie pliku wynikowego
    printf("output file name: %s  (wait...)\n",outFileName);
    if(writeTxt(outFileName))
        {printf("!error: output file\n"); free(G); free(S); return 1;}
    
    printf("\nvrb2txt: done\n"); 
    free(G); free(S); return 0;
}


int readVrb(const char* pathName){
	//uzywa zmiennych globalnych: vrbH, G[], S[]
	//
    FILE *vrbF=fopen(pathName,"rb"); if(vrbF==NULL) return 1;
    //przeczytanie naglowka pliku
    if(fread(&vrbH,VRBHEADSIZE,1,vrbF)!=1) goto allErrors;
    unsigned_int nvs1=vrbH.NV*(vrbH.NS+1);
    //rezerwuje pamiec
	G=(real*)calloc(vrbH.NV,REALSIZE);
	S=(struct point*)calloc(nvs1,POINTSIZE);
    if(G==NULL || S==NULL) goto allErrors;
    memset(G,0,vrbH.NV*REALSIZE);
    memset(S,0,nvs1*POINTSIZE);
    //czytanie mocy wirow
    if(fread(G,REALSIZE,vrbH.NV,vrbF)!=vrbH.NV) goto allErrors;
    if(fread(S,POINTSIZE,nvs1,vrbF)!=nvs1) goto allErrors;
    fclose(vrbF); return 0;
    allErrors:
    fclose(vrbF); free(G); free(S); G=NULL; S=NULL; return 1;
}

int writeTxt(const char* pathName){
    //reads global variables: vrbH, G, S
    //returns 1 if a problem with the file, 0 on success
    //
    FILE *vrbF=fopen(pathName,"w+t"); if(vrbF==NULL) return 1;
    unsigned_int i,j; struct point P;
    
    //zapis naglowka
    fprintf(vrbF, "number of vortexes: %d\n"
				  "number of shifts: %d\n"
				  "viscosity: %f\n"
			      "dT=%f\n",
			      vrbH.NV,vrbH.NS,vrbH.v,vrbH.dT
		   );
	//zapis wirow i ich krokow
	for(j=0; j<vrbH.NV; j++){
		fprintf(vrbF,"%d) G=%f\n",j,G[j]);
		for(i=0; i<=vrbH.NS; i++){
			if(!(i%VERSE_BREAK)) fprintf(vrbF,"\n");
			P=*(S+i*vrbH.NV+j);
			fprintf(vrbF,"[%f, %f]",P.x,P.y);
		}
		fprintf(vrbF,"\n\n");
		
	}
	//
	fclose(vrbF); return 0;
}

//
