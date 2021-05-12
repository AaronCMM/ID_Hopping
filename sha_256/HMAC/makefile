LNK_OPT = -g -L/usr/lib64/ -lssl  -L/lib64/ -lcrypto -lpthread 
 
all:
	rm -f *.o
	rm -f canrec
	g++ -g -c algo_hmac.cpp ctimer.cpp ge_idpool.cpp
	g++ -g canrec2.cpp -o canrec algo_hmac.o ge_idpool.o ctimer.o $(LNK_OPT)
	g++ -g cansend.cpp -o cansend algo_hmac.o ge_idpool.o ctimer.o $(LNK_OPT)
 
clean:
	rm -f *.o
#	rm -f canrec
