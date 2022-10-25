#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <omp.h>

#define num_threads 8
#define frames_total 120
#define width 640
#define height 360
#define video_name "video_converted_640x360.yuv"

struct node {
  int diff;
  int x;
  int y;
};

int frames_chunk = frames_total/num_threads;

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

void write_file(uint8_t raw_frames[frames_total][height][width]);

struct node * full_search(
    uint8_t frames[frames_total][height][width], 
    int frame
);

int main() {

    /*
        Tamanho do arquivo em bytes: 41472000
        blocos cb e cr: duas matrizes 320x180 cada. Tamanho frames_total*180*320
        no total, essas posições ocupam 115200 bytes: 180*320*2, que devem ser compensados na leitura 
    */

    uint8_t (*raw_frames)[height][width] = calloc(frames_total, sizeof(*raw_frames));
    
    omp_set_num_threads(num_threads);

    /* Reads file */
    #pragma omp parallel for 
        for (int i = 0; i < num_threads; i++) {
            read_file(
                raw_frames, 
                i * frames_chunk,
                i * frames_chunk + frames_chunk
            );
        }


    /* printf("tempo resultante do parallel for: %f \n", end-start); */
   
    struct node *best_frames[119];
    
    #pragma omp parallel for 
    for(int i = 1; i < frames_total; i++) {
        best_frames[i-1] = full_search(raw_frames, i);
    }

    free(raw_frames);
 
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
        
        /* Crominância */
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

    struct node best_block;

    //aqui cabem os dados de um unico frame ((height/8)*(630/8))
    struct node (*frames_video) = calloc(resolution, sizeof(struct node));

    int aux, posArray = 0;
    
    printf("comecei a executar o frame %d \n", frame);

    for (int vertical = 0; vertical < 45; vertical++) {

        for (int horizontal = 0; horizontal < 80; horizontal++) {

            best_block.diff = 99999;

            for (int x = 0; x <= height-8; x++) { 

                for (int y = 0; y <= width-8; y++){ 

                    aux = compare_block(frames, frame, x, y, vertical * 8, horizontal * 8); 

                    if (aux < best_block.diff) {
                        best_block.diff = aux;
                        best_block.x = x;
                        best_block.y = y;
                    }

                    if (best_block.diff == 0) {
                        break;
                    }
                }

                if (best_block.diff == 0) {
                    break;
                }
            }

        frames_video[posArray] = best_block;

        posArray++;
        
        }
    }

    printf("terminei de executar o frame %d \n", frame);

    return frames_video;
}

void write_file(uint8_t frames[frames_total][height][width]) {
    FILE* file = fopen("video_comprimido.yuv", "w"); 

    if(file == NULL){
        printf("arquivo não pode ser criado\n");
        return;
    }
    uint8_t chromaFalsa = 0;


    for(int i = 0; i < height; i++){
        for(int j = 0; j < width; j++){
            fwrite(&frames[0][i][j], sizeof(uint8_t), 1, file);
        }
    }
    //precisa escrever crominancia pra funcionar!
    for(int i = 0; i < 180; i++){
        for(int j = 0; j < 320; j++){
            fwrite(&chromaFalsa, sizeof(uint8_t), 1, file);
        }
    }
    for(int i = 0; i < 180; i++){
        for(int j = 0; j < 320; j++){
            fwrite(&chromaFalsa, sizeof(uint8_t), 1, file);
        }
    }


    return;
}