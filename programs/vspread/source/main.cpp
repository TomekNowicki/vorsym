#include <cstdlib>
#include <iostream>
#include <fstream>
#include <ctime>
#include <cmath>

/*
    vspread -program do generowania losowych wirów
    generator losowy jest inicjalizowany czasem
    u¿ycie programu: vspread B N Gmin Gmax fileName
    gdzie:
    D -dlugosc boku kwadratowej domeny
    N -ilosc przedzialow na ktore nalezy podzielic bok
    (Gmin, Gmax) -przedzial wirowosci
    fileName -nazwa pliku
    
    Domena jest kwadratowa. Srodek ukladu wspolrzednych znajduje sie w srodku
    kwadratu, a osie kwadratu sa równolegle do jego boków. Domena zostaje
    podzielona na NxN cel w kazda cele wkladany jest losowo wir o losowej mocy.

*/

using namespace std;

//rownomierny generator liczb losowych
class Rnd{
      double x_3, x_2, x_1;
      const double d;
      int error;//jezeli 0 to nie ma bledu
public:
      Rnd(void);
      double next(double from=0, double to=1);
      int ok(void) const {return !error;}
};//class rnd


//wir na plaszczyznie
class Vortex2D{
public:
       double x, y, G;
};//class Vortex

ostream& operator<<(ostream& s, const Vortex2D& V){
    s<<V.x<<" "<<V.y<<" "<<V.G<<"\n";
    return s;
}

//
int main(int argc, char *argv[])
{
    cout<<"--\nvspread\n\n";
     
    if(argc!=6){
        cout<<"(!) 5 parameters are needed\n";
        return 1;
    }
    
    //dlugosc boku kwadrata
    double B;
    //ilosc przedzialow na boku B
    unsigned long N;
    //maksymalna i minimalna wartosc wirowosci
    double Gmin, Gmax;
    //nazwa pliku wynikowego
    char* fileName;
    
    //przeczytanie parametrów wywo³ania programu    
    B=atof(argv[1]); N=atol(argv[2]);
    Gmin=atof(argv[3]); Gmax=atof(argv[4]);
    fileName=argv[5];
    if(B<=0 || N<=0 || Gmin>Gmax){
        cout<<"(!) Incorrect parameters\n";
        return 1;
    }
  
    cout<<"the domain's span: "<<B<<"\n";
    cout<<"number of cells: " <<N<<"x"<<N<<"\n";
    cout<<"vortexes strength range: ("<<Gmin<<", "<<Gmax<<")\n";



    Vortex2D V;//zmienna pomocnicza
    Rnd R;//generator liczb losowych

    if(!R.ok()){cout<<"(!) problem with UTC time\n"; return 1;}
    double x=-B/2.0, y=x, dx=B/(double)N, dy=dx;
    ofstream vrtF(fileName);
    if(!vrtF){cout<<"(!) Cannot create a file\n"; return 1;}

    cout<<"Generating...\n";    

    for(unsigned long i=0; i<N; i++){
        //zewnetrzna petla pokazuje wiersze zaczynajac od dolu
        x=-B/2.0;
        for(unsigned long j=0; j<N; j++){
            //wewnêtrzna pêtla przebiega wiersz zaczynajac z lewej
            V.x=R.next(x,x+dx); V.y=R.next(y,y+dy); V.G=R.next(Gmin,Gmax);
            //wyrzucenie wiru na ekran
            //cout<<V;
            //zapis wiru do pliku
            vrtF<<V;
            //przesuniecie w prawo
            x+=dx;
        }
        //przesuniecie do góry
        y+=dy;
    }
    vrtF.close();
    
    cout<<"\n"<<N*N<<" vortexes were generated\n";
    cout<<"Saved to file: "<<fileName<<"\n--\n";


	return 0;    

}

//

Rnd::Rnd(void):error(0),d(pow(2.0,35.0)-5),x_1(0),x_2(0),x_3(0){
    //inicjalizowanie generatora mikroczasem; styl "c"
    time_t T; tm* UTC=NULL;
    time(&T);//pobranie aktualnego czasu
    UTC= gmtime(&T);//przeksztalcenie czasu na UTC
     if(UTC==NULL) {error=1; return;}
    //w mianowniku pojawiaja sie stale, zeby nie mogl sie wyzerowac
    x_1=(double)T/((double)UTC->tm_sec*(double)UTC->tm_yday +0.02334);
    x_2=(double)UTC->tm_hour*(double)UTC->tm_mon/((double)UTC->tm_sec+1.8364);
    x_3=1000.0*(double)UTC->tm_year/((double)UTC->tm_min +0.49938);
}

double Rnd::next(double from, double to){
    //wyznaczanie kolejnej losowej liczby
    //obliczam kolejno wartosc ze wzoru Marsaglia
    double x=fmod(1176.0*x_1+1476.0*x_2+1776.0*x_3,d);
    //przesuwam wszystko
    x_3=x_2; x_2=x_1; x_1=x;
    //przeskalowuje na przedzial (0, 1)
    x/=(d-1);
    //zwracam ta wartosc jezeli uzytkownik nie zadal przedzialu
    if(from==0 && to==1) return x;
    //sprawdzam czy podany przedzial ma sens, jezeli nie to zwroci from
    if(to<from) return from;
    //przeskalowuje na przedzial (form, to)
    return from+x*(to-from);
	return 0;
}

//
