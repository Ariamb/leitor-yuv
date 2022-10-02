
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

 
int main()
{
 
    FILE* video;
    int i = 0, j = 0, k = 0;
    uint8_t byte; //representa perfeitamente os dados do video: 8 bits sem sinal
    //se usasse char, teria problemas de conversão e sinal


    //int array[120][640][360]; esse tamanho de array causa segmentation fault. É necessário dinamicamente construir o array
    //resolução de 640x360 com 120 frames, de acordo com o programa que abre arquivos yuv
    //a ALTURA é 360, e LARGURA é 640. 
    //Portando, considerando ordenação esquerda-pra-direita e cima-pra-baixo, o array fica [360][640]
    //Tamanho 120*360*640
    //além disso, temos os blocos cb e cr: duas matrizes 320x180 cada. Tamanho 120*180*320
    //char cb[120][180][320];
    //char cr[120][180][320];

    int resolucao[] = {120,360,640}; //frames x altura x largura
    
    //gambiarras pra uso de um espaço de memória contigua (precisa ser contiguo pra funcionar em mpi dps):
    uint8_t (*pixel)[resolucao[1]][resolucao[2]] = calloc(resolucao[0], sizeof(*pixel));
    uint8_t (*cb)[resolucao[1]/2][resolucao[2]/2] = calloc(resolucao[0], sizeof(*cb));
    uint8_t (*cr)[resolucao[1]/2][resolucao[2]/2] = calloc(resolucao[0], sizeof(*cr));

    video = fopen("video_converted_640x360.yuv", "r");
    // lê toda a matriz Y
    // em seguida, lê as duas matrizes cb e cr
    for(i = 0; i < 120; i++){
        for(j = 0; j < 360; j++){
            for(k = 0; k < 640; k++){
                byte = fgetc(video);
                pixel[i][j][k] = byte;
            }
        }

        for(j = 0; j < 180; j++){ 
            for(k = 0; k < 320; k++){
                byte = fgetc(video);
                cb[i][j][k] = byte;
            }
        }
        for(j = 0; j < 180; j++){ 
            for(k = 0; k < 320; k++){
                byte = fgetc(video);
                cr[i][j][k] = byte;
            }
        }
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

    free(pixel); // tem que lembrar de limpar a memória pq juntas, as 3 váriaveis passam dos 40mb
    free(cb);
    free(cr);
 
    return 0;
}
