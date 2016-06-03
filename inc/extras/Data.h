/***************************************************************
 * Name:      Data.h
 * Purpose:   Data related objects
 * Author:    Ilya Bizyaev (bizyaev@lyceum62.ru)
 * Created:   24.06.2016
 * Copyright: Ilya Bizyaev
 * License:   GNU GPL v3
 **************************************************************/

#ifndef DATA_H
#define DATA_H

/** ------------ Include files ------------ **/
#include <fstream>              //File operations
#include <wx/arrstr.h>          //wxArrayString
#include <wx/filename.h>        //File existence and permissions
#include <wx/utils.h>           //Showing spinner when shredding big files
/** --------------------------------------- **/

/** ------------- Definitions ------------- **/
#define ENTANGLE_CORE 4
#define BUF_SIZE 16384
#define TAG_SIZE 16
#define IV_SIZE 16
#define HEAD_KEY_SIZE 16
#define KEY_SIZE 32
/** --------------------------------------- **/

using namespace std;

typedef unsigned char byte;
typedef unsigned long long ullong;
enum MODE {Encrypt, Decrypt};

//Used to convert numbers to wxStrings
inline wxString ToString(int number) { return wxString::FromDouble(number); }

/* Contains everything that is provided by user */
struct UserData
{
    UserData();
    UserData(wxArrayString g_tasks, wxString g_password, MODE g_mode);
    //Data
    wxArrayString tasks;
    wxArrayString dirs;
    wxString password;
    MODE mode;
};

/* A simple array class that performs dynamic memory  */
/* management and casting to (byte*), which allows to */
/* use it as a usual array. */
class ByteArray
{
public:
    ByteArray(unsigned long size);
    ~ByteArray();
    //Typecast operator
    operator byte*();
    //Getter
    unsigned long size();
private:
    byte * data;
    unsigned long m_size;
};

/* A wrapper for C++ file streams and wxWidgets file functions */
class BinFile
{
public:
    //Constructor and opening function
    BinFile(const wxString & filename, ios_base::openmode g_mode);
    void open(const wxString & filename, ios_base::openmode g_mode);
    //Getters
    wxString GetPath(); ullong size();
    bool is_open(); wxString why_failed();
    //Basic file operations
    void seek_start();
    void read(byte* data, int size);
    void read(ByteArray & data);
    void write(const byte* data, int size);
    void write(ByteArray & data);
    void flush();
    //Advanced actions
    bool rename(const wxString & new_name);
    bool remove(); bool shred();
    //Closing and destruction
    void close(); ~BinFile();
private:
    fstream cfile;
    ios_base::openmode mode;
    wxString path, fail_reason;
    bool IsOk;
};

void AddSlashIfNecessary(wxString & path);
ullong GetFileSize(const wxString & path);
//A function that traverses the path array (expands all paths)
wxArrayString Traverse (wxArrayString & input);

/* Block operations logic */
class BlockReader
{
public:
    BlockReader(ullong to_process);
    //Block operations
    int block_size();
    bool next();
private:
    int blocks, tail;
};

#endif // DATA_H

