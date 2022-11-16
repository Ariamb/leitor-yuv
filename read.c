#include "headers.h"

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