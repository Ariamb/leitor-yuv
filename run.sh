if [ -f "video_converted_640x360.yuv" ]; 
then
    echo "Compiling"
    
    # If first parameter is not empty, use clang to compile the file
    if [ $# -eq 0 ]; 
    then
        gcc -fopenmp -Wall leitor.c -lpthread -o leitor
    else
        /usr/local/opt/llvm/bin/clang -fopenmp -L/usr/local/opt/llvm/lib leitor.c -o leitor
    fi

    echo "Executing"
    ./leitor

    echo "Removing executable"
    rm leitor
else
    echo "video_converted_640x360.yuv has not been found"
fi

