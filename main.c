#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <omp.h>

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


    uint8_t (*raw_frames)[height][width] = calloc(frames_total, sizeof(*raw_frames));
    


    struct node *best_frames[frames_total];


    double start, end;
    start = omp_get_wtime();

    int num_threads = omp_get_max_threads();

    int frames_chunk = frames_total/num_threads;

    #pragma omp parallel for 
        for (int i = 0; i < num_threads; i++) 
        {
            read_file(
                raw_frames, 
                i * frames_chunk,
                i * frames_chunk + frames_chunk
            );
        }

    for(int l = 0; l <120; l++) 
    {
        best_frames[l] = full_search(raw_frames, l);
    }

    end = omp_get_wtime();
    printf("execution time: %f \n", end-start);
    write_uncompressed_file(raw_frames, best_frames);

    write_crompressed_vectors(best_frames);

    for(int i = 0; i < frames_total; i++){
        free(best_frames[i]);
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

