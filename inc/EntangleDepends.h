#ifndef ENTANGLE_DEPENDS_H
#define ENTANGLE_DEPENDS_H

#include<fstream>  //Read/write operations
#include<memory> //Memory copying
#include<wx/filename.h>//Useful class
#include<wx/aboutdlg.h>
#include<wx/msgdlg.h>

#ifdef __WIN32__
#include<aes.h> //AES cryptography
#include<osrng.h>//Random IV
#include<modes.h>//That's clear ;)
#include<filters.h>
#include<gcm.h>
#include<cryptlib.h>
#include<pwdbased.h>
#include<sha.h>
#else
#include<cryptopp/aes.h> //AES cryptography
#include<cryptopp/osrng.h>//Random IV
#include<cryptopp/filters.h>
#include<cryptopp/gcm.h>
#include<cryptopp/modes.h>
#include<cryptopp/cryptlib.h>
#include<cryptopp/pwdbased.h>
#include<cryptopp/sha.h>
#endif // __WIN32__

#endif // ENTANGLE_DEPENDS_H

