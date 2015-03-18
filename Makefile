CC=g++
CFLAGS=-I/usr/lib/x86_64-linux-gnu/wx/include/gtk2-unicode-3.0-unofficial -I/usr/include/wx-3.0-unofficial -D_FILE_OFFSET_BITS=64 -D__WXGTK__ -pthread -Wall -O2 -c
CRYPTOPP=../cryptopp562/libcryptopp.a

build:
	$(CC) $(CFLAGS) /home/ilya/Projects/Entangle/EntangleApp.cpp -o EntangleApp.o

	$(CC) $(CFLAGS) /home/ilya/Projects/Entangle/EntangleMain.cpp -o EntangleMain.o

	$(CC) -o Entangle EntangleApp.o EntangleMain.o  -s -L/usr/lib/x86_64-linux-gnu -pthread -lwx_gtk2u_unofficial_core-3.0 -lwx_baseu_unofficial-3.0   $(CRYPTOPP)
install:
	install Entangle /usr/local/bin
uninstall:
	rm -rf /usr/local/bin/Entangle
clean:
	rm -rf *.o Entangle
