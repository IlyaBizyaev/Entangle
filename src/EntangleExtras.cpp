/***************************************************************
 * Name:      EntangleExtras.cpp
 * Purpose:   Implements Entangle's Extra Modules
 * Author:    Ilya Bizyaev (bizyaev.game@yandex.ru)
 * Created:   2015-01-01
 * Copyright: Ilya Bizyaev (utor.ucoz.ru)
 * License:   GNU GPL v3
 **************************************************************/

#include "EntangleExtras.h"
#include "EntangleApp.h"
using namespace std;

/* Header's constructor */
Header::Header(unsigned long long fsize)
{
    //Clean the header
    memset(this, 0x00, sizeof(Header));
    //Set the file size
    file_size = fsize;
    //Write the version of my lovely program \(^_^)/
    core_version = ENTANGLE_CORE;
    //Generate random keys
    AutoSeededRandomPool rnd;
    rnd.GenerateBlock(keys, 32);
}

/* DroppedFilesReceiver's methods */
//Constructor;
DroppedFilesReciever::DroppedFilesReciever(EntangleDialog * g_dialog) { dialog = g_dialog; }

//Called when something is dropped onto the window
bool DroppedFilesReciever::OnDropFiles(wxCoord x, wxCoord y, const wxArrayString &filenames)
{
    //Save paths to the global array
    dialog->AddDropped(filenames);
    //Update the UI
    dialog->UpdateTasks();
    return true;
}


/* EntangleSink's Put2() method */
//Function which accepts data from AES/GCM and writes to the file
size_t EntangleSink::Put2(const byte *inString, size_t length, int, bool)
{
	if(!inString || !length) return length;
	//Writing the data
	output.write(inString, length);
	return 0;
}

/* BinFile wrapper's methods */

//Constructor
BinFile::BinFile() { IsOk = false; }

//Constructor (accepts name and open mode, calls open()
BinFile::BinFile(wxString & filename, ios_base::openmode file_mode)
{
    open(filename, file_mode | ios_base::binary);
}

//Destructor, calls close()
BinFile::~BinFile() { close(); }

wxString BinFile::GetName() { return name; }

//Opens the file and checks for success
void BinFile::open(wxString & filename, ios_base::openmode file_mode)
{
    cfile.open(filename, file_mode);
    if(!cfile.is_open()) { IsOk = false; return; }
    name = filename; mode = file_mode; IsOk = true;
}

bool BinFile::seek_start()
{
    try
    {
        cfile.seekp(0, ios::beg);
    }
    catch(ios_base::failure & except)
    {
        return false;
    }
    return true;
}

//Reads data from the file
bool BinFile::read(byte* data, int size)
{
    if(!(mode & ios_base::in)) return false;
    cfile.read((char*)data, size);
    return true;
}

//Writes data to the file
bool BinFile::write(const byte* data, int size, bool flush)
{
    if(!(mode & ios_base::out)) return false;
    try
    {
        cfile.write(reinterpret_cast<const char*>(data), size);
        if(flush) cfile.flush();
    }
    catch(ios_base::failure & except)
    {
        return false;
    }
    return true;
}

//Checks the status
bool BinFile::is_open() { return IsOk; }

//Closes the file
void BinFile::close() { cfile.close(); }
