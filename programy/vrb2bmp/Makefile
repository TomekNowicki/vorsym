all: EasyBMP.o engine.o main.o
	g++ ./object/EasyBMP.o ./object/engine.o ./object/main.o -o ./bin/vrb2bmp
main.o: ../vortex.h ./source/main.cpp
	g++ -c ./source/main.cpp -o ./object/main.o
engine.o: ../vortex.h ./source/engine.h ./source/engine.cpp
	g++ -c ./source/engine.cpp -o ./object/engine.o
EasyBMP.o: ./source/EasyBMP.h ./source/EasyBMP.cpp
	g++ -c ./source/EasyBMP.cpp -o ./object/EasyBMP.o
clean:
	rm ./object/EasyBMP.o
	rm ./object/engine.o
	rm ./object/main.o
	rm ./bin/vrb2bmp
