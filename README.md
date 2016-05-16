# Entangle v. 1.0
![Entangle Logo](http://entangle.ucoz.net/text_logo3.png)  
![Screenshot](http://entangle.ucoz.net/Screenshots/Entangle_1.0.png)  

## Description
**Entangle** is a simple application, designed to help users protect their private data in a few clicks using AES encryption.  
It can also be used to make backups, as it supports ZIP compression.  
**Entangle** is built using wxWidgets and Crypto++ libraries, which makes it cross-platform.
### Features:
- User-friendly
- Multilingual
- Cross-platform
- Lightweight
- Two modes: GUI and console

**Project website:** http://entangle.ucoz.net/
## Building from sources
[![Build Status](https://travis-ci.org/IlyaBizyaev/Entangle.svg?branch=master)](https://travis-ci.org/IlyaBizyaev/Entangle)
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

## Note
Any feedback, improvements and fixes are appreciated!
## License
Entangle is distributed under the GNU GPL v3 license.
But please, notify me if you want to use this code in any projects.
## Thanks to...
...**doublemax** and **DenDev** for answering my wxQuestions :)

...**Jean-Pierre Münch**, **Jeffrey Walton** and **Mobile Mouse** for help with the encryption.

...**RostakaGmfun** for testing and Ukrainian translation.
## Copyright
Copyright (C) Ilya Bizyaev <bizyaev@lyceum62.ru>, 2015-2016.
