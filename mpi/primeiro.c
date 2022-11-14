#include <stdio.h>
#include <mpi.h>

int main(int argc, char** argv) {
  int worldSize, myRank, aux;
  char computador[MPI_MAX_PROCESSOR_NAME];

  MPI_Init(NULL, NULL);  // Inicialização

  MPI_Comm_size(MPI_COMM_WORLD, &worldSize); // Quantos processos envolvidos?
  MPI_Comm_rank(MPI_COMM_WORLD, &myRank); // Meu identificador

  MPI_Get_processor_name(computador, &aux);

  printf("Estou executando no computador %s, meu rank %d de um total de %d processos\n",
         computador, myRank, worldSize);

  MPI_Finalize(); // Finalização
}

