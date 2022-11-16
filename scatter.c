#include "headers.h"

int scatter(uint8_t raw_frames[frames_total][height][width], uint8_t scattered_frames[frames_total][height][width]){
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

    return node_type;
}