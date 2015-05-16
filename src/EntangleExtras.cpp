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


/* EntangleSink's methods */
//Constructor (accepts pointer to an array and to size_t variable)
EntangleSink::EntangleSink(byte ** g_output, size_t & g_size) : output(g_output), out_size(g_size) { Clean(); }

//Destructor
EntangleSink::~EntangleSink() { Clean(); }

// Function which accepts data from AES/GCM and puts to the linked array
size_t EntangleSink::Put2(const byte *inString, size_t length, int, bool)
{
	if(!inString || !length) return length;
	//Reallocating the array
	*output = (byte*)realloc(*output, out_size+length);
	//Adding new data
	byte * WhereToJoin = *output + out_size;
	memcpy(WhereToJoin, inString, length);
	//Updating the size
	out_size+=length;
	return 0;
}

// Clean(): deallocates the array and resets the out_size variable.
void EntangleSink::Clean()
{
	if(*output!=NULL)
	{
	    free(*output);
	    *output = NULL;
	}
	out_size=0;
}


/* EFile wrapper's methods */
//Constructor
EFile::EFile() { IsOk = false; }

//Constructor (accepts name and open mode, calls open()
EFile::EFile(wxString filename, ios_base::openmode file_mode)
{ open(filename, file_mode); }

//Destructor, calls close()
EFile::~EFile() { close(); }

//Opens the file and checks for success
void EFile::open(wxString filename, ios_base::openmode file_mode)
{
    cfile.open(filename, file_mode);
    if(!cfile.is_open()) { IsOk = false; return; }
    name = filename; mode = file_mode; IsOk = true;
}

//Reads data from the file
bool EFile::read(byte* data, int size)
{
    if(!(mode & ios_base::in)) return false;
    cfile.read((char*)data, size);
    return true;
}

//Writes data to the file
bool EFile::write(const byte* data, int size)
{
    if(!(mode & ios_base::out)) return false;
    cfile.write(reinterpret_cast<const char*>(data), size);
    return true;
}

//Checks the status
bool EFile::is_open() { return IsOk; }

//Closes the file
void EFile::close() { cfile.close(); }
