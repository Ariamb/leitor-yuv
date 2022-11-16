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