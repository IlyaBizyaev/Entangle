/***************************************************************
 * Name:      EntangleExtras.cpp
 * Purpose:   Implements Entangle's Extra Modules
 * Author:    Ilya Bizyaev (bizyaev.game@yandex.ru)
 * Created:   2015-01-01
 * Copyright: Ilya Bizyaev (utor.ucoz.ru)
 * License:   GNU GPL v3
 **************************************************************/

#include "EntangleExtras.h"

#include <wx/filename.h>        //File existence and permissions
#include <wx/log.h>             //Error log in GUI mode

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
    RandomGenerator rnd;
    rnd.GenerateBlock(keys, 32);
}

/* ErrorTracker's static variables and methods */
wxArrayString ErrorTracker::errors;
bool ErrorTracker::WentWrong = false;
bool ErrorTracker::console = false;

void ErrorTracker::AddError(wxString filename, wxString message)
{
    //Producing a human-readable output
    //and pushing the result to the main list.
    wxString info = filename+" ("+message+")";
    if(console)
        errors.push_back(info);
    else
        wxLogError(info);
    WentWrong = true;
}

bool ErrorTracker::HasIssues() { return WentWrong; }

void ErrorTracker::ShowIssues()
{
    if(HasIssues())
    {
        if(console)
        {
            cout << "Something went wrong:" << endl;
            for(size_t i=0; i < errors.GetCount(); ++i)
                cout << i+1 << ": " << wxString(errors[i]).ToAscii() << endl;
        }
        else
        {
            wxLogError(_("Something went wrong:"));
            wxLog::FlushActive();
        }
        WentWrong = false;
    }
}

/* DroppedFilesReceiver's methods */
//Constructor;
DroppedFilesReciever::DroppedFilesReciever(EntangleFrame * g_dialog) { dialog = g_dialog; }

//Called when something is dropped onto the window
bool DroppedFilesReciever::OnDropFiles(wxCoord x, wxCoord y, const wxArrayString &filenames)
{
    //Copy paths to the dialog
    dialog->AddDropped(filenames);
    return true;
}

/* ProgressDisplayer's methods */
//Constructor
ProgressDisplayer::ProgressDisplayer(EntangleFrame * g_frame)
{
    frame = g_frame;
    text = _("Starting...");
    current = 0; total = 0; progress = 0;
    if(frame!=NULL) frame->UpdateProgress(progress, text);
}

//Destructor
ProgressDisplayer::~ProgressDisplayer()
{
    if(frame==NULL) cout << "\r" << text + wxS("... 100%") << "\n";
}

//3 setters
void ProgressDisplayer::SetTotal(ullong g_total) { total = g_total; }
void ProgressDisplayer::SetText(wxString g_text)
{
    if(frame==NULL)
    {
        cout << "\r" << text + wxS("... 100%") << "\n";
        cout << g_text + wxS("... 0%");
    }
    else
        frame->UpdateProgress(progress, g_text);
    text = g_text;
}
void ProgressDisplayer::IncreaseCurrent(ullong to_add)
{
    current+=to_add;
    CalcProgress();
    //Updating the progress
    if(frame == NULL) //Console mode
        cout << "\r" << text+wxS("... ")+ToString(progress)+wxS("%");
    else //GUI mode
        frame->UpdateProgress(progress);
}

//Actual progress calculator
void ProgressDisplayer::CalcProgress()
{
    //Dividing by zero is a crime!
    assert(total!=0);
    progress = (double)current/total*100;
    if(progress > 100) progress = 100;
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

//Constructor (accepts name and open mode)
BinFile::BinFile(wxString & filename, ios_base::openmode file_mode)
{
    cfile.open(filename, file_mode | ios_base::binary);
    if(!cfile.is_open()) { IsOk = false; return; }
    name = filename; mode = file_mode; IsOk = true;
}

//Destructor, calls close()
BinFile::~BinFile() { close(); }

wxString BinFile::GetName()
{
    if(IsOk)
        return name;
    else
        return wxEmptyString;
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

/* RandomGenerator's methods */
AutoSeededRandomPool RandomGenerator::rnd;

void RandomGenerator::GenerateBlock(byte * output, size_t size)
{
    rnd.GenerateBlock(output, size);
}

unsigned int RandomGenerator::RandomNumber(int num_min, int num_max)
{
    unsigned int result;
    GenerateBlock((byte*)&result, sizeof(unsigned int));
    result = result % (num_max-num_min+1) + num_min;
    return result;
}

wxString RandomGenerator::RandTempName(wxString location)
{
    wxString temp_name;
    do //While such file exists
    {
        //Random filename length (1 - 20):
        int length = RandomNumber(1, 20);
        //Creating new char buffer for the filename
        Array<char> filename(length+1);
        //Filling the array (a-z, A-Z, 0-9):
        for(int i=0; i<length; ++i)
        {
            int range = RandomNumber(1, 3);
            if(range==1) //Number (0-9):
                filename[i] = RandomNumber(48, 57);
            else if(range==2) //Capital letter (A-Z):
                filename[i] = RandomNumber(65, 90);
            else if(range==3) //Small letter (a-z):
                filename[i] = RandomNumber(97, 122);
        }
        //Writing zero character to the end
        filename[length] = '\0';
        //Building the full path
        temp_name = location + wxString(filename);
    } while(wxFileExists(temp_name));
    //Returning filename and finishing
    return temp_name;
}

/* Array's methods */
template<typename T>
T& Array<T>::operator[] (unsigned long Index)
{
    assert(Index>=0);
    assert(Index<m_size);
    assert(data!=NULL);
    return data[Index];
}
