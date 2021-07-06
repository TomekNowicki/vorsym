#include "../../vortex.h"
#include "data.h"
#include "inout.h"
#include "domain.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#define ERROR "!error:"
#define PARAMETERS_ERROR "WRONG PARAMETERS"
#define INPUT_ERROR "PROBLEM WITH THE INPUT FILE"
#define REPORT_ERROR "PROBLEM TO WRITE THE REPORT FILE"
#define OUTPUT_ERROR "PROBLEM TO WRITE THE OUTPUT FILE"

/*

parametry programu:
          1. wariant - rozpoczecie obliczen:
          vorsym input.vrt output.vrb NS D v dT
          2. wariant - kontynuacja obliczen:
          vorsym input.vrb output.vrb NS

jezeli v!=0 to NS jest podwajane, bo wtedy kazdy krok to dwa podkroki    

*/

int main(int argc, char *argv[]){

  printf("\nVorSym - slow\n\n");
  
  //przeczytanie parametrów wywolania
  if(readParameters(&argc,argv))
      {printf("%s %s\n",ERROR,PARAMETERS_ERROR); return 1;}
  
  //przeczytanie pliku z danymi
  if(readData(inputFile)){
      printf("%s %s\n",ERROR,INPUT_ERROR); return 1;
  }else{
            //zapisanie raportu - blad zapisania nie konczy programu
            char reportFile[STRINGS_CAPACITY]="";
            strcpy(reportFile,inputFile);
            changeExt(reportFile,REPORT_FILE_EXT);
            if(writeReport(reportFile)) printf("%s %s\n",ERROR,REPORT_ERROR);
  }

  //komunikat na ekran
  printf("input  file: %s\n"
         "output file: %s\n"
         "domain size: %f\n"
         "viscosity: %f\n"
         "timestep: %f\n"
         "number of vortexes: %d\n"
         "number of steps: %d\n\n",
         inputFile,outputFile,D,v,dT,NV,NS);

  //----------------------------------------------------------------------------
 
  unsigned_int i=0;//pokazuje na aktualne polozenia wirow w macierzy STEP

  time_t t0, t1, *t=NULL;//czas
  double t_step, t_total=0;//czasy obliczen dla kroku i calosciowy
  double t_start, t_stop, t_loop;
  int do_diff=(v>ZERO);//zeby nie wyliczac za kazdym razem wartosci wyrazenia

  //sumulacja n-body  
  t_start=time(t);
  
  do{
         //wykonaj NS krokow
         printf("%d convection",i+1);
         //zlap aktualny czas
         
         t0=time(t);       
         //policz predkosci dla polozenia opisanego w wierszu i macierzy STEP
         calcVelocities(i);
         //zrob konwekcje - nowe polozenia w nastepnym wierszu
         convectVortexes(i++);
         //zrób dyfuzje, jezeli byla dana lepkosc
         if(do_diff) {printf(", %d diffusion",i+1); diffuseVortexes(i++);}
         //zlap aktulany czas
         t1=time(t);
         //wyswietl czas wykonania kroku
         t_step=difftime(t1,t0);
         printf(" [%f secons]\n",t_step);
         //dosumuj do calkowitego czasu
         t_total+=t_step;

  }while(i<NS);
  
  t_stop=time(t);
  t_loop=difftime(t_stop,t_start);
  
  printf("\ntotal time: %f seconds\n",t_total);
  printf("\nstop-start: %f seconds\n",t_loop);
  
  //----------------------------------------------------------------------------
  
  //zapisanie wyników symulacji do pliku  
  if(writeVrb(outputFile)) printf("%s %s\n",ERROR,OUTPUT_ERROR);
  else printf("\noutput written to file: %s\n\n",outputFile);
  
  //zwolnienie pamieci
  freeMemory();
  
  //system("PAUSE");	
  return 0;
}

//
