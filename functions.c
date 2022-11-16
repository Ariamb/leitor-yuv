#include "headers.h"

void full_search(uint8_t frames[frames_total][height][width], int frame, uint8_t reference_frame[height][width], struct node best_block[frames_total / world_size][3600]) {

    printf("comecei a executar o frame %d no processador %d \n", frame, my_rank);
     #pragma omp parallel for collapse(2)
    for (int vertical = 0; vertical < 45; vertical++) {
        for (int horizontal = 0; horizontal < 80; horizontal++) {
            int aux; 
            struct node local_block;
            local_block.diff = 99999;
            for (int x = 0; x <= height-8; x++) { 

                for (int y = 0; y <= width-8; y++){ 

                    aux = compare_block(frames, frame, x, y, vertical * 8, horizontal * 8, reference_frame); 

                    if (aux < local_block.diff) {
                        local_block.diff = aux;
                        local_block.x = x;
                        local_block.y = y;
                    }

                }
            }
            best_block[frame][80 * vertical + horizontal] = local_block;
        }
    }
    printf("terminei de executar o frame %d no processador %d \n", frame, my_rank);
}

int compare_block(
    uint8_t frames[frames_total][height][width], 
    int frame, 
    int pixel_x, 
    int pixel_y, 
    int vertical, 
    int horizontal,
    uint8_t reference_frame[height][width]
    ) { 

    int diff = 0;
    for(int row = 0; row < 8; row++) {
        for(int col = 0; col < 8; col++) {
            diff += abs(reference_frame[pixel_x + row][pixel_y + col] - frames[frame][vertical + row][horizontal + col]);
        }   
    }

    return diff;
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

void write_uncompressed_file(uint8_t frames[frames_total][height][width], struct node best_frames[frames_total][3600]) { //uncompressed
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

void write_crompressed_vectors(struct node best_frames[frames_total][3600]){
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