# leitor-yuv

## Compilando:

gcc leitor.c -o leitor

./ leitor

(sem paralelismo por hora)

(requer arquivo "video_converted_640x360.yuv" no diretório)

## Especificação do trabalho

Trabalhos em grupos de até 4 indivíduos
Implementação em OpenMP
Data de entrega:  14/outubro
Tema: Algoritmo Full Search para estimação de movimento em vídeos

Considere  um vídeo e a necessidade de armazená-lo ou enviá-lo via streaming. É interessante que a representação deste vídeo seja compactada, visando economia de recursos. O algoritmo Full Search aplica uma heurística de compactação que compara dois frames de um vídeo, de forma a enviar o primeiro e, do segundo, apenas as informações "inéditas" e de deslocamento das informações já contidas no frame anterior.  O algoritmo básico é:

Tenha como entrada dois frames, o frame R (referência)  e o frame A (o frame atual, o qual irá ser comparado)
Considere a comparação de blocos de tamanho 8x8
A saída é são dois vetores Rv e Ra de pares x,y, indicando qual bloco 8x8 em R contém as informações do bloco 8x8 em A
Percorra toda o frame A pixel à pixel, (escolha se vai na horizontal ou na vertical primeiro) a partir do pixel selecionado, monte um bloco 8x8 e procure onde este bloco se encontra no frame R
Se encontrou o frame, armazenar nos vetores Rv e Ra a correspondência
Implemente este algoritmo em OpenMP, explorando seu alto grau de paralelismo. Lembre que o Trabalho de Implementação B (troca de mensagens) este programa deverá ser estendido para suportar MPI.

Formato de arquivo de vídeo sugerido Y'UV:

Pode ser usado outro, se sentirem maior conforto
Extensão: .yuv

Um vídeo é uma sequência de frames
Cada frame é descrito por três canais:
O primeiro canal é o canal Y, de luminância
O segundo canal é o canal Cb, de crominância azul
O terceiro canal é o canal Cr, de crominância vermelha
O vídeo é de 640x360 (640 pixels de largura por 360 pixels de altura)
No arquivo, para cada frame, existem os dados dos canais Y, Cbe Cr: O canal de luminância possui todos os pixels (640x360). Já os canais Cb e Cr possuem apenas 1/4 dos pixels (portanto, 320x180). Logo, a leitura de cada quadro deve ser feita lendo uma matriz de 640x360 (Canal Y), e na sequência duas matrizes de 320x180 (Canais Cb e Cr).
Observações:

Para nosso exercício, apenas o primeiro canal (o canal Y) pode ser considerado
Caso o grupo queria usar outro formato de arquivo, sinta-se a vontade
No nosso exercício, iremos tomar como frame R o primeiro frame do vídeo, devendo como saída ser apresentado os vetores de deslocamento de cada frame
Percorrer todo frame R para fazer a comparação é uma simplificação da solução. Heurísticas mais elaboradas delimitam uma região de busca pela proximidade da região selecionada
