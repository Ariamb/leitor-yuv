#include "headers.h"

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