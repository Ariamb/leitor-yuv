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
    uint8_t raw_video[frames_total][height][width], 
    int i, 
    int max
);

void write_file(uint8_t raw_video[frames_total][height][width]);

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

    uint8_t (*raw_video)[height][width] = calloc(frames_total, sizeof(*raw_video));
    
    omp_set_num_threads(num_threads);

    /* Reads file */
    #pragma omp parallel for 
        for (int i = 0; i < num_threads; i++) {
            read_file(
                raw_video, 
                i * frames_chunk,
                i * frames_chunk + frames_chunk
            );
        }


    /* printf("tempo resultante do parallel for: %f \n", end-start); */
   
    struct node *best_frames[119];
    
    #pragma omp parallel for 
    for(int i = 1; i < frames_total; i++) {
        best_frames[i-1] = full_search(raw_video, i);
    }

    free(raw_video);
 
    return 0;
}


void read_file(uint8_t raw_video[frames_total][height][width], int i, int max){
    FILE* file = fopen(video_name, "r");

    fseek(
        file, 
        i * (width * height + 320 * 180 * 2), 
        SEEK_SET
    );

    int j = 0, k = 0;

    for(; i < max; i++){
        for(j = 0; j < height; j++)
            for(k = 0; k < width; k++)
                raw_video[i][j][k] = fgetc(file);
        
        /* Crominância */
        fseek(
            file, 
            180 * 320 * 2, 
            SEEK_CUR
        ); 
    }

    fclose(file);

}

int compare_block(uint8_t frames[frames_total][height][width], int frame, int x, int y, int p, int q) { 
    //frames, frame a comparar, pixel vertical, pixel horizontal, deslocamento de bloco vertical. deslocamento de bloco horizontal

    int diff = 0;

    for(int row = 0; row < 8; row++) {
        for(int col = 0; col < 8; col++) {
            diff += abs(frames[0][x + row][y + col] - frames[frame][p + row][q + col]);
        }   
    }

    return diff;
}

struct node * full_search(uint8_t frames[frames_total][height][width], int frame){

    int resolucao = 3600;

    struct node melhorBloco;
    //aqui cabem os dados de um unico frame ((height/8)*(630/8))
    struct node (*framesVideo) = calloc(resolucao, sizeof(struct node));

    //cada frame tem 45*80 blocos compactáveis
    //num frame, cada bloco vai fazer uma busca em até 352*632 blocos
    //então vamos term 45*80*352*632 testes nesse for
    //800870400 iterações
    //800 MILHÕES de iterações
    //acho que tem alguma coisa errada aqui, na moral
    int aux, posArray = 0;
    printf("comecei a executar o frame %d \n", frame);
    for(int p = 0; p < 45; p++){//vertical
        for(int q = 0; q < 80; q++){//horizontal
            melhorBloco.diff = 99999;
            for(int i = 0; i <= height-8; i++){ //<=height-8 //i<= 632 vertical
                for(int j = 0; j <= width-8; j++){ //<=width-8 horizontal

                    aux = compare_block(frames, frame, i, j, p * 8, q * 8); //canto superior esquerdo
                    //frames gerais, frame a comparar, pixel vertical, pixel horizontal, deslocamento de bloco vertical, deslocamento de bloco horizontal
                    if(aux < melhorBloco.diff){
                        melhorBloco.diff = aux;
                        melhorBloco.x = i;
                        melhorBloco.y = j;
                    }
                    if(melhorBloco.diff == 0){
                        break;
                    }
                }
                if(melhorBloco.diff == 0){
                    break;
                }
            }
        //printf("funcionando");
        //printf("achei o bloco no %d %d do frame %d \n", p, q, frame);
        framesVideo[posArray] = melhorBloco;
        posArray++;
        }
    }
    //framesVideo[0] primeiro bloco
    //framesVideo[80] ultimo bloco da primeira linha
    //framesVideo[80] primeiro bloco da primeira linha
    //framesVIdeo[3600] ultimo bloco
    printf("terminei de executar o frame %d \n", frame);

    return framesVideo;
}



void write_file(uint8_t frames[frames_total][height][width]){//, struct node blocos[119][3600]){
    FILE* file = fopen("video_comprimido.yuv", "w"); //n adianta ser .yuv

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
