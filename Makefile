CC=g++
### DEFINITIONS ###
ifeq ($(OS),Windows_NT)
# FOR WINDOWS
WXDIR=C:\wxWidgets-3.0.2
# Where the cryptopp dir is located 
CRYPTDIR='C:\'
CFLAGS=-Wall -pipe -mthreads -D__GNUWIN32__ -D__WXMSW__ -DwxUSE_UNICODE -I$(WXDIR)\include -I$(CRYPTDIR) -I$(WXDIR)\lib\gcc_lib\mswu -I./inc/ -c
LIBS=-lwxmsw30u_core -lwxbase30u -lwxmsw30u_adv -lwxpng -lwxjpeg -lwxtiff -lwxzlib -lkernel32 -lgdi32 -lwinspool -lcomdlg32 -lshell32 -lole32 -loleaut32 -luuid -lcomctl32
else
# FOR LINUX
CFLAGS=-I/usr/lib/x86_64-linux-gnu/wx/include/gtk2-unicode-3.0 -I/usr/include/wx-3.0 -I./inc/ -D_FILE_OFFSET_BITS=64 -D__WXGTK__ -Wall -O2 -c
endif

build: i18n
#### BUILDING ####
	$(CC) $(CFLAGS) ./src/EntangleApp.cpp -o EntangleApp.o
	$(CC) $(CFLAGS) ./src/EntangleMain.cpp -o EntangleMain.o
	$(CC) $(CFLAGS) ./src/EntangleExtras.cpp -o EntangleExtras.o

ifeq ($(OS),Windows_NT)
# WINDOWS BUILD
	windres.exe -I$(WXDIR)\include -I$(WXDIR)\lib\gcc_lib\mswu -J rc -O coff -i resource.rc -o resource.res

	$(CC) -L$(WXDIR)\lib\gcc_lib -o Entangle.exe EntangleApp.o EntangleMain.o EntangleExtras.o resource.res -s -static-libgcc -static-libstdc++ -mthreads $(LIBS) $(CRYPTDIR)cryptopp\libcryptopp.a -mwindows
else
# LINUX BUILD
	$(CC) -o Entangle EntangleApp.o EntangleMain.o EntangleExtras.o -s -pthread -lwx_gtk2u_core-3.0 -lwx_baseu-3.0  -lwx_gtk2u_adv-3.0 -lcryptopp
endif

.PHONY: i18n install uninstall clean
i18n:
	@for file in ./lang/*/Entangle.po; do\
		msgfmt $$file -o "$${file%po}mo"; \
	done
launch:
ifeq ($(OS),Windows_NT)
	./Entangle.exe
else
	./Entangle
endif
install:
	install Entangle /usr/local/bin
uninstall:
	rm -rf /usr/local/bin/Entangle
clean:
	rm -rf *.o Entangle Entangle.exe ./lang/*/Entangle.mo resource.res
