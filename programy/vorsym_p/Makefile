all: data.o inout.o domain.o main.o
	mpicc -lm ./object/data.o ./object/inout.o ./object/domain.o ./object/main.o -o ./bin/vorsym_p
main.o: ./source/main.c
	mpicc -c ./source/main.c -o ./object/main.o
domain.o: ../vortex.h ./source/domain.h ./source/domain.c
	cc -c ./source/domain.c -o ./object/domain.o
inout.o: ../vortex.h ./source/inout.h ./source/inout.c
	cc -c ./source/inout.c -o ./object/inout.o
data.o: ../vortex.h ./source/data.h ./source/data.c
	cc -c ./source/data.c -o ./object/data.o
clean:
	rm ./object/data.o
	rm ./object/inout.o
	rm ./object/domain.o
	rm ./object/main.o
	rm ./bin/vorsym_p
