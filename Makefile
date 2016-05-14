### GENERAL ###
CXX=g++
SOURCES=$(wildcard src/*.cpp) $(wildcard src/extras/*.cpp)
OBJECTS:=$(SOURCES:%.cpp=%.o)
LANG=$(wildcard lang/*/Entangle.po)
LANG_OBJ=$(LANG:.po=.mo)

CXXFLAGS=`wx-config --cppflags` --std=c++11 -I./inc/ -Wall -O2 -c
LIBS=`wx-config --libs`

### PLATFORM-DEPENDENT ###
ifeq ($(OS),Windows_NT)
# FOR WINDOWS
EXECUTABLE=Entangle.exe
CXXFLAGS+= -pipe -mthreads -Wno-deprecated-declarations -I/c/
LIBS += /c/cryptopp/libcryptopp.a -static-libgcc -static-libstdc++ -mthreads -mwindows resource.res
else
# FOR LINUX
EXECUTABLE=Entangle
LIBS += -lcryptopp
endif

### TARGETS ###

build: $(SOURCES) resource.res $(EXECUTABLE) $(LANG_OBJ)

.cpp.o:
	$(CXX) $(CXXFLAGS) $< -o $@

lang/%/Entangle.mo: lang/%/Entangle.po
	msgfmt $< -o $@

$(EXECUTABLE): $(OBJECTS) 
	$(CXX) $(OBJECTS) -o $@ -s $(LIBS)


# ICON FOR WINDOWS
resource.res:
ifeq ($(OS),Windows_NT)
	`wx-config --rescomp` -J rc -O coff -i resource.rc -o resource.res
endif


.PHONY: launch test install uninstall clean
launch:
	./$(EXECUTABLE)

test:
	cp -r src test
	./Entangle -m=e -p=superdupersecret ./test
	./Entangle -m=d -p=superdupersecret ./test
	@for file in ./test/*; do\
		printf "Comparing $$file and ./src$${file#./test}..."; \
		if cmp --silent $$file "./src$${file#./test}"; then \
			echo " PASS!"; \
		else \
			echo " FAIL!"; \
		fi; \
	done
	rm -r ./test
install:
	install Entangle /usr/bin
	desktop-file-install Entangle.desktop
	@for lang in ./lang/*/; do\
		cp "$$lang/Entangle.mo" "/usr/share/locale$${lang##./lang}LC_MESSAGES/Entangle.mo"; \
	done
	cp ./img/icon.png /usr/share/pixmaps/entangle.png
	mkdir -p /usr/share/Entangle/img/
	cp -p ./img/??cryption.png /usr/share/Entangle/img/
uninstall:
	rm -rf /usr/bin/Entangle /usr/share/applications/Entangle.desktop /usr/share/pixmaps/entangle.png /usr/share/Entangle/ /usr/share/locale/*/LC_MESSAGES/Entangle.mo
clean:
	rm -rf $(OBJECTS) $(EXECUTABLE) $(LANG_OBJ) resource.res
	find "." -name "*~" | xargs rm -f
