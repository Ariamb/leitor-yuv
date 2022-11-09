#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <omp.h>
#include <mpi.h>

#define frames_total 120
#define width 640
#define height 360
#define video_name "video_converted_640x360.yuv"

struct node {
  int diff;
  int x;
  int y;
};


int compare_block(
    uint8_t frames[frames_total][height][width], 
    int frame,
    int x, 
    int y, 
    int p, 
    int q
);

void read_file(
    uint8_t raw_frames[frames_total][height][width], 
    int i, 
    int max
);
void write_crompressed_vectors(struct node *best_frames[frames_total]);

void write_uncompressed_file(uint8_t frames[frames_total][height][width], struct node *best_frames[frames_total]);

void write_frame_zero(uint8_t frames[frames_total][height][width]);
struct node * full_search(
    uint8_t frames[frames_total][height][width], 
    int frame
);

void write_frame(uint8_t frames[frames_total][height][width], struct node *best_frames);

int main() {


    




    double start, end;
    start = omp_get_wtime();

    int num_threads = omp_get_max_threads();

    int frames_chunk = frames_total/num_threads;

    int world_size, my_rank;

    MPI_Init(NULL, NULL);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);


/*
    já que reclamaram do commit em inglês:

    o ranque zero é o mestre. Nesse caso, ele faz aquela parte inicial de leitura do arquivo,
    e manda tudo pro ranque 1. O ranque 1 executa tudo como se fosse um único processador.
    Esse código só funfa se rodar com 2 processos no mpirun

    Tem que arrumar pra delegar entre mais processos
    e de forma que o ranque 0 não fique a toa o tempo todo

*/

    if(my_rank == 0){
    uint8_t (*raw_frames)[height][width] = calloc(frames_total, sizeof(*raw_frames));

        #pragma omp parallel for 
            for (int i = 0; i < num_threads; i++) 
            {
                read_file(
                    raw_frames, 
                    i * frames_chunk,
                    i * frames_chunk + frames_chunk
                );
            }

        for (int f = 0; f < frames_total; f++){
            int target = 1; //temos que fazer funcionar pra vários processos
            MPI_Send(raw_frames[f], width * height, MPI_UINT8_T, target, 0, MPI_COMM_WORLD);
            printf("sent frame %d to target %d \n", f, target);
        }
        free(raw_frames);
    }

    if(my_rank == 1){
        uint8_t (*raw_frames)[height][width] = calloc(frames_total, sizeof(*raw_frames));

        struct node *best_frames[frames_total];

        for (int f = 0; f < frames_total; f++){
            MPI_Status status; 
            int ierr = MPI_Recv(raw_frames[f], width * height, MPI_UINT8_T, 0, 0, MPI_COMM_WORLD, &status);
            if(ierr != MPI_SUCCESS)
                printf("Error found during frame %d\n", f);
            else printf("received %d frame\n", f);
            
        }
        
        for(int l = 0; l < frames_total; l++) {   
            best_frames[l] = full_search(raw_frames, l);
        }
        write_uncompressed_file(raw_frames, best_frames);

        write_crompressed_vectors(best_frames);
        
        free(raw_frames);
        for(int i = 0; i < frames_total; i++){
            free(best_frames[i]);
        }
        
    }

    end = omp_get_wtime();
    printf("execution time: %f \n", end-start);
    
    MPI_Finalize();
    return 0;
}


void read_file(uint8_t raw_frames[frames_total][height][width], int i, int max){
    FILE* file = fopen(video_name, "r");

    fseek(
        file, 
        i * (width * height + 320 * 180 * 2), 
        SEEK_SET
    );

    for(; i < max; i++){
        for(int j = 0; j < height; j++)
            for(int k = 0; k < width; k++)
                raw_frames[i][j][k] = fgetc(file);
        
        fseek(
            file, 
            180 * 320 * 2, 
            SEEK_CUR
        ); 
    }

    fclose(file);

}

int compare_block(
    uint8_t frames[frames_total][height][width], 
    int frame, 
    int pixel_x, 
    int pixel_y, 
    int vertical, 
    int horizontal
    ) { 

    int diff = 0;
    for(int row = 0; row < 8; row++) {
        for(int col = 0; col < 8; col++) {
            diff += abs(frames[0][pixel_x + row][pixel_y + col] - frames[frame][vertical + row][horizontal + col]);
        }   
    }

    return diff;
}

struct node * full_search(uint8_t frames[frames_total][height][width], int frame) {

    int resolution = 3600;

    struct node (*frames_video) = calloc(resolution, sizeof(struct node));
    
    printf("comecei a executar o frame %d \n", frame);
     #pragma omp parallel for collapse(2)
    for (int vertical = 0; vertical < 45; vertical++) {
        for (int horizontal = 0; horizontal < 80; horizontal++) {
            int aux; 
            struct node best_block;
            best_block.diff = 99999;
            for (int x = 0; x <= height-8; x++) { 

                for (int y = 0; y <= width-8; y++){ 

                    aux = compare_block(frames, frame, x, y, vertical * 8, horizontal * 8); 

                    if (aux < best_block.diff) {
                        best_block.diff = aux;
                        best_block.x = x;
                        best_block.y = y;
                    }

                }
            }
            frames_video[80 * vertical + horizontal] = best_block;
        }
    }
    printf("terminei de executar o frame %d \n", frame);

    return frames_video;
}


void write_uncompressed_file(uint8_t frames[frames_total][height][width], struct node *best_frames[frames_total]) { //uncompressed
    FILE* file = fopen("video_uncompressed.yuv", "w"); 

    if(file == NULL){
        printf("arquivo não pode ser criado\n");
        return;
    }
    uint8_t chromaFalsa = 0;

    for(int f = 0; f < frames_total; f++){
        for(int i = 0; i < 45; i++){
            for(int row  = 0; row < 8; row++){
                for(int j = 0; j < 80; j++){
                    struct node b = best_frames[f][80 * i + j];
                    for(int col = 0; col < 8; col++){
                        fwrite(&frames[0][row + b.x][col + b.y], sizeof(uint8_t), 1, file);
                    }
                }
            }
        }

        for(int i = 0; i < 180 * 2; i++){
            for(int j = 0; j < 320; j++){
                fwrite(&chromaFalsa, sizeof(uint8_t), 1, file);
            }
        }
    }

    return;
}





void write_crompressed_vectors(struct node *best_frames[frames_total]){
    FILE * frv = fopen("rvcompressed.bin", "w");
    FILE * fra = fopen("racompressed.bin", "w");
    struct node b;
    for(int f = 0; f < 120; f++){
        for(int vertical=0; vertical < 45; vertical++){
            for( int horizontal=0; horizontal < 80; horizontal++){
                b = best_frames[f][80 * vertical + horizontal];
                fwrite(&b.x, sizeof(int), 1, frv);
                fwrite(&b.y, sizeof(int), 1, frv);

                int aux1 = vertical * 8;
                int aux2 = horizontal * 8;
                fwrite(&aux1, sizeof(int), 1, fra);
                fwrite(&aux2, sizeof(int), 1, fra);

            }
        }
            
        }
    fclose(frv);
    fclose(fra);
    return;
}

