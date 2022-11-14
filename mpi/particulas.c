#include <stdio.h>
#include <mpi.h>

struct Particula {
    char c;
    double d[6];
    char b[7];
};
int main(int argc, char *argv[]) {
	struct Particula vet[1000];
	int myrank;
	MPI_Status st;
	MPI_Datatype Particula_t;
	MPI_Datatype types[3] = { MPI_CHAR, MPI_DOUBLE, MPI_CHAR };
	int blocklen[3] = { 1, 6, 7 };
	MPI_Aint disp[3];
 
    MPI_Init(&argc, &argv);
 
    disp[0] = (int)(&vet[0].c) - (int)&vet[0];
    disp[1] = (int)(&vet[0].d) - (int)&vet[0];
    disp[2] = (int)(&vet[0].b) - (int)&vet[0];
    MPI_Type_create_struct(3, blocklen, disp, types, &Particula_t);
    MPI_Type_commit(&Particula_t);
 
    MPI_Comm_rank(MPI_COMM_WORLD, &myrank);
 
    if( myrank == 0 )
      MPI_Send(vet, 1000, Particula_t, 1, 123, MPI_COMM_WORLD);
    else if (myrank == 1) MPI_Recv(vet, 1000, Particula_t, 0, 123, MPI_COMM_WORLD, &st);
    MPI_Finalize();
    return 0;
}

