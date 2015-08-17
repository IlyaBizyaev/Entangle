CC=g++
SOURCES=EntangleMain.cpp EntangleApp.cpp EntangleFrame.cpp EntangleExtras.cpp
### DEFINITIONS ###
ifeq ($(OS),Windows_NT)
# FOR WINDOWS
WXDIR=/c/wxWidgets-3.0.2
# Where the 'cryptopp' dir is located 
CRYPTDIR=/c/
CFLAGS=-Wall -pipe -mthreads -D__GNUWIN32__ -D__WXMSW__ -O2 -DwxUSE_UNICODE -I$(WXDIR)/include -I$(CRYPTDIR) -I$(WXDIR)/lib/gcc_lib/mswu -I./inc/ -c
LIBS=-lwxmsw30u_core -lwxbase30u -lwxmsw30u_adv -lwxpng -lwxjpeg -lwxtiff -lwxzlib -lkernel32 -lgdi32 -lwinspool -lcomdlg32 -lshell32 -lole32 -loleaut32 -luuid -lcomctl32
else
# FOR LINUX
CFLAGS=-I/usr/lib/x86_64-linux-gnu/wx/include/gtk2-unicode-3.0 -I/usr/include/wx-3.0 -I./inc/ -D_FILE_OFFSET_BITS=64 -D__WXGTK__ -Wall -O2 -c
endif

build: i18n
#### BUILDING ####
	@for file in $(SOURCES); do\
		printf "Building $$file..."; \
		$(CC) $(CFLAGS) ./src/$$file -o "$${file%cpp}o"; \
		echo "\t Done!"; \
	done

ifeq ($(OS),Windows_NT)
# WINDOWS BUILD
	windres.exe -I$(WXDIR)/include -I$(WXDIR)/lib/gcc_lib/mswu -J rc -O coff -i resource.rc -o resource.res

	$(CC) -L$(WXDIR)\lib\gcc_lib $(SOURCES:.cpp=.o) -o Entangle.exe resource.res -s -static-libgcc -static-libstdc++ -mthreads $(LIBS) $(CRYPTDIR)cryptopp/libcryptopp.a -mwindows
else
# LINUX BUILD
	$(CC) $(SOURCES:.cpp=.o) -o Entangle -s -pthread -lwx_gtk2u_core-3.0 -lwx_baseu-3.0  -lwx_gtk2u_adv-3.0 -lcryptopp
endif

.PHONY: i18n launch test install uninstall clean
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

test:
	mkdir ./test
	cp ./src/*.cpp ./test/
	./Entangle -m=encryption -p=superdupersecret ./test
	./Entangle -m=decryption -p=superdupersecret ./test
	@for file in ./test/*; do\
		printf "Comparing $$file and ./src$${file#./test}..."; \
		if cmp --silent $$file "./src$${file#./test}"; then \
			echo "\t PASS!"; \
		else \
			echo "\t FAIL!"; \
		fi; \
	done
	rm -r ./test
install:
	install Entangle /usr/local/bin
uninstall:
	rm -rf /usr/local/bin/Entangle
clean:
	rm -rf *.o Entangle Entangle.exe ./lang/*/Entangle.mo resource.res ./test
