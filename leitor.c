
#include <stdio.h>
#include <stdlib.h>
 
int main()
{
 
    // pointer demo to FILE
    FILE* demo;
    char c; //cha é exatamente 8 bits, então tem q fazer gambiarra de tipo que o C adora
    int i = 0;
    //char: size é 1 byte
    //100*640*360 = 23040000
    //int array[100][640][360]; esse tamanho causa segmentation fault. É necessário dinamicamente construir o array
    //640x360
    //100 frames, de acordo com o murilo do vitech (acho q é esse  nome dele)

    //além disso, temos os blocos cb e cr: duas matrizes 320x180 cada
    //int cb[100][320][180];
    //int cr[100][320][180];
    //100*320*180 = 5760000
    //*2, por 2 arrays
    
    demo = fopen("video_converted_640x360.yuv", "r");
    for(i = 0; i < 100; i++){
        c = fgetc(demo);
        //pixel[0][i][320] = c;
        printf("%d \n", c);    
    }
    // closes the file
    fclose(demo);
 
    return 0;
}
