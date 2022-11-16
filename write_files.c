#include "headers.h"

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