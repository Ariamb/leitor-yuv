clear
if [ -f "video_converted_640x360.yuv" ]; 
then
    echo "Compiling"
    
    # If first parameter is not empty, use clang to compile the file
    if [ $# -eq 0 ]; 
    then
        gcc -fopenmp -Wall main.c -lpthread -o main
    else
        /usr/local/opt/llvm/bin/clang -fopenmp -L/usr/local/opt/llvm/lib main.c -o main
    fi

    echo "Executing"
    ./main

    echo "Removing executable"
    rm main
else
    echo "video_converted_640x360.yuv has not been found"
fi

