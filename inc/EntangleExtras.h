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

#include <cryptopp/simple.h> //Some basic declarations
#include <wx/dnd.h> //File drag & drop
#include "EntangleMain.h"
typedef unsigned char byte;
using namespace std;
using namespace CryptoPP;

//My lovely structures ^_^

/* Header; written to encrypted files before main data */
struct Header
{
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

/* A sink class; implements data exchange between AES/GCM and C++ arrays */
class EntangleSink : public Bufferless<Sink> /* Array-based sink class for GCM */
{
public:
    //Constructor (accepts pointer to an array and to size_t variable)
    EntangleSink(byte ** g_output, size_t & g_size);
    //Destructor
    ~EntangleSink();
    // Function which accepts data from AES/GCM and puts to the linked array
    size_t Put2(const byte *inString, size_t length, int, bool);
    // Clean(): deallocates the array and resets the out_size variable.
    void Clean();
private:
    byte ** output;     //Pointer to an array
    size_t & out_size;  //Stores number of bytes in array
};

/* A simple wrapper for C++ file streams */
class EFile
{
public:
    EFile();
    void open(wxString filename, ios_base::openmode file_mode);
    EFile(wxString filename, ios_base::openmode file_mode);
    bool read(byte* data, int size);
    bool write(const byte* data, int size);
    bool is_open(); void close(); ~EFile();

private:
    fstream cfile;
    ios_base::openmode mode;
    wxString name;
    bool IsOk;
};

#endif // ENTANGLE_EXTRAS_H
