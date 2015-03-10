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

long GetFolderSize(wxString path); //Works on Unix
bool IsANum(char);
void AddError(wxString, wxString);
bool SmartRemove(wxString path);
bool SmartRename(wxString before, wxString after);

int NumFiles=0, ShowProgress=0;
unsigned long long Total=0, NumBytes=0;
bool NowWorking=false; bool ShouldDecrypt = false;
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

class CRYPTOPP_DLL SecByteBlockSink : public Bufferless<Sink>
{
public:
    SecByteBlockSink(SecByteBlock& sbb) : m_sbb(sbb) { }

    size_t Put2(const byte *inString, size_t length, int /*messageEnd*/, bool /*blocking*/)
    {
        if(!inString || !length)
            return length;
        m_sbb += SecByteBlock (inString, length);
        return 0;
    }

private:
    SecByteBlock& m_sbb;
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
    //Assign password to key
    SecByteBlock key(0x00, 16);
    const byte *byteKey = (byte*) wxpassword.c_str().AsChar();
    key.Assign(byteKey, 16);
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
        Total+=GetFolderSize(choice[i]);
    //If there are Drag&Drop tasks
    for(size_t i=0; i<drop_files.GetCount(); ++i)
        Total+=GetFolderSize(drop_files[i]);

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

void EntangleDialog::Process(wxString first, const SecByteBlock & key)
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
	}
	CloseHandle(h);
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
            //Not this folder, not outer one and not a backup
            if(fname!="."&&fname!=".."&&fname.find('~')==wxString::npos)
                Process(first+"/"+fname, key);
        }
        closedir(dir);
    }///////////////////////////////////////////////
    #endif // __WIN32__
    else//////////////////FILE//////////////////////
    {
        fstream In, Out;
        //Renaming file
        size_t cut = first.find_last_of('/');
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
            In.close();
            rename(temp_path.c_str(),first.c_str());
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
            byte iv[16]; SecByteBlock retrieved(0x00, 0); Header DecryptedHeader;
            //Reading the IV
            In.read((char*)&iv, 16);
            try
            {
                //New AES Decryption object
                GCM<AES>::Decryption d;
                //Setting key and IV
                d.SetKeyWithIV(key.BytePtr(), key.SizeInBytes(), iv, sizeof(iv));
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

                //If the object does not throw, here's the only
                //opportunity to check the data's integrity
                //TODO: Check if it's needed
                bool b = df.GetLastResult();
                assert(b == true);

                //Get data from channel
                df.SetRetrievalChannel("");
                size_t n = (size_t)df.MaxRetrievable();
                retrieved.resize(n);

                if(n > 0) { df.Get((byte*)retrieved.BytePtr(), n); }
                memcpy(&DecryptedHeader, retrieved.BytePtr(), n);
            }
           	catch(CryptoPP::InvalidArgument& e)
            {
                AddError(first, "INV_ARGUMENT");
                In.close(); Out.close();
                SmartRename(temp_path, first);
                return;
            }
            catch(CryptoPP::AuthenticatedSymmetricCipher::BadState& e)
            {
                // Pushing PDATA before ADATA results in:
                AddError(first, "BAD_STATE");
                In.close(); Out.close();
                SmartRename(temp_path, first);
                return;
            }
            catch(CryptoPP::HashVerificationFilter::HashVerificationFailed& e)
            {
                //Caught HashVerificationFailed
                AddError(first, "BAD_PASSWORD");
                In.close(); Out.close();
                SmartRename(temp_path, first);
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
            byte iv[16]; rnd.GenerateBlock(iv, 16);
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
                e.SetKeyWithIV(key.BytePtr(), key.SizeInBytes(), iv, sizeof(iv));
                //Creating SecByteBlock for the header
                SecByteBlock blockky(0x00, 0);
                //Filter with a SecByteBlockSink
                AuthenticatedEncryptionFilter ef(e,
                new SecByteBlockSink(blockky), false, TAG_SIZE);
                //Encrypting MakeHeader
                ef.ChannelPut("", (const byte*)&MakeHeader, sizeof(MakeHeader));
                ef.ChannelMessageEnd("");
                //Writing SecByteBlock
                Out.write(reinterpret_cast<const char*>(blockky.BytePtr()), blockky.SizeInBytes());
            }
            catch(CryptoPP::BufferedTransformation::NoChannelSupport& e)
            {
                // The tag must go in to the default channel
                AddError(first, "NO_CH_SUPPORT");
                In.close(); Out.close();
                SmartRename(temp_path, first);
                return;
            }
            catch(CryptoPP::AuthenticatedSymmetricCipher::BadState& e)
            {
                // TODO: Get more info about this.
                AddError(first, "BAD_STATE");
                In.close(); Out.close();
                SmartRename(temp_path, first);
                return;
            }
            catch(CryptoPP::InvalidArgument& e)
            {
                AddError(first, "INV_ARGUMENT");
                In.close(); Out.close();
                SmartRename(temp_path, first);
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
        if(!SmartRemove(temp_path))
        {
            AddError(first, "BAD_TEMP_DELETE");
            return;
        }
        ++NumFiles;
    }
    return;
}


long GetFolderSize(wxString path)
{
    FILE* pipe = popen(wxString("du -b -s \"" + path + "\"").c_str(), "r");
    if(!pipe) return -1;
    char buffer[128]; wxString result = "";
    while(!feof(pipe))
    {
    	if(fgets(buffer, 128, pipe)!=NULL)
    		result += buffer;
    }
    pclose(pipe);
    wxString temp=""; long lresult=0;
    for(size_t i=0; i<result.length(); ++i)
    {
        if(IsANum(result.c_str()[i]))
            temp+=result.c_str()[i];
        else
            break;
    }
    temp.ToLong(&lresult);
    return lresult;
}

void EntangleDialog::UpdateProgress()
{
    wxASSERT(Total!=0);
    wxASSERT(NumBytes!=0);
    ShowProgress = (int)((double)NumBytes/Total*100);
    if(ShowProgress>100) ShowProgress=100;
    ProgressDialog1->Update(ShowProgress, show_str);
}

bool SmartRemove(wxString path)
{
    /* TODO: Implement safe deletion! */
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
    #endif // _WIN32

    return false;
}

bool SmartRename(wxString before, wxString after)
{
    if(rename(before.c_str(), after.c_str())==0)
        return true;
    if(SmartRemove(after))
    {
        if(rename(before.c_str(), after.c_str())==0)
            return true;
    }

    AddError(before, "BAD_RENAME");
    return false;
}

bool IsANum(char symbol)
{
    int code = (int)symbol;
    if(code>47&&code<58)
        return true;
    return false;
}

void AddError(wxString fname, wxString code)
{
    mistakes.push_back(fname+" ("+code+")");
}
