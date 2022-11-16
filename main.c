#include "headers.h"

int main(int argc, char * argv[]) {
    double start, end;
    start = omp_get_wtime();

    int num_threads = omp_get_max_threads();

    int frames_chunk = frames_total/num_threads;


    MPI_Init(NULL, NULL);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

    printf("processos %d\n", world_size);

    printf("mpi rodando no processo: %d \n", my_rank);
/*
    já que reclamaram do commit em inglês:

    o ranque zero é o mestre. Nesse caso, ele faz aquela parte inicial de leitura do arquivo,
    e manda tudo pro ranque 1. O ranque 1 executa tudo como se fosse um único processador.
    Esse código só funfa se rodar com 2 processos no mpirun

    Tem que arrumar pra delegar entre mais processos
    e de forma que o ranque 0 não fique a toa o tempo todo

*/
    uint8_t (*raw_frames)[height][width] = NULL;
    uint8_t (*reference_frame)[width] = NULL;
    if(my_rank == 0){
        raw_frames = calloc(frames_total, sizeof(*raw_frames));
        #pragma omp parallel for 
            for (int i = 0; i < num_threads; i++) 
            {
                read_file(
                    raw_frames, 
                    i * frames_chunk,
                    i * frames_chunk + frames_chunk
                );
            }

        for (int f = 1; f < world_size; f++){
            int target = f;
            printf("trying to send frame zero to proccess %d \n", f);
            MPI_Send(raw_frames[0], width * height, MPI_UINT8_T, target, 0, MPI_COMM_WORLD);
            printf("sent frame 0 to target %d \n",  target);
        }
    }

    
    
    if(my_rank != 0){
        reference_frame = calloc(height, sizeof(*reference_frame));
        MPI_Status status;
        int ierr = MPI_Recv(reference_frame, width * height, MPI_UINT8_T, 0, 0, MPI_COMM_WORLD, &status);
        if (ierr != MPI_SUCCESS){
            printf("erro no envio no proccess %d \n", ierr);
        }
    } else {
        reference_frame = raw_frames[0];
    }
    

    printf("funfou até aqui \n");

    uint8_t (*scattered_frames)[height][width] = calloc(frames_total / world_size, sizeof(*scattered_frames));
    // MPI_Datatype node_type = scatter(raw_frames, scattered_frames);
    MPI_Scatter(raw_frames, width * height * frames_total  / world_size, MPI_UINT8_T, 
                scattered_frames, width * height * frames_total  / world_size, MPI_UINT8_T, 
                0, MPI_COMM_WORLD);

    //TODO: colocar essa aberração numa função
    //o objetivo desse código é permitir que a gente envie a nossa struct pelo scather/gatter sem usar packs
    //ta uma desgraça, alguem esconde isso daqui por favor
    MPI_Datatype node_type;
    int lengths[3] = { 1, 1, 1 };

    MPI_Aint displacements[3];
    struct node dummy_node;
    MPI_Aint base_address;
    MPI_Get_address(&dummy_node, &base_address);
    MPI_Get_address(&dummy_node.diff, &displacements[0]);
    MPI_Get_address(&dummy_node.x, &displacements[1]);
    MPI_Get_address(&dummy_node.y, &displacements[2]);
    displacements[0] = MPI_Aint_diff(displacements[0], base_address);
    displacements[1] = MPI_Aint_diff(displacements[1], base_address);
    displacements[2] = MPI_Aint_diff(displacements[2], base_address);
 
    MPI_Datatype types[3] = { MPI_INT, MPI_INT, MPI_INT };
    MPI_Type_create_struct(3, lengths, displacements, types, &node_type);
    MPI_Type_commit(&node_type);


    //gatter

    struct node (*best_frames)[3600] = calloc(frames_total / world_size, sizeof(*best_frames));

    for (int f = 0; f < frames_total / world_size; f++){
        full_search(scattered_frames, f, reference_frame, best_frames);
    }

    
    struct node (*all_best_frames)[3600] = NULL;
    
    if(my_rank == 0){
        all_best_frames = calloc(frames_total, sizeof(*all_best_frames));
    }    


    printf("finalizou todos os pedaços do vetor \n");

    MPI_Gather(best_frames, 3600 * (frames_total / world_size), node_type, 
                all_best_frames, 3600 * (frames_total / world_size), node_type, 0, MPI_COMM_WORLD);
    

    if(my_rank == 0){
        write_uncompressed_file(raw_frames, all_best_frames);

        write_crompressed_vectors(all_best_frames);
    }

    free(raw_frames);
    free(best_frames);
    free(all_best_frames);
    free(scattered_frames);

    end = omp_get_wtime();
    printf("execution time: %f \n", end-start);
    
    MPI_Finalize();
    return 0;
}

