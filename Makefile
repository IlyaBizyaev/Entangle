build:
	g++ -I/usr/lib/x86_64-linux-gnu/wx/include/gtk2-unicode-3.0-unofficial -I/usr/include/wx-3.0-unofficial -D_FILE_OFFSET_BITS=64 -D__WXGTK__ -pthread -Wall -O2 -c /home/ilya/Projects/Entangle/EntangleApp.cpp -o EntangleApp.o

	g++ -I/usr/lib/x86_64-linux-gnu/wx/include/gtk2-unicode-3.0-unofficial -I/usr/include/wx-3.0-unofficial -D_FILE_OFFSET_BITS=64 -D__WXGTK__ -pthread -Wall -O2 -c /home/ilya/Projects/Entangle/EntangleMain.cpp -o EntangleMain.o

	g++  -o Entangle EntangleApp.o EntangleMain.o  -s -L/usr/lib/x86_64-linux-gnu -pthread   -lwx_gtk2u_unofficial_xrc-3.0 -lwx_gtk2u_unofficial_webview-3.0 -lwx_gtk2u_unofficial_html-3.0 -lwx_gtk2u_unofficial_qa-3.0 -lwx_gtk2u_unofficial_adv-3.0 -lwx_gtk2u_unofficial_core-3.0 -lwx_baseu_unofficial_xml-3.0 -lwx_baseu_unofficial_net-3.0 -lwx_baseu_unofficial-3.0   ../cryptopp562/libcryptopp.a

install:
	install Entangle /usr/local/bin
uninstall:
	rm -rf /usr/local/bin/Entangle
clean:
	rm -rf *.o Entangle
