/***************************************************************
 * Name:      Encryption.h
 * Purpose:   Declares extras, used for encryption
 * Author:    Ilya Bizyaev (bizyaev@lyceum62.ru)
 * Created:   26.04.2016
 * Copyright: Ilya Bizyaev
 * License:   GNU GPL v3
 **************************************************************/

#ifndef ENCRYPTION_H
#define ENCRYPTION_H

/** ------------ Include files ------------ **/
#include "Data.h"

#include <cryptopp/simple.h>    //Basics of Crypto++
#include <cryptopp/osrng.h>     //Random generator
/** --------------------------------------- **/

using namespace std;
using namespace CryptoPP;


/* Header; written to encrypted files before main data */
struct Header
{
    Header(ullong file_size = 0);
	//Data
    byte key[KEY_SIZE];     /* AES-256 key storage area */
    ullong file_size;       /* Size of original file */
    int core_version;       /* Header format version */
};

//Checks if header's versions matches program's one
bool CheckHeader(Header & header);


/* A sink class; writes data from a AES/GCM filter to a binary file */
class EntangleSink : public Bufferless<Sink>
{
public:
    EntangleSink(BinFile & g_output) : output(g_output) {  }
    //Function which accepts data from AES/GCM and writes to the file
    size_t Put2(const byte *inString, size_t length, int, bool);
    static bool write_failed();
private:
    BinFile & output;
    static bool m_fail;
};


/* A simple class which provides access to a random pool */
/* and implements its own methods, based on this RNG.    */
class RNG
{
public:
    //Crypto++ method's wrapper
    static void GenerateBlock(byte * output, size_t size);
    //Generates a random number in the given range
    static unsigned int RandomNumber(int min_val, int max_val);
    //Makes up a name for a temp file
    static wxString TempName(const wxString & location);
private:
    //System random generator
    static AutoSeededRandomPool rnd;
};

#endif // ENCRYPTION_H
