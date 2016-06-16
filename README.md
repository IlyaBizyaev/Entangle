# Entangle v. 1.0
![Logo](http://ilyabizyaev.github.io/Entangle/images/logo_with_text.png)  
![Screenshot](http://ilyabizyaev.github.io/Entangle/images/Entangle_1.0.png)  

## Description
**Entangle** is a simple and user-friendly application, designed to help users protect their private data in a few clicks using AES encryption. It can also be used to make backups, as it supports ZIP compression.  
Entangle is free and open source. It is built using wxWidgets and Crypto++ libraries, which makes it flexible and cross-platform.  
### Features:
- User-friendly minimalistic interface
- Supports Drag & Drop.
- Multilingual
- Cross-platform
- Lightweight
- Two modes: GUI and console

**Supported platforms:** Windows, Linux.  
**License:** GPL v3  
**Project website:** http://ilyabizyaev.github.io/Entangle/
## Building from sources
### Linux
To build the application on Linux, you need to install the following packages:
- `libwxgtk3.0` and `libwxgtk3.0-dev` for wxWidgets
- `libcrypto++9` and `libcrypto++-dev` for Crypto++  
It is also possible to use manually built libraries.

**Note.** On some distributions, Cryto++ packages may be called `libcryptopp`.

### Windows
On Windows, you have to download wxWidgets and Crypto++ libraries from official websites and build them manually.  
**Notes:**  
For wxWidgets, it is recommended to use MSYS and run `make install`.  
For Crypto++, it is recommended to locate it at `C:\cryptopp`.  
You may have to edit the Makefile otherwise.

**Supported compilers:** GCC (TDM-GCC on Windows).

### Compilation
```
cd ~/Entangle/ # Depends on the location
make
sudo make install
make clean
```

## Notes
Any feedback, improvements and fixes are appreciated!
Also, please, notify me if you want to use this code in any projects.
## Thanks to...
...**doublemax** and **DenDev** for answering my wxQuestions :)

...**Jean-Pierre Münch**, **Jeffrey Walton** and **Mobile Mouse** for help with the encryption.

...**RostakaGmfun** for testing and Ukrainian translation.
## Copyright
Copyright (C) Ilya Bizyaev <bizyaev@lyceum62.ru>, 2015-2016.
