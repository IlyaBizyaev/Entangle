/***************************************************************
 * Name:      Data.cpp
 * Purpose:   Implements data related objects
 * Author:    Ilya Bizyaev (bizyaev@lyceum62.ru)
 * Created:   26.04.2016
 * Copyright: Ilya Bizyaev
 * License:   GNU GPL v3
 **************************************************************/

#include "extras/Data.h"
#include "extras/UI.h"

#include <wx/filename.h>        //File existence and permissions
using namespace std;


/* UserData's constructors */
UserData::UserData() : mode(Encrypt) { }

UserData::UserData(wxArrayString g_tasks, wxString g_password, MODE g_mode)
: tasks(Traverse(g_tasks)), password(g_password), mode(g_mode)
{
    //Collecting directories (because we may need to remove them)
    for(size_t i=0; i<g_tasks.size(); ++i)
        if(wxDirExists(g_tasks[i]))
        {
            //By wxFileName's logics, there should be a trailing
            //separator at the end of directory's path.
            AddSlashIfNecessary(g_tasks[i]);
            dirs.push_back(g_tasks[i]);
        }
    tasks = Traverse(g_tasks);
}


/* ByteArray's methods */
//Constructor
ByteArray::ByteArray(unsigned long size)
{
    try
    {
        data = new byte[size];
        m_size = size;
    }
    catch(...)
    {
        wxFAIL_MSG("Could not allocate " + ToString(size) + " bytes.");
        data = NULL; m_size = 0;
    }
}

//Destructor
ByteArray::~ByteArray()
{
    if(data!=NULL) delete[] data;
}

ByteArray::operator byte*()
{
    assert(data!=NULL);
    return data;
}

unsigned long ByteArray::size() {return m_size; }


/* BinFile's methods */
BinFile::BinFile(const wxString & filename, ios_base::openmode g_mode) : IsOk (false) { open(filename, g_mode); }

void BinFile::open(const wxString & filename, ios_base::openmode g_mode)
{
    try
    {
        if(is_open()) close();
        cfile.exceptions(cfile.exceptions() | ios_base::failbit);
        cfile.open(filename, g_mode | ios_base::binary);
    }
    catch(ios_base::failure & e)
    {
        fail_reason = e.what();
        return;
    }
    path = filename; mode = g_mode; IsOk = true;
}

BinFile::~BinFile() { close(); }

wxString BinFile::GetPath() { return path; }

void BinFile::seek_start()
{
    cfile.seekp(0, ios::beg);
}

//Reads data from the file
void BinFile::read(byte* data, int size)
{
    assert(mode & ios_base::in);
    cfile.read((char*)data, size);
}

void BinFile::read(ByteArray & data) { this->read(data, data.size()); }

//Writes data to the file
void BinFile::write(const byte* data, int size)
{
    assert(mode & ios_base::out);
    cfile.write((const char*)(data), size);
}

void BinFile::write(ByteArray & data) { this->write(data, data.size()); }

void BinFile::flush() {  cfile.flush(); }

bool BinFile::rename(const wxString & new_name)
{
    try
    {
        wxString old_name = GetPath();
        close();
        wxRenameFile(old_name, new_name);
    }
    catch(...)
    {
        return false;
    }
    return true;
}

bool BinFile::remove()
{
    wxString name = GetPath();
    if(is_open()) close();
    //If there is already no such file, terminate.
    if(!wxFileExists(name)) return true;
    //if read-only, make writable
    if(!wxFileName::IsFileWritable(name))
    {
        wxFileName fname(name);
        if(!fname.IsOk()) return false;
        //Setting read-write permissions
        fname.SetPermissions(wxS_IRUSR | wxS_IWUSR);
    }
    return wxRemoveFile(name);
}

/** Shreds files **/
/* TODO, FIXME, WARNING - treat it as you want: */
/* This does *not* always work properly on different medium. */
/* Due to hardware features and lots of ways to recover the data, */
/* it is close to impossible to securely shred files! */
/* Also, shredding big files may harm the storage device: e.g. SSDs/ */
/* USE AT YOUR OWN RISK! */
bool BinFile::shred()
{
    //Opening this file for writing
    if(!(mode & ios_base::out))
        open(GetPath(), ios_base::out);
    if(!is_open()) return false;

    static ByteArray buffer(BUF_SIZE);

    /**--------------------------------------------**/
    for(int i=0; i<10; ++i) // 10 iterations
    {
        //Generating random data to write
        RNG::GenerateBlock(buffer);
        //Moving to the beginning
        seek_start();
        //Overwriting the data,
        //ensuring it is written, not cached
        BlockReader rdr(size());
        do
        {
            write(buffer, rdr.block_size());
            flush();
        } while(rdr.next());
        wxYield(); //For big files
    }
    /**--------------------------------------------**/
    //Closing the file
    close();
    return true;
}

ullong BinFile::size()
{
    return GetFileSize(path);
}

//Checks the status
bool BinFile::is_open() { return IsOk; }

//Returns error text on fail
wxString BinFile::why_failed()
{
    wxString to_ret = fail_reason;
    fail_reason = "";
    return to_ret;
}

//Closes the file
void BinFile::close()
{
    if(IsOk)
        cfile.close();
    path = ""; IsOk = false;
}

void AddSlashIfNecessary(wxString & path)
{
    if(!path.EndsWith(wxFileName::GetPathSeparator()))
        path+=wxFileName::GetPathSeparator();
}

ullong GetFileSize(const wxString & path)
{
    //If file does not exist
    if(!wxFileExists(path)) return ULLONG_MAX;
    //Getting file size
    wxULongLong result = wxFileName::GetSize(path);
    //If everything is OK
    if(result!=wxInvalidSize) return result.GetValue();
    //Otherwise
    return ULLONG_MAX;
}

wxArrayString Traverse(wxArrayString & input)
{
    wxArrayString result;
    //For each task
    for(size_t pos = 0; pos < input.size(); ++pos)
    {
        /* If the object does not exist */
        if(!wxFileName::Exists(input[pos]))
        {
            Issues::SetFile(input[pos]);
            Issues::Add(_("Does not exist"));
        }
		/* Checking object type */
		else if(wxDirExists(input[pos]))// If that's folder, scan all subdirectories and files inside it.
			wxDir::GetAllFiles(input[pos], &result);
		else//If that's file, simply push it to the vector.
			result.push_back(input[pos]);
    }
    return result;
}

/* BlockReader's methods */
BlockReader::BlockReader(ullong to_process)
{
    tail = to_process % BUF_SIZE;
    blocks = (to_process - tail)/BUF_SIZE;
}

int BlockReader::block_size()
{
    return blocks ? BUF_SIZE : tail;
}

bool BlockReader::next()
{
    if(blocks)
    {
        --blocks;
        return true;
    }
    return false;
}
