
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <omp.h>

struct resultadoBloco {   
  int diferenca;
  int posx;
  int posy;
};

const int readingThreads = 24; // a quantia de tasks/threads usadas

const int fileLength = 120;
const int framesChunk = fileLength/readingThreads;
 

int compara(int bloco0[8][8], int blocoframe[8][8]);
void leFrames(uint8_t pixel[120][360][640], int i, int max);

struct resultadoBloco * buscaCompleta(uint8_t frames[120][360][640]);


int main()
{
    int i = 0, j = 0, k = 0;
    uint8_t byte; //representa perfeitamente os dados do video: 8 bits sem sinal
    //se usasse char, teria problemas de conversão e sinal

    //tamanho do arquivo em bytes: 41472000


    //int array[120][640][360]; esse tamanho de array causa segmentation fault. É necessário dinamicamente construir o array
    //resolução de 640x360 com 120 frames, de acordo com o programa que abre arquivos yuv
    //a ALTURA é 360, e LARGURA é 640. 
    //Portando, considerando ordenação esquerda-pra-direita e cima-pra-baixo, o array fica [360][640]
    //Tamanho 120*360*640
    //além disso, temos os blocos cb e cr: duas matrizes 320x180 cada. Tamanho 120*180*320
    //no total, essas posições ocupam 115200 bytes: 180*320*2, que devem ser compensados na leitura

    int resolucao[] = {fileLength,360,640}; //frames x altura x largura
    
    //gambiarras pra uso de um espaço de memória contigua (precisa ser contiguo pra funcionar em mpi dps):
    
    uint8_t (*pixel)[resolucao[1]][resolucao[2]] = calloc(resolucao[0], sizeof(*pixel));


    // DUAS IMPLEMENTAÇÕES POSSÍVEIS: ESCOLHAM
    // USANDO PARALLEL FOR OU USANDO TASKS/TASKWAIT
    // ps: a implementação que for feita primeiro pode ser mais lenta (caching e talz)

    double start;
    double end;
    
    omp_set_num_threads(readingThreads);

    start = omp_get_wtime();
    #pragma omp parallel for 
        for (int i = 0; i < readingThreads; i++)
        {
            leFrames(pixel, i * framesChunk, i * framesChunk + framesChunk);
        }
    end = omp_get_wtime();
    printf("tempo resultante do parallel for: %f \n", end-start);


    //full search alchemist




    free(pixel); // tem que lembrar de limpar a memória pq essa variável é gigante
 
    return 0;
}


void leFrames(uint8_t pixel[120][360][640], int i, int max){
    FILE* arquivo = fopen("video_converted_640x360.yuv", "r");

    fseek(arquivo, i*(640*360 + 320*180*2), SEEK_SET);

    int j = 0, k = 0;
    for(; i < max; i++){
        for(j = 0; j < 360; j++)
            for(k = 0; k < 640; k++)
                pixel[i][j][k] = fgetc(arquivo);
        
        fseek(arquivo, 180*320*2, SEEK_CUR); // compensa crominância
    }

    fclose(arquivo);

}

int compara(int bloco0[8][8], int blocoframe[8][8]){ // heurística de comparação de blocos

    int r;
    int diff = 0;
    #pragma omp parallel for collapse(2)
    for(int i = 0; i < 8; i++){
        for(int j = 0; j < 8; j++){
            diff += abs(blocoframe[i][j] - bloco0[i][j]);
        }   
    }


    return diff;
}

struct resultadoBloco * buscaCompleta(uint8_t frames[120][360][640]){

    int resolucao = 3600;

    struct resultadoBloco (*framesVideo) = calloc(resolucao, sizeof(struct resultadoBloco *));

    
}
