/***************************************************************
 * Name:      EntangleExtras.h
 * Purpose:   Defines Entangle's Extra Modules
 * Author:    Ilya Bizyaev (bizyaev.game@yandex.ru)
 * Created:   2015-05-15
 * Copyright: Ilya Bizyaev (utor.ucoz.ru)
 * License:   GNU GPL v3
 **************************************************************/

#ifndef ENTANGLE_EXTRAS_H
#define ENTANGLE_EXTRAS_H

#include <fstream>              //File operations
#include <wx/dnd.h>             //File drag & drop
#include <cryptopp/simple.h>    //Basics of Crypto++
#include <cryptopp/osrng.h>     //Random generator


#include "EntangleMain.h"

#define ENTANGLE_CORE 3

typedef unsigned char byte;
using namespace std;
using namespace CryptoPP;

//My lovely structures ^_^

/* Header; written to encrypted files before main data */
struct Header
{
    Header() {   }
    Header(unsigned long long file_size);
    int core_version;                   /* Header format version */
    unsigned long long file_size;       /* Size of original file */
    byte keys[32];                      /* AES-256 key storage area */
};

/* A simple class which accepts dropped files */
class DroppedFilesReciever : public wxFileDropTarget
{
public:
	//Constructor
    DroppedFilesReciever(EntangleDialog * g_dialog);
    //Called when something is dropped onto the window
    bool OnDropFiles(wxCoord x, wxCoord y, const wxArrayString &filenames);
private:
    EntangleDialog * dialog; //Pointer to the dialog, needed to refresh the StaticText
};

/* A simple wrapper for C++ file streams */
class BinFile
{
public:
    BinFile();
    BinFile(wxString & filename, ios_base::openmode file_mode);
    wxString GetName();
    void open(wxString & filename, ios_base::openmode file_mode);
    bool seek_start();
    bool read(byte* data, int size);
    bool write(const byte* data, int size, bool flush = false);
    bool is_open(); void close(); ~BinFile();

private:
    fstream cfile;
    ios_base::openmode mode;
    wxString name;
    bool IsOk;
};

/* A sink class; writes data from a AES/GCM filter to a binary file */
class EntangleSink : public Bufferless<Sink>
{
public:
    //Constructor (accepts link to a BinFile object)
    EntangleSink(BinFile & g_output) : output(g_output) {  }
    //Destructor
    ~EntangleSink() {  }
    //Function which accepts data from AES/GCM and writes to the file
    size_t Put2(const byte *inString, size_t length, int, bool);
private:
    BinFile & output;
};

/* A simple class which provides access to a random pool */
/* and implements its own methods, based on this RNG.    */
class RandomGenerator
{
public:
    //Crypto++ method's wrapper
    void GenerateBlock(byte * output, size_t size);
    //Generates a random number in the given range
    unsigned int RandomNumber(int num_min, int num_max);
    //Makes up a name for a temp file
    void RandTempName(wxString & temp_name);
private:
    //System random generator
    static AutoSeededRandomPool rnd;
};

#endif // ENTANGLE_EXTRAS_H
