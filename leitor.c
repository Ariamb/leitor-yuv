
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
 

int compara(uint8_t frames[120][360][640], int frame, int x, int y, int p, int q);
void leFrames(uint8_t pixel[120][360][640], int i, int max);
void escreveArquivo(uint8_t pixel[120][360][640]);
struct resultadoBloco * buscaCompleta(uint8_t frames[120][360][640], int frame);


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



    for(i = 0; i < 8; i++){
        for(j = 0; j < 8; j++){
            printf("%d ", pixel[0][i][j]);
        }
        printf("\n");
    }

    //printf("tempo resultante do parallel for: %f \n", end-start);

    //----------------full search alchemist---------------------
   /*
    struct resultadoBloco (*framesVideo) = buscaCompleta(pixel, 1);
    //printf("tem isso no array: %d", framesVideo[0].posx);
    for(int i = 0; i < 2; i++){
        printf("bloco que eu quero: \n");
        for(int j = 0; j < 8; j++){
            for(int k = 0; k < 8; k++){
                printf("%d ", pixel[0][j + i * 8][k]);
            }
            printf("\n");
        }
        printf("stats do bloco que achei: \n x: %d, y: %d, diff: %d \n", framesVideo[i].posx, framesVideo[i].posy, framesVideo[i].diferenca);
        printf("bloco que achei: \n");
        for(int j = 0; j < 8; j++){
            for(int k = framesVideo[i].posy; k < 8; k++){
                printf("%d ", pixel[0][j + framesVideo[i].posx][k + framesVideo[i].posy]);
            }
            printf("\n");
        }        
    }
    */
    escreveArquivo(pixel);
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

// heurística de comparação de sem usar novas variaveis
int compara(uint8_t frames[120][360][640], int frame, int x, int y, int p, int q){ 

    int diff = 0;
    for(int i = 0; i < 8; i++){
        for(int j = 0; j < 8; j++){
            diff += abs(frames[0][x+i][y+j] - frames[frame][p+i][q+j]);
            //escolher um cálculo melhor em vez de só acumular a diferença?
        }   
    }
    return diff;
}


//119
struct resultadoBloco * buscaCompleta(uint8_t frames[120][360][640], int frame){

    int resolucao = 3600;

    struct resultadoBloco melhorBloco;
    //aqui cabem os dados de um unico frame ((360/8)*(630/8))
    struct resultadoBloco (*framesVideo) = calloc(resolucao, sizeof(struct resultadoBloco *));

    //cada frame tem 45*80 blocos compactáveis
    //num frame, cada bloco vai fazer uma busca em até 352*632 blocos
    //então vamos term 45*80*352*632 testes nesse for
    //800870400 iterações
    //800 MILHÕES de iterações
    //acho que tem alguma coisa errada aqui, na moral
    int aux, posArray = 0, totalIteracoes = 0;
    for(int p = 0; p < 45; p++){
        for(int q = 0; q < 80; q++){
            melhorBloco.diferenca = 99999;
            for(int i = 0; i <= 640-8; i++){ //<=360-8
                for(int j = 0; j <= 360-8; j++){ //<=640-8

                    aux = compara(frames, frame, i, j, p * 8, q * 8); //canto superior esquerdo
                    if(aux < melhorBloco.diferenca){
                        melhorBloco.diferenca = aux;
                        melhorBloco.posx = i;
                        melhorBloco.posy = j;
                    }
                    if(melhorBloco.diferenca == 0){
                        break;
                    }
                }
                if(melhorBloco.diferenca == 0){
                    break;
                }
            }
        //printf("funcionando");
        framesVideo[posArray] = melhorBloco;
        posArray++;
        }
    }
    printf("total de entradas no array: %d \n", posArray);
    printf("total de testes: %d \n", totalIteracoes);

    //framesVideo[0] primeiro bloco
    //framesVIdeo[3600] ultimo bloco
    return framesVideo;
}



void escreveArquivo(uint8_t frames[120][360][640]){//, struct resultadoBloco blocos[119][3600]){
    FILE* arquivo = fopen("video_comprimido.yuv", "w"); //n adianta ser .yuv

    if(arquivo == NULL){
        printf("arquivo não pode ser criado\n");
        return;
    }
    uint8_t chromaFalsa = 0;


    for(int i = 0; i < 360; i++){
        for(int j = 0; j < 640; j++){
            fwrite(&frames[0][i][j], sizeof(uint8_t), 1, arquivo);
        }
    }
    //precisa escrever crominancia pra funcionar!
    for(int i = 0; i < 180; i++){
        for(int j = 0; j < 320; j++){
            fwrite(&chromaFalsa, sizeof(uint8_t), 1, arquivo);
        }
    }
    for(int i = 0; i < 180; i++){
        for(int j = 0; j < 320; j++){
            fwrite(&chromaFalsa, sizeof(uint8_t), 1, arquivo);
        }
    }


    return;
}
