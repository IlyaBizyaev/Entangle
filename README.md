# Entangle v. 0.9
![Entangle Logo](http://entangle.ucoz.net/text_logo.png)

![Screenshot](http://entangle.ucoz.net/Screenshots/Entangle_0.9.png)

[![Build Status](https://travis-ci.org/IlyaBizyaev/Entangle.svg?branch=unstable)](https://travis-ci.org/IlyaBizyaev/Entangle)

## Description
Entangle is a simple application, designed to help users protect their private and personal data in a few clicks.
Entangle is built using wxWidgets and Crypto++ libraries, which makes it cross-platform.

**Project website:** http://entangle.ucoz.net/
## Building from sources
### Linux
To build the application on Linux, you need to install the following packages:
- `libwxgtk3.0` and `libwxgtk3.0-dev`
- `libcrypto++9` and `libcrypto++-dev`

**Note.** On some distributions, Cryto++ packages may be called `libcryptopp`.

### Windows
On Windows, you have to download wxWidgets and Crypto++ libraries from official websites and build them manually.

**Supported compilers:** GCC, MinGW.

### Compilation
```
cd ~/Downloads/Entangle-unstable/ # Depends on the location
make
sudo make install
make clean
```
**Note.** You may need to redefine `WXDIR` and `CRYPTDIR` constants on Windows.

## Note
For now, this application is under active development, which means it is not for public usage; so, the "master" branch is empty. However, you can see the pre-release code in the "unstable" branch.
Any fixes and improvements are appreciated!
## License
Entangle is distributed under the GNU GPL v3 license; however, I STRONGLY recommend not to make forks or use this code in any other applications until first official release.
Please notify me if you want to use this code in any projects.
## Thanks to...
...**DenDev** for the event processing algorithm.

...**doublemax** for the progress bar.

...**Jean-Pierre Münch**, **Jeffrey Walton** and **Mobile Mouse** for help with the encryption.

...**RostakaGmfun** for improving the Makefile and Ukrainian translation.
## Copyright
Copyright (C) Ilya Bizyaev <bizyaev@lyceum62.ru>, 2015.
