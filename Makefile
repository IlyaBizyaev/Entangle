CC=g++
### DEFINITIONS ###
ifeq ($(OS),Windows_NT)
# FOR WINDOWS
WXDIR=C:\wxWidgets-3.0.2
CRYPTDIR='C:\cryptopp562\'
CFLAGS=-Wall -pipe -mthreads -D__GNUWIN32__ -D__WXMSW__ -DwxUSE_UNICODE -O2 -I$(WXDIR)\include -I$(CRYPTDIR) -I$(WXDIR)\lib\gcc_lib\mswu -c
LIBS=-lwxmsw30u_core -lwxbase30u -lwxpng -lwxjpeg -lwxtiff -lwxzlib -lkernel32 -lgdi32 -lwinspool -lcomdlg32 -lshell32 -lole32 -loleaut32 -luuid -lcomctl32
else
# FOR LINUX
CFLAGS=-I/usr/lib/x86_64-linux-gnu/wx/include/gtk2-unicode-3.0 -I/usr/include/wx-3.0 -D_FILE_OFFSET_BITS=64 -D__WXGTK__ -pthread -Wall -O2 -c
CRYPTOPP=-lcryptopp		# Redefine on Linux if you built Crypto++ from sources
endif

build: i18n
#### BUILDING ####
	$(CC) $(CFLAGS) EntangleApp.cpp -o EntangleApp.o
	$(CC) $(CFLAGS) EntangleMain.cpp -o EntangleMain.o

ifeq ($(OS),Windows_NT)
# WINDOWS BUILD
	windres.exe -I$(WXDIR)\include -I$(WXDIR)\lib\gcc_lib\mswu -J rc -O coff -i resource.rc -o resource.res

	$(CC) -L$(WXDIR)\lib\gcc_lib -o Entangle.exe EntangleApp.o EntangleMain.o  resource.res -s -static-libgcc -static-libstdc++ -mthreads $(LIBS) $(CRYPTDIR)libcryptopp.a -mwindows
else
# LINUX BUILD
	$(CC) -o Entangle EntangleApp.o EntangleMain.o  -s -L/usr/lib/x86_64-linux-gnu -pthread -lwx_gtk2u_core-3.0 -lwx_baseu-3.0   $(CRYPTOPP)
endif
i18n:
	msgfmt -o ./ru/Entangle.mo ./ru/Entangle.po
	msgfmt -o ./de/Entangle.mo ./de/Entangle.po
install:
	install Entangle /usr/local/bin
uninstall:
	rm -rf /usr/local/bin/Entangle
clean:
	rm -rf *.o Entangle Entangle.exe ./ru/Entangle.mo ./de/Entangle.mo resource.res
