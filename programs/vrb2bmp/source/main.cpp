#include "engine.h"
#include <iostream>
#include <string>
#include <sstream>

using namespace std;


//uzycie: input.vrb bitmap_size output


int main(int argc, char* argv[]){
	
	cout<<"\nvrb2txt\n";
	
		
	if(argc!=4)
		{cout<<"!error: parameters\n"; return 1;}
		
	if(readVrb(argv[1]))
		{cout<<"!error: input file "<<argv[1]<<"\n"; return 1;}
	
	int bitmap_size=atoi(argv[2]);
	string bmpFile;
	ostringstream step;
	
	int ilosc_cyfr=0, ns=vrbH.NS, ilosc_zer;
	while(ns>0) {ilosc_cyfr++; ns=ns/10;}
	

	for(unsigned_int i=0; i<=vrbH.NS; i++){
		//przygotuj nazwe pliku
		step.str("");
		if(i<100) step<<0;
		if(i<10) step<<0;		
		step<<i;
	
		bmpFile=argv[3]+step.str()+".bmp";
		cout<<bmpFile<<"\n";
		//zrob bitmape
		resetImage(bitmap_size);
		drawVortexesForStep(i);
		//zapisz bitmape do pliku
		saveImage(bmpFile.c_str());	
	}

	freeMemory();
	
	return 0;
}
