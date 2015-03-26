/***************************************************************
 * Name:      EntangleMain.cpp
 * Purpose:   Code for Application Frame
 * Author:    Ilya Bizyaev (bizyaev.game@yandex.ru)
 * Created:   2015-01-01
 * Copyright: Ilya Bizyaev (utor.ucoz.ru)
 * License:   GNU GPL v3
 **************************************************************/
#include "EntangleMain.h"
#include <wx/msgdlg.h>
#include "EntangleApp.h"
#include<fstream>  //Read/write operations
#include<string>   //Needed for mode check (ONCE!)
#include<stdio.h>  //For file renaming
#include<clocale>  //For languages support
#include<wx/dnd.h> //File drag & drop!

#ifdef __WIN32__
#include "wx_pch.h"
#include<aes.h> //AES cryptography
#include<osrng.h>//Random IV
#include<modes.h>//That's clear ;)
#include<filters.h>
#include<gcm.h>
#include<cryptlib.h>
#else
#include<dirent.h> //File browsing (UNIX ONLY!!!)
#include<cryptopp/aes.h> //AES cryptography
#include<cryptopp/osrng.h>//Random IV
#include<cryptopp/filters.h>
#include<cryptopp/gcm.h>
#include<cryptopp/modes.h>
#include<cryptopp/cryptlib.h>
#endif // __WIN32__

#define BUF_SIZE 16384
#define TAG_SIZE 16
#define ENTANGLE_CORE 1
using namespace std;
using namespace CryptoPP;

unsigned long long GetFileSize(wxString path); //Works on Unix
void AddError(wxString, wxString);
bool SmartRemove(wxString path, bool safe = false);
bool SmartRename(wxString before, wxString after);
void GoodFinish(fstream&, fstream&, wxString&, wxString&);

int NumFiles=0, ShowProgress=0;
unsigned long long Total=0, NumBytes=0;
bool NowWorking=false;
locale current("");
wxArrayString drop_files, mistakes;
wxString show_str="Starting...";
AutoSeededRandomPool rnd;

//My lovely structures ^_^
struct Header
{
    int prog_version;           /* Header format version */
    unsigned long file_size;    /* Size of original file */
    byte keys[32];              /* AES-256 key storage area */
};

class EntangleSink : public Bufferless<Sink>
{
public:
    EntangleSink(byte ** g_output, size_t * g_size) : output(g_output), out_size(g_size)
    {   Clean();  }

    ~EntangleSink()
    {   Clean();  }

    size_t Put2(const byte *inString, size_t length, int, bool)
    {
        if(!inString || !length)
			return length;
        //Reallocating the array
        *output = (byte*) realloc(*output, *out_size+length);
        //Adding new data
        byte * WhereToJoin = *output + *out_size;
        memcpy(WhereToJoin, inString, length);
        //Updating the size
        *out_size+=length;
        return 0;
    }

    void Clean()
    {
        if(*output!=NULL)
        {
            free(*output);
            *output = NULL;
        }
        *out_size=0;
    }
private:
    byte ** output;
    size_t * out_size;
};


//(*InternalHeaders(EntangleDialog)
#include <wx/string.h>
#include <wx/intl.h>
//*)

//(*IdInit(EntangleDialog)
const long EntangleDialog::ID_STATICTEXT1 = wxNewId();
const long EntangleDialog::ID_GENERICDIRCTRL1 = wxNewId();
const long EntangleDialog::ID_STATICTEXT2 = wxNewId();
const long EntangleDialog::ID_TEXTCTRL1 = wxNewId();
const long EntangleDialog::ID_BUTTON2 = wxNewId();
const long EntangleDialog::ID_BUTTON1 = wxNewId();
const long EntangleDialog::ID_MESSAGEDIALOG1 = wxNewId();
const long EntangleDialog::ID_PROGRESSDIALOG1 = wxNewId();
//*)

BEGIN_EVENT_TABLE(EntangleDialog,wxDialog)
    //(*EventTable(EntangleDialog)
    //*)
END_EVENT_TABLE()

class DroppedFilesReciever : public wxFileDropTarget
{
public:
    DroppedFilesReciever(EntangleDialog * g_dialog) {dialog=g_dialog;}
    bool OnDropFiles(wxCoord x, wxCoord y, const wxArrayString &filenames)
    {
        for(unsigned int i=0; i<filenames.GetCount(); ++i) drop_files.push_back(filenames[i]);
        dialog->StaticText2->SetLabelText(_("Received ")+wxString::FromDouble(drop_files.GetCount())+_(" file(s)"));
        return true;
    }
private:
    EntangleDialog * dialog;
};


//wxDIRCTRL_MULTIPLE
//_("Simple application for protecting\nyour personal data\n(C) Ilya Bizyaev, 2015")+"\nbizyaev@lyceum62.ru"
EntangleDialog::EntangleDialog(wxWindow* parent,wxWindowID id)
{
    //(*Initialize(EntangleDialog)
    wxFlexGridSizer* FlexGridSizer1;
    wxBoxSizer* BoxSizer1;

    Create(parent, wxID_ANY, _("Entangle"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE, _T("wxID_ANY"));
    SetFocus();
    FlexGridSizer1 = new wxFlexGridSizer(5, 1, 0, 0);
    StaticText1 = new wxStaticText(this, ID_STATICTEXT1, _("Choose files or folders:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
    FlexGridSizer1->Add(StaticText1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    GenericDirCtrl1 = new wxGenericDirCtrl(this, ID_GENERICDIRCTRL1, wxEmptyString, wxDefaultPosition, wxSize(190,190), wxDIRCTRL_MULTIPLE, wxEmptyString, 0, _T("ID_GENERICDIRCTRL1"));
    FlexGridSizer1->Add(GenericDirCtrl1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, wxDLG_UNIT(this,wxSize(5,0)).GetWidth());
    StaticText2 = new wxStaticText(this, ID_STATICTEXT2, _("Enter the password:"), wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE, _T("ID_STATICTEXT2"));
    FlexGridSizer1->Add(StaticText2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    TextCtrl1 = new wxTextCtrl(this, ID_TEXTCTRL1, wxEmptyString, wxDefaultPosition, wxSize(150,28), 0, wxDefaultValidator, _T("ID_TEXTCTRL1"));
    FlexGridSizer1->Add(TextCtrl1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    BoxSizer1 = new wxBoxSizer(wxHORIZONTAL);
    AboutButton = new wxButton(this, ID_BUTTON2, _("About"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON2"));
    BoxSizer1->Add(AboutButton, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Button1 = new wxButton(this, ID_BUTTON1, _("Start"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON1"));
    Button1->SetFocus();
    BoxSizer1->Add(Button1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer1->Add(BoxSizer1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    SetSizer(FlexGridSizer1);
    MessageDialog1 = new wxMessageDialog(this, _("Simple application for protecting\nyour personal data\n(C) Ilya Bizyaev, 2015")+"\nbizyaev@lyceum62.ru", _("About"), wxOK|wxICON_INFORMATION, wxDefaultPosition);
    FlexGridSizer1->Fit(this);
    FlexGridSizer1->SetSizeHints(this);
    Center();

    Connect(ID_BUTTON2,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&EntangleDialog::OnAbout);
    Connect(ID_BUTTON1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&EntangleDialog::OnButton1Click);
    //*)
    GenericDirCtrl1->CollapseTree();
    locale::global(current);
    SetDropTarget(new DroppedFilesReciever(this));
}

EntangleDialog::~EntangleDialog()
{
    //(*Destroy(EntangleDialog)
    //*)
}


void EntangleDialog::OnAbout(wxCommandEvent& event)
{
    MessageDialog1->ShowModal();
}

void EntangleDialog::OnButton1Click(wxCommandEvent& event)
{
    NumFiles=0; NumBytes=0; Total=0; //Clear All
    //Get and check password
    wxString wxpassword = TextCtrl1->GetLineText(0);
    if(wxpassword.length()<16)
    {
        StaticText2->SetLabelText(_("Too short"));
        return;
    }
    else if(wxpassword.length()>16)
    {
        StaticText2->SetLabelText(_("Too long"));
        return;
    }
    //Assign password to the key
    byte key[16];
    const char * charKey = wxpassword.c_str().AsChar();
    memcpy(key, (byte*)charKey, 16);
    //Get tasks
    wxArrayString choice;
    GenericDirCtrl1->GetPaths(choice);
    //If there are no tasks
    if(choice.empty()&&drop_files.empty())
    {
        wxMessageBox(_("No tasks!"));
        return;
    }
    //If there are Dir-Control tasks
    for(size_t i=0; i<choice.GetCount(); ++i)
        Total+=GetFileSize(choice[i]);
    //If there are Drag&Drop tasks
    for(size_t i=0; i<drop_files.GetCount(); ++i)
        Total+=GetFileSize(drop_files[i]);

    StaticText2->SetLabelText(_("Waiting...")); wxYield();
    ProgressDialog1 = new wxProgressDialog(_("Progress"), _("Starting..."), 100, this);
    ProgressDialog1->CenterOnParent();
    ProgressDialog1->Show();
    ProgressDialog1->Update(0, _("Starting..."));

    NowWorking = true;
    //Usual files
    for(size_t i=0; i<choice.GetCount(); ++i)
        Process(choice[i], key);
    //Dropped ones
    for(size_t i=0; i<drop_files.GetCount(); ++i)
        Process(drop_files[i], key);
    ProgressDialog1->Update(100, _("Done!"));
    NowWorking = false; drop_files.Clear();

    if(!mistakes.empty())
    {
        wxString ErrorList = _("Some files were not processed:");
        for(size_t i=0; i<mistakes.GetCount(); ++i)
        {
            ErrorList+=("\n"+mistakes[i]);
            if(i==10) {ErrorList=ErrorList+_("\n...and ")+wxString::FromDouble(mistakes.GetCount()-10)+_(" more ones."); break;}
        }
        StaticText2->SetLabelText(_("Went wrong :("));
        wxMessageBox(ErrorList, _("Error list"), wxICON_ERROR | wxSTAY_ON_TOP);
        mistakes.Clear();
    }
    else
        StaticText2->SetLabelText(_("Complete (")+wxString::FromDouble(NumFiles)+_(" file(s))"));
}

void EntangleDialog::Process(wxString first, byte key[])
{
    #ifdef __WIN32__
    WIN32_FIND_DATA f;
    HANDLE h = FindFirstFile(wxString(first+"/*").wc_str() , &f);
    wxASSERT(Total!=0);
    /////////////////////FOLDER/////////////////////
    if(h!=INVALID_HANDLE_VALUE)
	{
		wxString fname; //Name of current file
		do
		{
			fname = f.cFileName;
			//Not this folder and not outer one
            if(fname!="."&&fname!="..")
                Process(first+"/"+fname, key);
		} while(FindNextFile(h, &f));
		CloseHandle(h);
	}
    #else
    DIR *dir = opendir(first.c_str()); //Dir opening attempt
    wxASSERT(Total!=0);
    if(dir)/////////////////FOLDER/////////////////
    {
        struct dirent *ent; //For reading filenames
        wxString fname; //Name of current file
        while((ent = readdir(dir))!=NULL) //While there are files
        {
            fname=ent->d_name; //For easiness
            //Not this folder and not the outer one.
            if(fname!="."&&fname!="..")
                Process(first+"/"+fname, key);
        }
        closedir(dir);
    }///////////////////////////////////////////////
    #endif // __WIN32__
    else//////////////////FILE//////////////////////
    {
        fstream In, Out;
        //Renaming file
        #ifdef __WIN32__
        size_t cut = first.find_last_of('\\');
        #else
        size_t cut = first.find_last_of('/');
        #endif // __WIN32__
        wxString temp_path = first.substr(0, cut+1)+"TMP.TMP";
        if(!SmartRename(first, temp_path)) return;
        //Opening files
        In.open(temp_path, ios_base::in|ios_base::binary);
        if(!In.is_open())
        {
            AddError(first, "BAD_IN_OPEN");
            return;
        }
        Out.open(first, ios_base::out|ios_base::trunc|ios_base::binary);
        if(!Out.is_open())
        {
            GoodFinish(In, Out, temp_path, first);
            AddError(first, "BAD_RES_CREATE");
            return;
        }
        /** <<<<<<< MOST IMPORTANT PART >>>>>>> **/
        string verify; In >> verify;
        unsigned long fsize, checker, dleft;
        if(verify=="ENTANGLE")
        {
            /**DECRYPTION**/
            show_str = _("Decrypting ")+first.substr(cut+1, first.Length()-cut-1);
            In.get();
            //Reserving space for IV, decrypted data and retrieved header
            byte iv[16]; byte * Retrieved; Header DecryptedHeader;
            //Reading the IV
            In.read((char*)&iv, 16);
            try
            {
                //New AES Decryption object
                GCM<AES>::Decryption d;
                //Setting key and IV
                d.SetKeyWithIV(key, 16, iv, sizeof(iv));
                //Reserving space for header and MAC and reading them
                byte blockky[64]; In.read((char*)&blockky, 64);
                //Creating new Decryption filter
                AuthenticatedDecryptionFilter df(d, NULL,
                    AuthenticatedDecryptionFilter::MAC_AT_END |
                    AuthenticatedDecryptionFilter::THROW_EXCEPTION, TAG_SIZE);
                //Putting the decrypted header to the filter
                df.ChannelPut("", (const byte*)&blockky, 64);
                // If the object throws, it most likely occurs here
                df.ChannelMessageEnd("");

                //Get data from channel
                df.SetRetrievalChannel("");
                size_t n = (size_t)df.MaxRetrievable();
                Retrieved = new byte[n];

                if(n > 0) { df.Get(Retrieved, n); }
                memcpy(&DecryptedHeader, Retrieved, n);
            }
           	catch(CryptoPP::InvalidArgument& e)
            {
                AddError(first, "INV_ARGUMENT");
                GoodFinish(In, Out, temp_path, first);
                return;
            }
            catch(CryptoPP::AuthenticatedSymmetricCipher::BadState& e)
            {
                // Pushing PDATA before ADATA results in:
                AddError(first, "BAD_STATE");
                GoodFinish(In, Out, temp_path, first);
                return;
            }
            catch(CryptoPP::HashVerificationFilter::HashVerificationFailed& e)
            {
                //Caught HashVerificationFailed
                AddError(first, "BAD_PASSWORD");
                GoodFinish(In, Out, temp_path, first);
                return;
            }

            /** CFB Mode Processing **/
            CFB_Mode<AES>::Decryption cfbDecryption(DecryptedHeader.keys, 32, iv);
            byte transfer[BUF_SIZE];
            fsize = DecryptedHeader.file_size; dleft = fsize%BUF_SIZE;
            checker = fsize-dleft;
            for(unsigned long i=0; i<checker; i+=BUF_SIZE)
            {
                In.read((char*)&transfer, BUF_SIZE);
                cfbDecryption.ProcessData((byte*)&transfer, (byte*)&transfer, BUF_SIZE);
                Out.write(reinterpret_cast<const char*>(&transfer), BUF_SIZE);
                NumBytes+=BUF_SIZE;
                wxTheApp->Yield();
                UpdateProgress();
            }
            if(dleft!=0)
            {
                In.read((char*)&transfer, BUF_SIZE);
                cfbDecryption.ProcessData((byte*)&transfer, (byte*)&transfer, BUF_SIZE);
                Out.write(reinterpret_cast<const char*>(&transfer), dleft);
                NumBytes+=dleft;
                wxTheApp->Yield();
                UpdateProgress();
            }

        }
        else
        {
            /** ENCRYPTION **/
            show_str = _("Encrypting ")+first.substr(cut+1, first.Length()-cut-1);
            In.seekg(0, ios::beg);
            Out << "ENTANGLE" << endl;
            //Creating, generating and writing the IV
            byte iv[16]; rnd.GenerateBlock(iv, sizeof(iv));
            Out.write(reinterpret_cast<const char*>(&iv), 16);
            //Creating and cleaning the Entangle Header
            Header MakeHeader; memset(&MakeHeader, 0x00, sizeof(MakeHeader));
            //Getting and writing the file size
            In.seekg(0, ios::end); fsize=In.tellg(); In.seekg(0, ios::beg);
            MakeHeader.file_size = fsize;
            //Writing the version of my lovely program \_(^_^)_/
            MakeHeader.prog_version = ENTANGLE_CORE;
            //Creating, generating and copying file encryption keys
            byte file_keys[32]; rnd.GenerateBlock(file_keys, 32);
            memcpy(MakeHeader.keys, file_keys, 32);
            /** Header is now ready for saving! **/
            /** Encrypting and writing the header! **/
            try
            {
                //New AES Encryption object
                GCM<AES>::Encryption e;
                //Setting user key and random IV
                e.SetKeyWithIV(key, 16, iv, sizeof(iv));
                //Preparing storage for results
                byte * Received = NULL;
                size_t GotSize = 0;
                //Filter with an EntangleSink
                AuthenticatedEncryptionFilter ef(e,
                new EntangleSink(&Received, &GotSize), false, TAG_SIZE);
                //Encrypting MakeHeader
                ef.ChannelPut("", (const byte*)&MakeHeader, sizeof(MakeHeader));
                ef.ChannelMessageEnd("");
                //Writing encrypted data
                Out.write(reinterpret_cast<const char*>(Received), GotSize);
            }
            catch(CryptoPP::BufferedTransformation::NoChannelSupport& e)
            {
                // The tag must go in to the default channel
                AddError(first, "NO_CH_SUPPORT");
                GoodFinish(In, Out, temp_path, first);
                return;
            }
            catch(CryptoPP::AuthenticatedSymmetricCipher::BadState& e)
            {
                // TODO: Get more info about this.
                AddError(first, "BAD_STATE");
                GoodFinish(In, Out, temp_path, first);
                return;
            }
            catch(CryptoPP::InvalidArgument& e)
            {
                AddError(first, "INV_ARGUMENT");
                GoodFinish(In, Out, temp_path, first);
                return;
            }

            /** CFB Mode Processing **/
            //Create Encryption
            CFB_Mode<AES>::Encryption cfbEncryption(file_keys, 32, iv);
            byte transfer[BUF_SIZE];
            dleft = fsize%BUF_SIZE;
            checker = fsize-dleft;
            //THE VERY PROCESS
            for(unsigned long i=0; i<checker; i+=BUF_SIZE)
            {
                In.read((char*)&transfer, BUF_SIZE);
                cfbEncryption.ProcessData((byte*)&transfer, (byte*)&transfer, BUF_SIZE);
                Out.write(reinterpret_cast<const char*>(&transfer), BUF_SIZE);
                NumBytes+=BUF_SIZE;
                wxTheApp->Yield();
                UpdateProgress();
            }
            if(dleft!=0)
            {
                rnd.GenerateBlock(transfer, BUF_SIZE);
                In.read((char*)&transfer, dleft);
                cfbEncryption.ProcessData((byte*)&transfer, (byte*)&transfer, BUF_SIZE);
                Out.write(reinterpret_cast<const char*>(&transfer), BUF_SIZE);
                NumBytes+=dleft;
                wxTheApp->Yield();
                UpdateProgress();
            }

        }
        /** <<<<<<< FINISHED MAIN PART >>>>>>> **/
        In.close(); Out.close();
        if(!SmartRemove(temp_path, true))
        {
            AddError(first, "BAD_TEMP_DELETE");
            return;
        }
        ++NumFiles;
    }
    return;
}

#ifdef __WIN32__
unsigned long long GetFileSize(wxString path)
{
    //E.g. "E:\Projects\" becomes "E:\Projects"
    if(path[path.length()-1]=='\\')
        path = path.substr(0, path.length()-1);
    //Declaring some objects
    WIN32_FIND_DATAW f;
    HANDLE h = FindFirstFileW(path.wc_str(), &f);
    //If filename is correct
    if (h!=INVALID_HANDLE_VALUE)
    {
        unsigned long long result = 0;
        /** DIRECTORY **/
        if(f.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        {
            FindClose(h);
            h = FindFirstFileW((path+L"\\*.*").wc_str(), &f);
            if(h == INVALID_HANDLE_VALUE)
            {
                AddError(path, "BAD_FS_FNAME");
                return -1;
            }
            wstring temp;
            do
            {
                temp = f.cFileName;
                if(temp!=L"."&&temp!=L"..")
                    result+=GetFileSize(path+L"\\"+temp);
            }while(FindNextFileW(h, &f));
        }
        else /** USUAL FILE **/
            result =(f.nFileSizeHigh * (MAXDWORD+1)) + f.nFileSizeLow;
        FindClose(h);
        return result;
    }
    else
    {
        AddError(path, "BAD_FS_FNAME");
        return -1;
    }
}
#else

unsigned long long GetFileSize(wxString path)
{
    unsigned long long fsize=0;
    DIR *dir = opendir(path.c_str()); //Dir opening attempt
    if(dir)/////////////////FOLDER/////////////////
    {
        struct dirent *ent; //For reading filenames
        wxString fname; //Name of current file
        while((ent = readdir(dir))!=NULL) //While there are files
        {
            fname=ent->d_name; //For easiness
            //Not this folder and not outer one
            if(fname!="."&&fname!="..")
                fsize+=GetFileSize(path+"/"+fname);
        }
        closedir(dir);
    }///////////////////////////////////////////////
    else
    {
        struct stat st;
        if(stat(path.c_str(), &st)==0)
            fsize = st.st_size;
    }
    return fsize;
}

#endif // __WIN32__


void EntangleDialog::UpdateProgress()
{
    wxASSERT(Total!=0);
    wxASSERT(NumBytes!=0);
    ShowProgress = (int)((double)NumBytes/Total*100);
    if(ShowProgress>100) ShowProgress=100;
    ProgressDialog1->Update(ShowProgress, show_str);
}

bool SmartRemove(wxString path, bool safe)
{
    if(safe)
    {
        fstream target;
        //Attempting to open the file
        target.open(path, ios_base::in | ios_base::binary);
        if(!target.is_open()) return false;
        //If opened, getting its size and closing it.
        target.seekg(0, ios::end); unsigned int fsize = target.tellg();
        target.close();
        //Opening the same file for writing
        target.open(path, ios_base::out | ios_base::binary);
        target.seekp(0, ios::beg);
        //Making some preparations
        unsigned int dleft, multiple;
        dleft = fsize%BUF_SIZE; multiple = fsize-dleft;
        char buffer[BUF_SIZE]; memset(buffer, (int)' ', BUF_SIZE);
        //Overwriting the data
        for(unsigned int i=0; i<multiple; i+=BUF_SIZE)
            target.write(buffer, BUF_SIZE);
        target.write(buffer, dleft);
        //Closing the file
        target.close();
    }

    if(remove(path.c_str())==0)
        return true;

    /** Read-Only files removing **/
    #ifdef __WIN32__
    WIN32_FIND_DATA f;
    HANDLE h = FindFirstFile(path.wc_str() , &f);
    if(h == INVALID_HANDLE_VALUE) return false;
    if(f.dwFileAttributes & FILE_ATTRIBUTE_READONLY)
    {
        SetFileAttributes(path.wc_str(), FILE_ATTRIBUTE_NORMAL);
        if(remove(path.c_str())==0) return true;
    }
    FindClose(h);
    #endif // _WIN32

    return false;
}

bool SmartRename(wxString before, wxString after)
{
    //Trying usual renaming
    if(rename(before.c_str(), after.c_str())==0)
        return true;
    //If it fails, deleting the file with such a name...
    if(SmartRemove(after))
    {
        //...and retrying.
        if(rename(before.c_str(), after.c_str())==0)
            return true;
    }
    //If nothing helps, throwing an error.
    AddError(before, "BAD_RENAME");
    return false;
}

void AddError(wxString fname, wxString code)
{
    //Producing a human-readable output
    //and pushing the result to the main list.
    mistakes.push_back(fname+" ("+code+")");
}

void GoodFinish(fstream & In, fstream & Out, wxString & temp_path, wxString & first)
{
    //Checking whether any files are open.
    //If so, closing them.
    if(In.is_open()) In.close();
    if(Out.is_open()) Out.close();
    //Renaming the temp file into its original name.
    SmartRename(temp_path, first);
    wxTheApp->Yield();
}
