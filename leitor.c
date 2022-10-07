
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

struct resultadoBloco {   
  int diferenca;
  int posx;
  int posy;
};

 
int compara(int bloco0[8][8], int blocoframe[8][8]);


int main()
{
 
    FILE* video;
    //FILE* bytefind;

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

    int resolucao[] = {120,360,640}; //frames x altura x largura
    
    //gambiarras pra uso de um espaço de memória contigua (precisa ser contiguo pra funcionar em mpi dps):
    uint8_t (*pixel)[resolucao[1]][resolucao[2]] = calloc(resolucao[0], sizeof(*pixel));
    video = fopen("video_converted_640x360.yuv", "r");
    //bytefind = fopen("video_converted_640x360.yuv", "r");
    //bytefind = fseek(bytefind, 0, SEEK_END);

    // lê toda a matriz Y
    // em seguida, lê as duas matrizes cb e cr
    // skip de 115200 posições
    for(i = 0; i < 120; i++){
        for(j = 0; j < 360; j++){
            for(k = 0; k < 640; k++){
                byte = fgetc(video);
                pixel[i][j][k] = byte;
            }
        }
        fseek(video, 180*320*2, SEEK_CUR); // compensa crominância
    }

    // não fiz isso paralelamente por causa da forma que o fgetc funciona
    // MAS da pra fazer isso paralelamente. 
    // Pra isso, da pra abrir o mesmo arquivo com fopen um número de vezes igual o numero de threads
    // cada ponteiro do fopen aponta pra um lugar diferente
    // com fseek(), da pra pular a posição que cada ponteiro aponta
    // e da pra definir margens de até onde é lido
    // depois essas leituras quebradas são juntadas.
    // com uma boa aritmética e uns mutex da vida (que o openmp abstrai), da pra fazer isso rodar paralelo

    fclose(video);





    free(pixel); // tem que lembrar de limpar a memória pq essa variável é gigante

 
    return 0;
}

int compara(int bloco0[8][8], int blocoframe[8][8]){

    int r;
    int diff = 0;
    //#pragma omp parallel for colapse(2)
    for(int i = 0; i < 8; i++){
        for(int j = 0; j < 8; j++){
            diff += abs(blocoframe[i][j] - bloco0[i][j]);
        }   
    }


    return diff;
}
