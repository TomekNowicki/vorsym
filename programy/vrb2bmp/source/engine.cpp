#include "engine.h"
#include "../../vortex.h"
#include "EasyBMP.h"
#include <fstream>

using namespace std;

//zawartosc pliku .vrb
vrbHead vrbH; //naglowek
real  *G=NULL; //moce wirow
point *STEP=NULL; //pozycje wirow

//bitmapa
BMP Image;


int takeMemory(unsigned_int nv, unsigned_int ns){
	//zmienne globalne: G[], STEP[]
	//
	if(G!=NULL || STEP!=NULL) return 1;
	G=new real[nv];
    STEP=new point[nv*(ns+1)];
	if(G==NULL || STEP==NULL) {freeMemory(); return 1;}
	return 0;
}

void freeMemory(void){
	//zmienne globalne: G[], STEP[]
	//
    delete [] G;
    delete [] STEP;
    return;
}


int readVrb(const char* pathName){
    //zmienne globalne: vrbH, G[], STEP[]
    //!czytanie pliku bez sprawdzanie bledow
    //
    ifstream vrbF(pathName,ios::binary);
    unsigned_int vrbH_size_b=sizeof(vrbHead);
    vrbF.read((char*)&vrbH,vrbH_size_b);
	if(takeMemory(vrbH.NV,vrbH.NS)) {vrbF.close(); return 1;}
	unsigned_int G_size_b=vrbH.NV*(sizeof(real)), 
				 STEP_size_b=vrbH.NV*(vrbH.NS+1)*sizeof(point);
    vrbF.read((char*)G,G_size_b);
    vrbF.read((char*)STEP,STEP_size_b);
    vrbF.close();
    return 0;
}

int resetImage(int size){
	//zmienne globalne: B
	//
	if(!Image.SetSize(size,size)) return 1;
	if(!Image.SetBitDepth(8)) return 1;
	return 0;
}

int saveImage(const char* pathName){
	//
	if(!Image.WriteToFile(pathName)) return 1; else	return 0;
}


int drawVortexesForStep(unsigned_int step){
	//
	//
	if(vrbH.D<ZERO) return 1;
    RGBApixel px; px.Red=px.Green=px.Blue=0;
    point P; int xi, yi;
    real d2=vrbH.D/2, scale=(real)Image.TellWidth()/vrbH.D;
	for(unsigned_int i=0; i<vrbH.NV; i++){
		//
		P=*(STEP+step*vrbH.NV+i);
		P.x+=d2; P.y+=d2;
		P.x*=scale; P.y*=scale;
		xi=(int)P.x; yi=Image.TellWidth()-(int)P.y;
		Image.SetPixel(xi,yi,px);
	}
    return 0;
}



