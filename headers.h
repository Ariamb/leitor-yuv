#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <omp.h>
#include <mpi.h>

int world_size, my_rank;


#define frames_total 120
#define width 640
#define height 360
#define video_name "video_converted_640x360.yuv"
#define process_amount world_size //descobrir como pegar isso automaticamente

struct node {
  int diff;
  int x;
  int y;
};

int scatter(
    uint8_t raw_frames[frames_total][height][width],
    uint8_t scattered_frames[frames_total][height][width]
);

int compare_block(
    uint8_t frames[frames_total][height][width], 
    int frame,
    int x, 
    int y,
    int p,
    int q,
    uint8_t reference_frame[height][width]
);

void read_file(
    uint8_t raw_frames[frames_total][height][width], 
    int i, 
    int max
);
void write_crompressed_vectors(struct node best_frames[frames_total][3600]);

void write_uncompressed_file(uint8_t frames[frames_total][height][width], struct node best_frames[frames_total][3600]);

void write_frame_zero(uint8_t frames[frames_total][height][width]);

void full_search(
    uint8_t frames[frames_total][height][width], 
    int frame,
    uint8_t reference_frame[height][width],
    struct node best_block[frames_total / process_amount][3600]
);

void write_frame(uint8_t frames[frames_total][height][width], struct node *best_frames);
