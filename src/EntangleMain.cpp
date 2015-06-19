/***************************************************************
 * Name:      EntangleMain.cpp
 * Purpose:   Code for Application Frame
 * Author:    Ilya Bizyaev (bizyaev.game@yandex.ru)
 * Created:   2015-01-01
 * Copyright: Ilya Bizyaev (utor.ucoz.ru)
 * License:   GNU GPL v3
 **************************************************************/

/** ------------ Include files ------------ **/
#include "EntangleMain.h"
#include "EntangleApp.h"
#include "EntangleExtras.h"

#include <memory>               //Memory copying
#include <wx/filename.h>        //File existence and permissions
#include <wx/dir.h>
#include <wx/aboutdlg.h>
#include <wx/msgdlg.h>

#include <cryptopp/cryptlib.h>  //TODO: check if needed
#include <cryptopp/aes.h>       //AES algorithm
#include <cryptopp/gcm.h>       //AES/GCM mode
#include <cryptopp/pwdbased.h>  //Key derivation from password
#include <cryptopp/sha.h>       //SHA-512 hash function
/** --------------------------------------- **/

#define BUF_SIZE 16384
#define TAG_SIZE 16
using namespace std;
using namespace CryptoPP;

/** --------- Functions --------- **/
//Cryptography
void DeriveKey(byte * key, wxString & password, byte * iv);
void AddTail(BinFile & target);
//File functions
unsigned long long GetFileSize(wxString & path);
bool SmartRemove(wxString & path);
bool Shred(wxString & path);
//Helper functions
bool CheckHeader(Header & header, wxString & filename);
wxString ToString(int number);
//For emergencies
void AddError(wxString, wxString);
void GoodFinish(BinFile&, BinFile&);

/** --------- Global variables --------- **/
unsigned long long Total=0, NumBytes=0;
bool TasksSelected = false, PasswordTypedIn = false, ShouldDecrypt = false, WentWrong = false;

unsigned long long ULL_MAX = -1;

//(*InternalHeaders(EntangleDialog)
#include <wx/string.h>
#include <wx/intl.h>
#include <wx/bitmap.h>
#include <wx/image.h>
//*)

//(*IdInit(EntangleDialog)
const long EntangleDialog::ID_STATICTEXT1 = wxNewId();
const long EntangleDialog::ID_GENERICDIRCTRL1 = wxNewId();
const long EntangleDialog::ID_STATICTEXT2 = wxNewId();
const long EntangleDialog::ID_STATICTEXT3 = wxNewId();
const long EntangleDialog::ID_TEXTCTRL1 = wxNewId();
const long EntangleDialog::ID_BITMAPBUTTON1 = wxNewId();
const long EntangleDialog::ID_BUTTON2 = wxNewId();
const long EntangleDialog::ID_BUTTON1 = wxNewId();
const long EntangleDialog::ID_PROGRESSDIALOG1 = wxNewId();
//*)

BEGIN_EVENT_TABLE(EntangleDialog,wxDialog)
    //(*EventTable(EntangleDialog)
    //*)
END_EVENT_TABLE()

/* Window constructor and destructor */
//wxDIRCTRL_MULTIPLE
EntangleDialog::EntangleDialog(wxWindow* parent,wxWindowID id)
{
    //(*Initialize(EntangleDialog)
    wxFlexGridSizer* FlexGridSizer1;
    wxFlexGridSizer* FlexGridSizer2;
    wxBoxSizer* BoxSizer1;

    Create(parent, wxID_ANY, _("Entangle"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE, _T("wxID_ANY"));
    SetFocus();
    FlexGridSizer1 = new wxFlexGridSizer(5, 1, 0, 0);
    StaticText1 = new wxStaticText(this, ID_STATICTEXT1, _("Choose files or folders:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
    FlexGridSizer1->Add(StaticText1, 1, wxTOP|wxLEFT|wxRIGHT|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    GenericDirCtrl1 = new wxGenericDirCtrl(this, ID_GENERICDIRCTRL1, wxEmptyString, wxDefaultPosition, wxSize(190,190), wxDIRCTRL_MULTIPLE, wxEmptyString, 0, _T("ID_GENERICDIRCTRL1"));
    FlexGridSizer1->Add(GenericDirCtrl1, 1, wxTOP|wxLEFT|wxRIGHT|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, wxDLG_UNIT(this,wxSize(5,0)).GetWidth());
    FlexGridSizer2 = new wxFlexGridSizer(2, 2, 0, 0);
    StaticText2 = new wxStaticText(this, ID_STATICTEXT2, _("Enter the password:"), wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE, _T("ID_STATICTEXT2"));
    FlexGridSizer2->Add(StaticText2, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText3 = new wxStaticText(this, ID_STATICTEXT3, _("Mode"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT3"));
    FlexGridSizer2->Add(StaticText3, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    TextCtrl1 = new wxTextCtrl(this, ID_TEXTCTRL1, wxEmptyString, wxDefaultPosition, wxSize(150,28), 0, wxDefaultValidator, _T("ID_TEXTCTRL1"));
    FlexGridSizer2->Add(TextCtrl1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    BitmapButton1 = new wxBitmapButton(this, ID_BITMAPBUTTON1, wxBitmap(wxImage(_T("./img/Encryption.png"))), wxDefaultPosition, wxSize(35,35), wxBU_AUTODRAW, wxDefaultValidator, _T("ID_BITMAPBUTTON1"));
    FlexGridSizer2->Add(BitmapButton1, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer1->Add(FlexGridSizer2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    BoxSizer1 = new wxBoxSizer(wxHORIZONTAL);
    AboutButton = new wxButton(this, ID_BUTTON2, _("About"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON2"));
    BoxSizer1->Add(AboutButton, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Button1 = new wxButton(this, ID_BUTTON1, _("Start"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON1"));
    Button1->SetFocus();
    BoxSizer1->Add(Button1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer1->Add(BoxSizer1, 1, wxBOTTOM|wxLEFT|wxRIGHT|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    SetSizer(FlexGridSizer1);
    FlexGridSizer1->Fit(this);
    FlexGridSizer1->SetSizeHints(this);
    Center();

    Connect(ID_TEXTCTRL1,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&EntangleDialog::OnPasswordChange);
    Connect(ID_BITMAPBUTTON1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&EntangleDialog::OnLockClick);
    Connect(ID_BUTTON2,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&EntangleDialog::OnAbout);
    Connect(ID_BUTTON1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&EntangleDialog::OnButton1Click);
    //*)

    //Enable Drag & Drop
    SetDropTarget(new DroppedFilesReciever(this));
    //Collapse the file choosing tree
    GenericDirCtrl1->CollapseTree();
    //Remove any default selections
    GenericDirCtrl1->UnselectAll();
    //Set event processor for GenericDirControl
    Bind(wxEVT_DIRCTRL_SELECTIONCHANGED, &EntangleDialog::OnFileReselect, this, ID_GENERICDIRCTRL1);
}

EntangleDialog::~EntangleDialog()
{
    //(*Destroy(EntangleDialog)
    //*)
}

/* Main function */
void EntangleDialog::Process(size_t task_index, wxString & password)
{
    //Preparing name for temp file
    wxString name = tasks[task_index];
    size_t cut = name.find_last_of(wxFILE_SEP_PATH)+1;
    wxString temp_path = name.substr(0, cut);
    RandomGenerator rnd; rnd.RandTempName(temp_path);
    //Required variables
    unsigned long long fsize, checker, dleft;
    static wxString show_str;
    /** Opening files **/
    //Opening original file
    BinFile In(name, ios_base::in);
    if(!In.is_open())
    {
        //Can't open the input file
        AddError(name, _("Cannot open the input file"));
        return;
    }
    //Opening the temp file
    BinFile Out(temp_path, ios_base::out|ios_base::trunc);
    if(!Out.is_open())
    {
        //Can't open the output file
        GoodFinish(In, Out);
        AddError(name, _("Cannot create an output file"));
        return;
    }
    /** <<<<<<< MOST IMPORTANT PART >>>>>>> **/

    if(ShouldDecrypt)
    {
        /** DECRYPTION **/
        show_str = _("Decrypting ")+name.substr(cut, name.Length()-cut);
        UpdateProgress(show_str);
        //Reading the IV
        byte iv[16];  In.read(iv, 16);
        //Reserving space for retrieved header
        Header DecryptedHeader;
        //Deriving the key
        byte key[16]; DeriveKey(key, password, iv);
        //If something goes wrong, this line gets error text.
        wxString error_text; bool GoodHeader = false;
        try
        {
            /** ----- Working with header ----- **/
            //New AES Decryption object
            GCM<AES>::Decryption d;
            //Setting key and IV
            d.SetKeyWithIV(key, 16, iv, sizeof(iv));
            //Reserving space for header and MAC and reading them
            byte head_and_tag[64]; In.read(head_and_tag, 64);
            //Creating new Decryption filter
            AuthenticatedDecryptionFilter df(d, NULL,
                AuthenticatedDecryptionFilter::MAC_AT_END |
                AuthenticatedDecryptionFilter::THROW_EXCEPTION, TAG_SIZE);
            //Putting the decrypted header to the filter
            df.ChannelPut("", (const byte*)&head_and_tag, 64);
            //If the object throws, it most likely occurs here
            df.ChannelMessageEnd("");

            //Get data from channel
            df.SetRetrievalChannel("");
            size_t n = (size_t)df.MaxRetrievable();
            if(n != sizeof(Header))
            {
                AddError(name, _("Incorrect header size"));
                GoodFinish(In, Out);
                return;
            }
            df.Get((byte*)&DecryptedHeader, n);

            /** Comparing cores **/
            if(CheckHeader(DecryptedHeader, name))
                GoodHeader = true;
            else
            {
                GoodFinish(In, Out);
                return;
            }

            /** ----- Decrypting the very file ----- **/
            GCM<AES>::Decryption gcmDecrypt;
            gcmDecrypt.SetKeyWithIV(DecryptedHeader.keys, 32, iv);

            AuthenticatedDecryptionFilter gcm_f(gcmDecrypt,
                new EntangleSink(Out),
                AuthenticatedDecryptionFilter::MAC_AT_END |
                AuthenticatedDecryptionFilter::THROW_EXCEPTION, TAG_SIZE);


            byte transfer[BUF_SIZE];
            fsize = DecryptedHeader.file_size;
            dleft = fsize % BUF_SIZE;
            checker = fsize - dleft;

            //THE VERY PROCESS
            for(unsigned long i=0; i<checker; i+=BUF_SIZE)
            {
                In.read(transfer, BUF_SIZE);
                gcm_f.ChannelPut("", transfer, BUF_SIZE);
                NumBytes+=BUF_SIZE;
                UpdateProgress();
            }
            if(dleft!=0)
            {
                In.read(transfer, dleft);
                gcm_f.ChannelPut("", transfer, dleft);
                NumBytes+=dleft;
                UpdateProgress();
            }

            In.read(transfer, TAG_SIZE);
            gcm_f.ChannelPut("", transfer, TAG_SIZE);

            gcm_f.MessageEnd();

        }
        catch(CryptoPP::InvalidArgument& e)
        {
            error_text = wxString("INV_ARGUMENT: ") + e.what();
        }
        catch(CryptoPP::AuthenticatedSymmetricCipher::BadState& e)
        {
            error_text = wxString("BAD_STATE:") + e.what();
        }
        catch(CryptoPP::HashVerificationFilter::HashVerificationFailed& e)
        {
            //Caught HashVerificationFailed
            if(GoodHeader)
                error_text = _("Invalid password or mode");
            else
                error_text = _("The file is corrupted");
        }
        catch(...)
        {
            //Unknown exception
            error_text = _("Unknown exception");
        }

        if(!error_text.empty())
        {
            AddError(name, error_text);
            GoodFinish(In, Out);
            return;
        }

    }
    else
    {
        /** ENCRYPTION **/
        show_str = _("Encrypting ")+name.substr(cut, name.Length()-cut);
        UpdateProgress(show_str);
        //Creating, generating and writing the IV
        RandomGenerator rnd; byte iv[16];
        rnd.GenerateBlock(iv, sizeof(iv)); Out.write(iv, 16);
        //Deriving the key
        byte key[16]; DeriveKey(key, password, iv); //TODO: Make key a return value
        //Getting file size
        fsize = file_sizes[task_index];
        //Creating the Entangle header
        Header MakeHeader(fsize);

        /** Encrypting and writing the header **/
        wxString error_text;
        try
        {
            //New AES Encryption object
            GCM<AES>::Encryption e;
            //Setting user key and random IV
            e.SetKeyWithIV(key, 16, iv, sizeof(iv));
            //Filter with an EntangleSink
            AuthenticatedEncryptionFilter ef(e,
            new EntangleSink(Out), false, TAG_SIZE);
            //Encrypting MakeHeader
            ef.ChannelPut("", (const byte*)&MakeHeader, sizeof(MakeHeader));
            ef.ChannelMessageEnd("");
        }
        catch(CryptoPP::BufferedTransformation::NoChannelSupport& e)
        {
            // The tag must go in to the default channel
            error_text = "NO_CH_SUPPORT";
        }
        catch(CryptoPP::AuthenticatedSymmetricCipher::BadState& e)
        {
            // TODO: Get more info about this.
            error_text = "BAD_STATE";
        }
        catch(CryptoPP::InvalidArgument& e)
        {
            error_text = "INVALID_ARGUMENT";
        }
        catch(...)
        {
            //Unknown exception
            error_text = _("Unknown exception");
        }

        if(!error_text.empty())
        {
            AddError(name, error_text);
            GoodFinish(In, Out);
            return;
        }

        /** ENCRYPTING THE VERY FILE **/
        GCM<AES>::Encryption gcmEncrypt;
        gcmEncrypt.SetKeyWithIV(MakeHeader.keys, 32, iv);

        AuthenticatedEncryptionFilter gcm_f(gcmEncrypt,
        new EntangleSink(Out), false, TAG_SIZE);

        byte transfer[BUF_SIZE];
        dleft = fsize%BUF_SIZE;
        checker = fsize-dleft;

        //THE VERY PROCESS
        for(unsigned long i=0; i<checker; i+=BUF_SIZE)
        {
            In.read(transfer, BUF_SIZE);
            gcm_f.ChannelPut("", transfer, BUF_SIZE);
            NumBytes+=BUF_SIZE;
            UpdateProgress();
        }
        if(dleft!=0)
        {
            In.read(transfer, dleft);
            gcm_f.ChannelPut("", transfer, dleft);
            NumBytes+=dleft;
            UpdateProgress();
        }

        gcm_f.MessageEnd();

        AddTail(Out);
    }
    /** <<<<<<< FINISHED MAIN PART >>>>>>> **/
    In.close(); Out.close();

    if(!ShouldDecrypt) Shred(name);
    SmartRemove(name);
    if(!wxRenameFile(temp_path, name))
    {
        //If can`t rename
        AddError(name, _("Cannot rename the result"));
        return;
    }
    return;
}

/* Event processors */
void EntangleDialog::OnButton1Click(wxCommandEvent& WXUNUSED(event))
{
    /* PREPARATIONS */
    NumBytes=0; Total=0; //Clear counters
    //Ensure that tasks are selected and the password is typed in
    if(!TasksSelected)
    {
        SetText(1, _("No tasks selected!"));
        return;
    }
    if(!PasswordTypedIn)
    {
        SetText(2, _("Enter password first!"));
        return;
    }
    //Get the password
    wxString password = TextCtrl1->GetLineText(0);
    //Expand all the paths
    Preprocess();
    //Get file sizes
    int NumFiles = 0; GetSizes(NumFiles);
    //Update the UI
    SetText(2, _("Processing...")); wxTheApp->Yield();
    ProgressDialog1 = new wxProgressDialog(_("Progress"), _("Starting..."), 100, this);
    ProgressDialog1->CenterOnParent();
    ProgressDialog1->Show();
    ProgressDialog1->Update(0, _("Starting..."));

    /* PROCESSING THE TASKS */
    for(size_t task_index = 0; task_index<tasks.GetCount(); ++task_index)
    {
        if(tasks[task_index]!="SKIP")
        {
            Process(task_index, password);
            ++NumFiles;
        }
    }
    ProgressDialog1->Update(100, _("Done!"));

    //Check if there were any problems during processing
    if(WentWrong)
    {
        //If it is so, show them to the user
        SetText(2, _("Went wrong :("));
        wxLogError(_("Something went wrong:"));
        wxLog::FlushActive();
        WentWrong = false;
    }
    else
    {
        wxString text;
        if(ShouldDecrypt)
            text = wxString::Format(wxPLURAL("Decrypted %i file", "Decrypted %i files", NumFiles), NumFiles);
        else
            text = wxString::Format(wxPLURAL("Encrypted %i file", "Encrypted %i files", NumFiles), NumFiles);
        wxMessageBox(text, _("Done!")); SetText(2, _("Done!"));
    }
    //Cleaning up
    CleanUp();
}

void EntangleDialog::OnFileReselect(wxTreeEvent& WXUNUSED(event))
{
    this->UpdateTasks();
}

void EntangleDialog::OnLockClick(wxCommandEvent& WXUNUSED(event))
{
    /* Reverses current mode */
    if(ShouldDecrypt)
        BitmapButton1->SetBitmap(wxImage("./img/Encryption.png"));
    else
        BitmapButton1->SetBitmap(wxImage("./img/Decryption.png"));
    ShouldDecrypt = !ShouldDecrypt;
}

void EntangleDialog::OnPasswordChange(wxCommandEvent& WXUNUSED(event))
{
    wxString wxpassword = TextCtrl1->GetLineText(0);
    int length = wxpassword.length();
    PasswordTypedIn = true;
    if(!length)
    {
        SetText(2, _("Enter the password:"));
        PasswordTypedIn = false;
    }
    else if(length > 16)
         SetText(2, _("Good")+" (>16)");
    else if(length == 16)
        SetText(2, _("Good")+" (16/16)");
    else if(length > 10)
        SetText(2, _("Medium")+" ("+ToString(length)+"/16)");
    else
        SetText(2, _("Short")+" ("+ToString(length)+"/16)");
    wxTheApp->Yield();
}

void EntangleDialog::OnAbout(wxCommandEvent& WXUNUSED(event))
{
    wxAboutDialogInfo aboutInfo;
    aboutInfo.SetName("Entangle");
    aboutInfo.SetVersion("0.9");
    aboutInfo.SetDescription(_("Simple and user-friendly application\nfor AES-based data encryption"));
    aboutInfo.SetCopyright("(C) Ilya Bizyaev <bizyaev@lyceum62.ru>, 2015");
    aboutInfo.SetWebSite("http://entangle.ucoz.net");
    wxAboutBox(aboutInfo, this);
    wxTheApp->SafeYield(NULL, false);
}

/* Cryptography */
void AddTail(BinFile & target)
{
    /** Adding random 'tail' **/
    RandomGenerator rnd;
    int tail_size = rnd.RandomNumber(1, 50);
    byte * tail = new byte[tail_size];
    rnd.GenerateBlock(tail, tail_size);
    target.write(tail, tail_size);
    delete[] tail;
}

void DeriveKey(byte * key, wxString & password, byte * iv)
{
    /** Deriving a key from password **/
    //Convert password to UTF-8
    wxCharBuffer cbuff = password.mb_str(wxMBConvUTF8());
    //Copy UTF-8 data to a byte array
    byte * bpass = new byte[cbuff.length()];
    memcpy((void*)bpass, (void*)cbuff.data(), cbuff.length());
    //Derive the key
    PKCS5_PBKDF2_HMAC<SHA512> KeyDeriver;
    KeyDeriver.DeriveKey(key, 16, (byte)0, bpass, cbuff.length(), iv, 16, 1);
    delete[] bpass;
}

/* File functions */
//Tasks preprocessor
void EntangleDialog::Preprocess()
{
    /* COLLECTING FILE PATHS */
    wxArrayString all_tasks;
    all_tasks.insert(all_tasks.end(), UI_files.begin(), UI_files.end());
    all_tasks.insert(all_tasks.end(), drop_files.begin(), drop_files.end());
    //For each task
    for(size_t pos = 0; pos < all_tasks.size(); ++pos)
    {
        /* If the object does not exist */
        if(!wxFileName::Exists(all_tasks[pos]))
        {
            AddError(all_tasks[pos], _("Does not exist"));
            continue;
        }

		/* Checking object type */
		if(wxDirExists(all_tasks[pos]))// If that's folder, scan all subdirectories and files inside it.
			wxDir::GetAllFiles(all_tasks[pos], &tasks);
		else//If that's file, simply push it to the vector.
			tasks.push_back(all_tasks[pos]);
    }

}

void EntangleDialog::GetSizes(int & NumFiles)
{
     /* GETTING FILE SIZES */
    file_sizes = new unsigned long long[tasks.size()];
    for(size_t i=0; i<tasks.GetCount(); ++i)
    {
        static unsigned long long fsize = GetFileSize(tasks[i]);
        //Processing exceptions
        if(fsize==0) //Empty file
        {
            tasks[i]="SKIP";
            ++NumFiles;
            continue;
        }
        if(fsize==ULL_MAX) //If GetFileSize() went wrong
        {
            tasks[i]="SKIP";
            AddError(tasks[i], _("Cannot access"));
            continue;
        }
        file_sizes[i] = fsize;
        Total+=fsize;
    }
}

//Cleaning up after processing
void EntangleDialog::CleanUp()
{
    //Clean task arrays
    drop_files.Clear(); tasks.Clear();
    //Deallocate array with file sizes
    delete[] file_sizes;
    //If only dropped files were processed, no tasks are now selected.
    if(UI_files.empty()) TasksSelected = false;
    UpdateTasks();
}

unsigned long long GetFileSize(wxString & path)
{

    //If file does not exist
    if(!wxFileExists(path)) return -1;
    //Getting file size
    wxULongLong result = wxFileName::GetSize(path);
    //If everything is OK
    if(result!=wxInvalidSize) return result.GetValue();
    //Otherwise
    return -1;
}

bool SmartRemove(wxString & path)
{
    //If there is already no such file, terminate.
    if(!wxFileExists(path)) return true;
    //if read-only, make writable
    if(!wxFileName::IsFileWritable(path))
    {
        wxFileName fname(path); if(!fname.IsOk()) return false;
        //Setting read-write permissions
        int permissions = wxS_IRUSR | wxS_IWUSR;
        fname.SetPermissions(permissions);
    }
    //Removing
    if(wxRemoveFile(path)) return true;
    //If fails, returning false.
    return false;
}

bool Shred(wxString & path)
{
    //Getting file size
    unsigned long long fsize = GetFileSize(path);
    //Opening this file for writing
    BinFile target(path, ios_base::out);
    if(!target.is_open()) return false;
    //Making some preparations
    unsigned long long dleft, checker;
    dleft = fsize % BUF_SIZE; checker = fsize - dleft;
    byte buffer[BUF_SIZE];
    RandomGenerator rnd;
    /**--------------------------------------------**/
    for(int iteration = 0; iteration < 10; ++iteration)
    {
        //Generating random data to write
        rnd.GenerateBlock((byte*)buffer, BUF_SIZE);
        //Moving to the beginning
        if(!target.seek_start()) return false;
        //Overwriting the data
        for(unsigned int i=0; i<checker; i+=BUF_SIZE)
            target.write(buffer, BUF_SIZE);
        //Ensure that the data was written, not cached
        target.write(buffer, dleft, true);
    }
    /**--------------------------------------------**/
    //Closing the file
    target.close();
    return true;
}

/* UI-based functions */
void EntangleDialog::UpdateTasks()
{
    //Get paths from the file tree
	GenericDirCtrl1->GetPaths(UI_files);

	wxString first_half, second_half, result;
	bool chosen = !UI_files.empty(), dropped = !drop_files.empty();
	size_t num_chosen = UI_files.size(), num_dropped = drop_files.size();

	if(chosen||dropped)
		TasksSelected = true;
	else
		TasksSelected = false;

	if(chosen)
        first_half = wxString::Format(wxPLURAL("Selected %lu", "Selected %lu", num_chosen), num_chosen);

	if(dropped)
        second_half = wxString::Format(wxPLURAL("Received %lu", "Received %lu", num_dropped), num_dropped);

	if(chosen && !dropped)
		result = first_half + wxPLURAL(" object", " objects", num_chosen);
	else if(dropped && !chosen)
		result = second_half + wxPLURAL(" object", " objects", num_dropped);
	else if(chosen && dropped)
		result = first_half + wxT(", ") + second_half.MakeLower();
	else if(!chosen && !dropped)
		result = _("Choose files or folders:");

	SetText(1, result);
}

void EntangleDialog::AddDropped(wxArrayString filenames)
{
    drop_files.insert(drop_files.end(), filenames.begin(), filenames.end());
}

void EntangleDialog::UpdateProgress(wxString show_str)
{
    //Re-calculates current progress
    //and updates the ProgressDialog()
    int ShowProgress = (double)NumBytes/Total*100;
    if(ShowProgress>100) ShowProgress=100;
    ProgressDialog1->Update(ShowProgress, show_str);
    wxTheApp->Yield();
}

void EntangleDialog::SetText(int line, wxString message)
{
    if(line==1)
        StaticText1->SetLabelText(message);
    else if(line==2)
        StaticText2->SetLabelText(message);
}

/* Helper functions */
bool CheckHeader(Header & header, wxString & filename)
{
    if(header.core_version!=ENTANGLE_CORE)
    {
        int that_core = header.core_version;
        //The core is newer and the user needs an upgrade (^_^)
        if(that_core > ENTANGLE_CORE)
            AddError(filename, _("Requires newer program version"));
        else //The core is older, an outdated version is needed.
            AddError(filename, _("Was encrypted by older version"));
        return false;
    }
    else
        return true;
}

wxString ToString(int number)
{
    return wxString::FromDouble(number);
}

/* For emergencies */
void AddError(wxString filename, wxString message)
{
    //Producing a human-readable output
    //and pushing the result to the main list.
    wxLogError(filename+" ("+message+")");
    WentWrong = true;
}

void GoodFinish(BinFile & In, BinFile & Out)
{
    //Checking whether any files are open.
    //If so, closing them.
    if(In.is_open()) In.close();
    if(Out.is_open()) Out.close();
    //Removing the temp file
    wxString temp_name = Out.GetName();
    SmartRemove(temp_name);
}
