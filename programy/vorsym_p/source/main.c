#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <mpi.h>

#include "../../vortex.h"
#include "data.h"
#include "inout.h"
#include "domain.h"

#if (UNSIGNED_INT==INT)
	#define UNSIGNED_INT_MPI MPI_UNSIGNED
#elif (UNSIGNED_INT==LONG)
	#define UNSIGNED_INT_MPI MPI_UNSIGNED_LONG
#else
	#define UNSIGNED_INT LONG
	#define UNSIGNED_INT_MPI MPI_UNSIGNED_LONG
#endif

#if (REAL_TYPE==FLOAT)
    #define REAL_MPI MPI_FLOAT
#elif (REAL_TYPE==DOUBLE)
    #define REAL_MPI MPI_DOUBLE
#elif (REL_TYPE==LONG_DOUBLE)
    #define REAL_MPI MPI_LONG_DOUBLE
#else
    #define REAL_TYPE DOUBLE
    #define REAL_MPI MPI_DOUBLE    
#endif

#define ERROR "!error:"
#define USAGE_ERROR "vorsym_p PARAMETERS_FILE"
#define PARAMETERS_ERROR "WRONG PARAMETERS"
#define INPUT_ERROR "PROBLEM WITH THE INPUT FILE"
#define REPORT_ERROR "PROBLEM TO WRITE THE REPORT FILE"
#define OUTPUT_ERROR "PROBLEM TO WRITE THE OUTPUT FILE"
#define MEM_ERROR "PROBLEM TO ALLOCATE MEMORY"
#define INI_ERROR "PROBLEM WHEN INITIALIZING SUBDOMAINS"
#define PARAM_ERROR "PROBLEM WITH THE START PARAMETERS FILE"
#define MPI_ERROR "MPI ERROR"
#define MPI_PROC "WRONG NUMBER OF PROCESSES"


/* Plik z parametrami wywolania:

input.vrt - dane
output.vrb - wyniki
NS - liczba krokow do wykonania
NP - ilosc procesow wzdluz boku (ilosc wszystkich procesow to NP*NP)
NSd - liczba SUBDOMEN wzdluz boku DOMENY_PROCESU (ilosc wszystkich SUBODMEN to NSd*NSd)
NVs - poczatkowa pojemnosc/skok dla tablicy subdomen GAs[] (jezeli 0 to auto)
V_Ps - poczatkwa pojemnosc/skok dla tablicy V_P[] (jezeli 0 to auto) 
D - wymiar DOMENY_SYMULACJI
v - lepkosc
dT - krok czasowy
 
*/

//zapisuje indywidualny raport procesu dla kroku i
int writeProcessReport(const char* pathName, unsigned_int i);

//!UWAGA: Przy tworzeniu struktury nalezaloby sprawdzic bledy MPI

//makro tworzy strukture MPI opisujaca struct point
#define CREATE_MPI_POINT                                              \
{                                                                     \
	struct point p_temp;                                              \
	MPI_Datatype old_types[]={REAL_MPI, REAL_MPI};                    \
	int blocklens[]={1,1};                                            \
	MPI_Aint indices[]={0,0};                                         \
	MPI_Aint baseaddress; MPI_Address(&p_temp, &baseaddress);         \
	MPI_Address(&p_temp.x,&indices[0]); indices[0]-=baseaddress;      \
	MPI_Address(&p_temp.y,&indices[1]); indices[1]-=baseaddress;      \
	MPI_Type_struct(2,blocklens,indices,old_types,&MPI_point);        \
	MPI_Type_commit(&MPI_point);                                      \
}	

//makro tworzy strukture MPI opisujaca struct vortex_trans
#define CREATE_MPI_VORTEX_TRANS										  \
{																	  \
	struct vortex_trans tv_temp;									  \
	MPI_Datatype old_types[]={UNSIGNED_INT_MPI,REAL_MPI,REAL_MPI};    \
	int blocklens[]={1,1,1};                                          \
	MPI_Aint indices[]={0,0,0};                                       \
	MPI_Aint baseaddress; MPI_Address(&tv_temp, &baseaddress);        \
	MPI_Address(&tv_temp.i,&indices[0]); indices[0]-=baseaddress;     \
	MPI_Address(&tv_temp.x,&indices[1]); indices[1]-=baseaddress;     \
	MPI_Address(&tv_temp.y,&indices[2]); indices[2]-=baseaddress;     \
	MPI_Type_struct(3,blocklens,indices,old_types,&MPI_vortex_trans); \
	MPI_Type_commit(&MPI_vortex_trans);	                              \
}

//makro przeslanie momentow SUBDOMENOWYCH
#define TRANSFER_MOMENTS                                              \
{																      \
	MPI_Comm cart_rank_temp;									      \
	unsigned_int ms_p=NSd*NSd*(1+NUMBER_OF_MOMENTS*2);                \
	real *start;                                                      \
	for(i=0; i<NP; i++)											      \
		for(j=0;j<NP;j++){                                            \
			cart_cords[0]=i; cart_cords[1]=j;                         \
			MPI_Cart_rank(cart_comm,cart_cords,&cart_rank_temp);      \
			start=MS_getStart(i,j,0,0,0);                             \
			if(MPI_Bcast(start,(int)ms_p,REAL_MPI,cart_rank_temp,     \
			   cart_comm)!=MPI_SUCCESS){	                          \
				printf("%s %s\n",ERROR,MPI_ERROR); goto allErrors;    \
			}                                                         \
		}												              \
}                                                                     \

#define TRANSFER_VORTEXES 											  \
{																	  \
	MPI_Request  req_tab[npnp];									      \
	MPI_Status  stat_tab[npnp];                                       \
	for(i=0; i<npnp; i++)                                             \
		if(NVtP[i])                                                   \
			if(MPI_Irecv(VtP[i].V,NVtP[i],MPI_vortex_trans,           \
			   (int)i,1,cart_comm,&req_tab[i])!=MPI_SUCCESS){         \
				printf("%s %s\n",ERROR,MPI_ERROR); goto allErrors;    \
			}                                                         \
	for(i=0; i<npnp; i++)                                             \
		if(NVfP[i])                                                   \
			if(MPI_Send(VfP[i].V,NVfP[i],MPI_vortex_trans,            \
			   (int)i,1,cart_comm)!=MPI_SUCCESS){                     \
				printf("%s %s\n",ERROR,MPI_ERROR); goto allErrors;    \
			}                                                         \
	for(i=0; i<npnp; i++)                                             \
		if(NVtP[i])                                                   \
			if(MPI_Wait(&req_tab[i],&stat_tab[i])!=MPI_SUCCESS){      \
				printf("%s %s\n",ERROR,MPI_ERROR); goto allErrors;    \
			}                                                         \
}

//---------------------------------------------------------------------

int main(int argc, char *argv[]){
	//
	unsigned_int i,j;//indeksy petli

	//opis topologii MPI_COMM_WORLD
	int comm_world_rank=-1, comm_world_rank_size=-1;
	
	//opis topologii kartezjanskiej
	MPI_Comm cart_comm;
	int cart_rank=-1, cart_cords[2]={-1,-1};
	
	//opis MPI struct vortex_trans i struct point
	MPI_Datatype MPI_vortex_trans, MPI_point;	
	
	//wystartuj bibioteke
	if(MPI_Init(&argc, &argv)!=MPI_SUCCESS){
		printf("%s %s\n",ERROR,MPI_ERROR); return 1;
	}
	
	//sprawdz czy jest szansa, ze uzytkownik podal nazwe pliku
	if(argc<2){
		printf("%s %s\n",ERROR,USAGE_ERROR); goto allErrors;
	}
	
	//zbuduj opis MPI typu vortex_trans (wywolanie makra)
	CREATE_MPI_VORTEX_TRANS
	
	//zbuduj opis MPI typu point (wywolanie makra)
	CREATE_MPI_POINT
	
	//uzyskaj swoj rank w MPI_COMM_WORLD
	if(MPI_Comm_rank(MPI_COMM_WORLD, &comm_world_rank)!=MPI_SUCCESS){
		printf("%s %s\n",ERROR,MPI_ERROR); goto allErrors;
	}	

	if(comm_world_rank==0){
		//master
		//przeczytaj parametry startowe z pliku (modyfikacja zmiennych globalnych)
		printf("\nVorSym - parallel\n\n" );
		printf("param file: %s\n",argv[1]);
		if(readParameters(argv[1])){
			printf("%s %s\n",ERROR,PARAM_ERROR); goto allErrors;	
		};
		//przeczytaj plik, zeby ustalic ile jest wirow (modyfikajca NV)
		if(firstReadVrt(inputFile)){
			printf("%s %s\n",ERROR,INPUT_ERROR); goto allErrors;
		}
	}

	//przeslanie parametrów informacji o ilosci wirow jako brodcast
	if(MPI_Bcast(&inputFile,STRINGS_CAPACITY,MPI_CHAR,0,MPI_COMM_WORLD)!=MPI_SUCCESS){
		printf("%s %s\n",ERROR,MPI_ERROR); goto allErrors;
	}
	if(MPI_Bcast(&outputFile,STRINGS_CAPACITY,MPI_CHAR,0,MPI_COMM_WORLD)!=MPI_SUCCESS){
		printf("%s %s\n",ERROR,MPI_ERROR); goto allErrors;
	}
	if(MPI_Bcast(&NS,1,UNSIGNED_INT_MPI,0,MPI_COMM_WORLD)!=MPI_SUCCESS){
		printf("%s %s\n",ERROR,MPI_ERROR); goto allErrors;
	}
	if(MPI_Bcast(&NP,1,UNSIGNED_INT_MPI,0,MPI_COMM_WORLD)!=MPI_SUCCESS){
		printf("%s %s\n",ERROR,MPI_ERROR); goto allErrors;
	}
	if(MPI_Bcast(&NSd,1,UNSIGNED_INT_MPI,0,MPI_COMM_WORLD)!=MPI_SUCCESS){
		printf("%s %s\n",ERROR,MPI_ERROR); goto allErrors;
	}
	if(MPI_Bcast(&V_Ps,1,UNSIGNED_INT_MPI,0,MPI_COMM_WORLD)!=MPI_SUCCESS){
		printf("%s %s\n",ERROR,MPI_ERROR); goto allErrors;
	}	
	if(MPI_Bcast(&D,1,REAL_MPI,0,MPI_COMM_WORLD)!=MPI_SUCCESS){
		printf("%s %s\n",ERROR,MPI_ERROR); goto allErrors;
	}
	if(MPI_Bcast(&v,1,REAL_MPI,0,MPI_COMM_WORLD)!=MPI_SUCCESS){
		printf("%s %s\n",ERROR,MPI_ERROR); goto allErrors;
	}
	if(MPI_Bcast(&dT,1,REAL_MPI,0,MPI_COMM_WORLD)!=MPI_SUCCESS){
		printf("%s %s\n",ERROR,MPI_ERROR); goto allErrors;
	}
	if(MPI_Bcast(&NV,1,UNSIGNED_INT_MPI,0,MPI_COMM_WORLD)!=MPI_SUCCESS){
		printf("%s %s\n",ERROR,MPI_ERROR); goto allErrors;
	}	
	
	
	//zapytaj o ilosc procesow
	if(MPI_Comm_size(MPI_COMM_WORLD, &comm_world_rank_size)!=MPI_SUCCESS){
		printf("%s %s\n",ERROR,MPI_PROC); goto allErrors;
	}
	//sprawdz czy ilosc procesow jest odpowiednia do dekompozycji zadania
	if(comm_world_rank_size!=(NP*NP)){
		 printf("%s %s\n",ERROR,MPI_PROC); goto allErrors;
	}

	//zrob wirtualna topologie z zawijaniem
	{
		int dim_size[]={NP,NP}, periods[]={1,1};
		if(MPI_Cart_create(MPI_COMM_WORLD,2,dim_size,periods,1,&cart_comm)!=MPI_SUCCESS){
			printf("%s %s\n",ERROR,MPI_ERROR); goto allErrors;
		}
		//uzyskaj swoj (liniowy) rank w wirtualnej topoligii
		if(MPI_Comm_rank(cart_comm, &cart_rank)!=MPI_SUCCESS){
			printf("%s %s\n",ERROR,MPI_ERROR); goto allErrors;
		}	
		//uzyskaj swoje wspolrzedne w wirtualnej topologii
		if(MPI_Cart_coords (cart_comm,cart_rank, 2, cart_cords)!=MPI_SUCCESS){
			printf("%s %s\n",ERROR,MPI_ERROR); goto allErrors;
		}		
	}
	
	//ustal wartosci zmiennym globalnym opisujacym DOMENE_PROCESU
	initiateProcesDomain(cart_cords[0], cart_cords[1]);
	
	//rezerwacja pamieci na podstawowe dane procesu
	if(takeMemory_0(NV,NS)){
		printf("%s %s\n",ERROR,MEM_ERROR); goto allErrors;
	};
	
	//przeczytanie pliku z danymi - czytanie pliku na dzielonym zasobie	
	if(readVrt(inputFile)){
		printf("%s %s\n",ERROR,INPUT_ERROR); goto allErrors;
	};
	
	//automatyczne ustalenie NVs - na podstawie ilosci wirow w DOMENIE_PROCESU
	if(!NVs){
		unsigned_int nvp=0;
		for(i=0; i<NV; i++)	if(EX[i]) nvp++;
		NVs=nvp/(NSd*NSd)+1;
	}
	
	//automatyczne ustalenie V_Ps - na podstawie NVs
	if(!V_Ps) V_Ps=NVs/8+1;
	
	//pobranie pamieci na subdomeny
	if(takeMemory_1(NSd,NVs,NP)){
		printf("%s %s\n",ERROR,MEM_ERROR); goto allErrors;
	};

	//inicjalizacja subdomen
	if(initiateSubdomains()){
		printf("%s %s\n",ERROR,MEM_ERROR); goto allErrors;
	}

	//komunikat na ekran
	if(comm_world_rank==0)
		printf("input  file: %s\n"
			   "output file: %s\n"
               "domain size: %f\n"
               "number of processes: %d x %d = %d\n"
               "number of subdomains in one process: %d x %d = %d  total:%d\n"
               "viscosity: %f\n"
               "timestep: %f\n"
               "total number of vortexes: %d\n"
               "number of steps: %d\nprocess\tNVs\tV_Ps\tnumber of vortexes\n",
                inputFile,outputFile,D,NP,NP,NP*NP,NSd,NSd,NSd*NSd,NP*NP*NSd*NSd,v,dT,NV,NS
              );
    MPI_Barrier(cart_comm);    
    printf("[%d, %d]\t%d\t%d\n",rowP,colP,NVs,V_Ps);
    
    
	//SYMULACJA
	MPI_Barrier(cart_comm);

	unsigned_int k=0, npnp=NP*NP;
	int do_diff=(v>ZERO);//zeby nie wyliczac za kazdym razem wartosci wyrazenia
	time_t t0, t1, *t=NULL;//czas
	double t_start, t_stop, t_loop;
	double t_step, t_total=0;//czasy obliczen dla kroku i calosciowy

	t_start=time(t);
	
	while(k<NS){		

		if(comm_world_rank==0) printf("step: %d ",k);
		
		//zlap aktualny czas
        t0=time(t);  
		
		calcMoments(k);//na podstawie wspolrzednych w wierszu k macierzy STEP[]
		MPI_Barrier(cart_comm);
		TRANSFER_MOMENTS //(wywolanie makra)
		calcVelocities(k);//na podstawie wspolrzednych w wierszu k macierzy STEP[]	
		convectVortexes(k);//z wiersza k do wiersza k+1
		
		//przeorganizuj i przygotuj wiry do przeslania
		//bierze pod uwage nowe polozenie, tj. wiersz k+1 macierzy STEP[]
		if(transferVortexes(k+1)){
			printf("%s %s\n",ERROR,MEM_ERROR); goto allErrors;
		}
		//przeslij/odbierz informacje o ilosci wirow do transferu
		for(i=0; i<npnp; i++)
			if(MPI_Scatter(NVfP,1,UNSIGNED_INT_MPI,NVtP+i,1,UNSIGNED_INT_MPI,i,cart_comm)!=MPI_SUCCESS){
				printf("%s %s\n",ERROR,MPI_ERROR); goto allErrors;
			}
		//wiem juz ile mam wyslac i ile od ktorego dostane
		//sprawdzam czy mam miejsce na nowe wiry
		if(checkSizeV_P(VtP, NVtP, npnp)){
			printf("%s %s\n",ERROR,MEM_ERROR); goto allErrors;
		}
 
 		//pora wysylac i odbierac

		{
			MPI_Request  req_tab[npnp];
			MPI_Status  stat_tab[npnp];
			for(i=0; i<npnp; i++)
				if(NVtP[i])
					if(MPI_Irecv(VtP[i].V,NVtP[i],MPI_vortex_trans,(int)i,1,cart_comm,&req_tab[i])!=MPI_SUCCESS){
						printf("%s %s\n",ERROR,MPI_ERROR); goto allErrors;
					}
			for(i=0; i<npnp; i++)
				if(NVfP[i])
					if(MPI_Send(VfP[i].V,NVfP[i],MPI_vortex_trans,(int)i,1,cart_comm)!=MPI_SUCCESS){
						printf("%s %s\n",ERROR,MPI_ERROR); goto allErrors;
					}
			for(i=0; i<npnp; i++)
				if(NVtP[i])
					if(MPI_Wait(&req_tab[i],&stat_tab[i])!=MPI_SUCCESS){
						printf("%s %s\n",ERROR,MPI_ERROR); goto allErrors;
					}
		}
		
        //teraz sobie te wiry dodaj do subdomen
		AddNewVortexes(k+1);
		
		
		if(do_diff){
			//jezeli jest krok dyfuzyjny to zrob dyfuzje
			k++;
			diffuseVortexes(k);//w wierszu k+1 sa natepne polozenia wirow
			//przeorganizuj i przygotuj wiry do przeslania
			//bierze pod uwage nowe polozenie, tj. wiersz k+1 macierzy STEP[]
			if(transferVortexes(k+1)){
				printf("%s %s\n",ERROR,MEM_ERROR); goto allErrors;
			}
					//przeslij/odbierz informacje o ilosci wirow do transferu
			for(i=0; i<npnp; i++)
				if(MPI_Scatter(NVfP,1,UNSIGNED_INT_MPI,NVtP+i,1,UNSIGNED_INT_MPI,i,cart_comm)!=MPI_SUCCESS){
					printf("%s %s\n",ERROR,MPI_ERROR); goto allErrors;
				}
			//wiem juz ile mam wyslac i ile od ktorego dostane
			//sprawdzam czy mam miejsce na nowe wiry
			if(checkSizeV_P(VtP, NVtP, npnp)){
				printf("%s %s\n",ERROR,MEM_ERROR); goto allErrors;
			}
			
			//pora wysylac i odbierac
			{
				MPI_Request  req_tab[npnp];
				MPI_Status  stat_tab[npnp];
				for(i=0; i<npnp; i++)
					if(NVtP[i])
						if(MPI_Irecv(VtP[i].V,NVtP[i],MPI_vortex_trans,(int)i,1,cart_comm,&req_tab[i])!=MPI_SUCCESS){
							printf("%s %s\n",ERROR,MPI_ERROR); goto allErrors;
						}
				for(i=0; i<npnp; i++)
					if(NVfP[i])
						if(MPI_Send(VfP[i].V,NVfP[i],MPI_vortex_trans,(int)i,1,cart_comm)!=MPI_SUCCESS){
							printf("%s %s\n",ERROR,MPI_ERROR); goto allErrors;
						}
				for(i=0; i<npnp; i++)
					if(NVtP[i])
						if(MPI_Wait(&req_tab[i],&stat_tab[i])!=MPI_SUCCESS){
							printf("%s %s\n",ERROR,MPI_ERROR); goto allErrors;
						}
			}
		
			//teraz sobie te wiry dodaj do subdomen
			AddNewVortexes(k+1);
		}//koniec dyfuzji
		
		//zlap aktulany czas
        t1=time(t);
		
		//wyswietl czas wykonania kroku
        t_step=difftime(t1,t0);
        
        if(comm_world_rank==0) printf(" [%f secons]\n",t_step);
        //dosumuj do calkowitego czasu
        t_total+=t_step;
				
		//writeProcessReport(inputFile, k+1);
		k++;
	}
	
	t_stop=time(t);
	t_loop=difftime(t_stop,t_start);
	
	
	//bariera po zakonczeniu symulacji
	MPI_Barrier(cart_comm);	
	
	
	if(comm_world_rank==0){
		printf("\ntotal time: %f seconds\n",t_total);
		printf("\nstop-start: %f seconds\n",t_loop);
	}

	//zapis do pliku *.vrb
	//najpierw pierwszy proces - tworzy plik z naglowkiem i wirami

	if(comm_world_rank==0){
		//
		printf("Creating the output file: %s\n",outputFile);
		if(createVrb(outputFile)){
			printf("%s %s\n",ERROR,OUTPUT_ERROR); goto allErrors;
		}
	    printf("Writing vortexes\n");	    
	}
	
	MPI_Barrier(cart_comm);	//bo plik musi byc utworzony	

//	if(writeVortexes(outputFile)){
//		printf("%s %s\n",ERROR,OUTPUT_ERROR); goto allErrors;
//	}

	//komunikat o zakonczeniu
	{
		char procName[STRINGS_CAPACITY]=""; int nameLen=0;
		if(MPI_Get_processor_name(procName,&nameLen)!=MPI_SUCCESS){
			printf("%s %s\n",ERROR,MPI_ERROR); goto allErrors;
		}
		printf("%d) %s [%d, %d]: done\n",cart_rank,procName,rowP,colP);
	}
		
	freeMemory_1(NP,NSd); freeMemory_0();
	MPI_Finalize();
	return 0;	
	
	allErrors: 
	freeMemory_1(NP,NSd); freeMemory_0(); 
	MPI_Abort(MPI_COMM_WORLD,MPI_ERR_UNKNOWN); MPI_Finalize();
	return 1;
}

//---------------------------------------------------------------------

int writeProcessReport(const char* pathName, unsigned_int i){
	//raport - kazdy proces to oddzielny plik
	//i to numer wiersza w macierzy STEP[]
	//
	char raportFile[STRINGS_CAPACITY]="", tmp_int[STRINGS_CAPACITY]="";
	//dodanie do nazwy pliku identyfikatorow
	strcpy(raportFile,pathName);
	strcat(raportFile,"_proc");	
	sprintf(tmp_int,"%d",rowP); strcat(raportFile,tmp_int);
	sprintf(tmp_int,"%d",colP); strcat(raportFile,tmp_int);
	strcat(raportFile,"_step");	
	sprintf(tmp_int,"%d",i);	strcat(raportFile,tmp_int);
	strcat(raportFile,REPORT_FILE_EXT);
    //
    return writeReport(raportFile,i);
}
