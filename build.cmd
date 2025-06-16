mingw32-make clean
gcc -O2 -Wall -I. -Istubs -c dump1090.c -D_WIN32
gcc -O2 -Wall -I. -Istubs -c anet.c -D_WIN32
gcc -O2 -Wall -I. -Istubs -c TLSsample/tserver.c -D_WIN32
gcc -O2 -Wall -I. -Istubs -c sqlog.c -D_WIN32