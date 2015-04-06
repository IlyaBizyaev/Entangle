CC=g++
CFLAGS=-I/usr/lib/x86_64-linux-gnu/wx/include/gtk2-unicode-3.0 -I/usr/include/wx-3.0 -D_FILE_OFFSET_BITS=64 -D__WXGTK__ -pthread -Wall -O2 -c
CRYPTOPP=-lcryptopp

build:
	$(CC) $(CFLAGS) EntangleApp.cpp -o EntangleApp.o

	$(CC) $(CFLAGS) EntangleMain.cpp -o EntangleMain.o

	$(CC) -o Entangle EntangleApp.o EntangleMain.o  -s -L/usr/lib/x86_64-linux-gnu -pthread -lwx_gtk2u_core-3.0 -lwx_baseu-3.0   $(CRYPTOPP)
	msgfmt -o ./ru/Entangle.mo ./ru/Entangle.po
	msgfmt -o ./de/Entangle.mo ./de/Entangle.po
i18n:
	msgfmt -o ./ru/Entangle.mo ./ru/Entangle.po
	msgfmt -o ./de/Entangle.mo ./de/Entangle.po
install:
	install Entangle /usr/local/bin
uninstall:
	rm -rf /usr/local/bin/Entangle
clean:
	rm -rf *.o Entangle ./ru/Entangle.mo ./de/Entangle.mo
