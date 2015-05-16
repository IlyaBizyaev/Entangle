/***************************************************************
 * Name:      EntangleMain.cpp
 * Purpose:   Code for Application Frame
 * Author:    Ilya Bizyaev (bizyaev.game@yandex.ru)
 * Created:   2015-01-01
 * Copyright: Ilya Bizyaev (utor.ucoz.ru)
 * License:   GNU GPL v3
 **************************************************************/

#include "EntangleMain.h"
#include "EntangleApp.h"
#include "EntangleExtras.h"
#include "EntangleDepends.h"
#define BUF_SIZE 16384
#define TAG_SIZE 16
#define ENTANGLE_CORE 2
using namespace std;
using namespace CryptoPP;

/** Functions **/
//File functions
unsigned long long GetFileSize(wxString & path);
bool SmartRemove(wxString & path, bool shred = false);
void RandTempName(wxString & temp_name);
//Helper functions
void DeriveKey(byte * key, wxString & password, byte * iv);
unsigned int RandomNumber(int num_min, int num_max);
wxString ToString(int number);
//For emergencies
void AddError(wxString, wxString);
void GoodFinish(EFile&, EFile&, wxString&);

/** Global variables **/
int NumFiles=0, ShowProgress=0;
unsigned long long Total=0, NumBytes=0;
bool TasksSelected = false, PasswordTypedIn = false, ShouldDecrypt = false, WentWrong = false;

unsigned long long ULL_MAX = -1;

/** Global objects **/
//String which is displayed in ProgressDialog()
wxString show_str = _("Starting...");
//Random data generator
AutoSeededRandomPool rnd;

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
void EntangleDialog::Process(wxString & name, wxString & password)
{
    wxASSERT(Total!=0);
    /** Checking the object type **/
    if(wxDirExists(name))////////////////// FOLDER //////////////////////
    {
        // If that's folder, call Process() recursively
        // for all the objects inside it.
        wxString searcher = wxFindFirstFile(name+"/*");
        while (!searcher.empty()) //While there are files
        {
            Process(searcher, password);
            searcher = wxFindNextFile();
        }
    }
    else//////////////////FILE//////////////////////
    {
        //Preparing name for temp file
        size_t cut = name.find_last_of(wxFILE_SEP_PATH)+1;
        wxString temp_path = name.substr(0, cut);
        RandTempName(temp_path);
        //Required variables
        unsigned long fsize, checker, dleft;
        //If encryption, getting file size
        if(!ShouldDecrypt)
        {
            fsize = GetFileSize(name);
            if(fsize==ULL_MAX)
            {
                AddError(name, _("Cannot access"));
                return;
            }
        }
        /** Opening files **/
        EFile In, Out;
        //Opening original file
        In.open(name, ios_base::in|ios_base::binary);
        if(!In.is_open())
        {
            //Can't open the input file
            AddError(name, _("Cannot open the input file"));
            return;
        }
        //Opening the temp file
        Out.open(temp_path, ios_base::out|ios_base::trunc|ios_base::binary);
        if(!Out.is_open())
        {
            //Can't open the output file
            GoodFinish(In, Out, temp_path);
            AddError(name, _("Cannot create an output file"));
            return;
        }
        /** <<<<<<< MOST IMPORTANT PART >>>>>>> **/

        if(ShouldDecrypt)
        {
            /**DECRYPTION**/
            show_str = _("Decrypting ")+name.substr(cut, name.Length()-cut);
            //Reserving space for IV, decrypted data and retrieved header
            byte iv[16]; byte * Retrieved; Header DecryptedHeader;
            //Reading the IV
            In.read(iv, 16);
            byte key[16]; DeriveKey(key, password, iv);
            try
            {
                //New AES Decryption object
                GCM<AES>::Decryption d;
                //Setting key and IV
                d.SetKeyWithIV(key, 16, iv, sizeof(iv));
                //Reserving space for header and MAC and reading them
                byte blockky[64]; In.read(blockky, 64);
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
                AddError(name, "INV_ARGUMENT");
                GoodFinish(In, Out, temp_path);
                return;
            }
            catch(CryptoPP::AuthenticatedSymmetricCipher::BadState& e)
            {
                // Pushing PDATA before ADATA results in:
                AddError(name, "BAD_STATE");
                GoodFinish(In, Out, temp_path);
                return;
            }
            catch(CryptoPP::HashVerificationFilter::HashVerificationFailed& e)
            {
                //Caught HashVerificationFailed
                AddError(name, _("Invalid password or mode"));
                GoodFinish(In, Out, temp_path);
                return;
            }
            /** Comparing cores **/
            if(DecryptedHeader.core_version!=ENTANGLE_CORE)
            {
                int that_core = DecryptedHeader.core_version;
                //The core is newer and the user needs an upgrade (^_^)
                if(that_core > ENTANGLE_CORE)
                    AddError(name, _("Requires newer program version"));
                else //The core is older, an outdated version is needed.
                    AddError(name, _("Was encrypted by older version"));
                GoodFinish(In, Out, temp_path);
                return;
            }
            /** CFB Mode Processing **/
            CFB_Mode<AES>::Decryption cfbDecryption(DecryptedHeader.keys, 32, iv);
            byte transfer[BUF_SIZE];
            fsize = DecryptedHeader.file_size; dleft = fsize%BUF_SIZE;
            checker = fsize-dleft;
            for(unsigned long i=0; i<checker; i+=BUF_SIZE)
            {
                In.read(transfer, BUF_SIZE);
                cfbDecryption.ProcessData((byte*)&transfer, (byte*)&transfer, BUF_SIZE);
                Out.write(transfer, BUF_SIZE);
                NumBytes+=BUF_SIZE;
                UpdateProgress();
            }
            if(dleft!=0)
            {
                In.read(transfer, dleft);
                cfbDecryption.ProcessData((byte*)&transfer, (byte*)&transfer, dleft);
                Out.write(transfer, dleft);
                NumBytes+=dleft;
                UpdateProgress();
            }

        }
        else
        {
            /** ENCRYPTION **/
            show_str = _("Encrypting ")+name.substr(cut, name.Length()-cut);
            //Creating, generating and writing the IV
            byte iv[16]; rnd.GenerateBlock(iv, sizeof(iv));
            Out.write(iv, 16);

            byte key[16]; DeriveKey(key, password, iv);

            /** Preparing the Entangle Header **/
            //Creating and cleaning
            Header MakeHeader; memset(&MakeHeader, 0x00, sizeof(MakeHeader));
            //Writing the file size
            MakeHeader.file_size = fsize;
            //Writing the version of my lovely program \_(^_^)_/
            MakeHeader.core_version = ENTANGLE_CORE;
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
                new EntangleSink(&Received, GotSize), false, TAG_SIZE);
                //Encrypting MakeHeader
                ef.ChannelPut("", (const byte*)&MakeHeader, sizeof(MakeHeader));
                ef.ChannelMessageEnd("");
                //Writing encrypted data
                Out.write(Received, GotSize);
            }
            catch(CryptoPP::BufferedTransformation::NoChannelSupport& e)
            {
                // The tag must go in to the default channel
                AddError(name, "NO_CH_SUPPORT");
                GoodFinish(In, Out, temp_path);
                return;
            }
            catch(CryptoPP::AuthenticatedSymmetricCipher::BadState& e)
            {
                // TODO: Get more info about this.
                AddError(name, "BAD_STATE");
                GoodFinish(In, Out, temp_path);
                return;
            }
            catch(CryptoPP::InvalidArgument& e)
            {
                AddError(name, "INV_ARGUMENT");
                GoodFinish(In, Out, temp_path);
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
                In.read(transfer, BUF_SIZE);
                cfbEncryption.ProcessData((byte*)&transfer, (byte*)&transfer, BUF_SIZE);
                Out.write(transfer, BUF_SIZE);
                NumBytes+=BUF_SIZE;
                UpdateProgress();
            }
            if(dleft!=0)
            {
                In.read(transfer, dleft);
                cfbEncryption.ProcessData((byte*)&transfer, (byte*)&transfer, dleft);
                Out.write(transfer, dleft);
                NumBytes+=dleft;
                UpdateProgress();
            }

        }
        /** <<<<<<< FINISHED MAIN PART >>>>>>> **/
        In.close(); Out.close();
        if(!ShouldDecrypt)
            SmartRemove(name, true);
        else
            SmartRemove(name);
        if(!wxRenameFile(temp_path, name))
        {
            //If can`t rename
            AddError(name, _("Cannot rename the result"));
            return;
        }
        ++NumFiles;
    }
    return;
}

/* Event processors */
void EntangleDialog::OnButton1Click(wxCommandEvent& WXUNUSED(event))
{
    NumFiles=0; NumBytes=0; Total=0; //Clear All
    //Check if tasks are selected and the password is typed in
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
    //Adding the Drag & Drop array
    tasks.insert(tasks.end(), drop_files.begin(), drop_files.end());
    drop_files.Clear();
    //If there are no tasks
    /* ESTIMATING REQUIRED WORK */
    unsigned long long fsize;
    for(size_t i=0; i<tasks.GetCount(); ++i)
    {
        if(!wxFileName::Exists(tasks[i]))
        {
            tasks[i]="SKIP";
            AddError(tasks[i], _("Does not exist"));
            continue;
        }
        fsize = GetFileSize(tasks[i]);
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
        Total+=fsize;
    }

    SetText(2, _("Processing...")); wxYield();
    ProgressDialog1 = new wxProgressDialog(_("Progress"), _("Starting..."), 100, this);
    ProgressDialog1->CenterOnParent();
    ProgressDialog1->Show();
    ProgressDialog1->Update(0, show_str);

    /* PROCESSING THE TASKS */
    for(size_t i=0; i<tasks.GetCount(); ++i)
    {
        if(tasks[i]!="SKIP")
            Process(tasks[i], password);
    }
    ProgressDialog1->Update(100, _("Done!"));

    if(WentWrong)
    {
        SetText(2, _("Went wrong :("));
        wxLogError(_("Something went wrong:"));
        wxLog::FlushActive();
        WentWrong = false;
    }
    else
        SetText(2, _("Complete (")+ToString(NumFiles)+_(" file(s))"));
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
    if(length == 0)
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
    wxYield();
}

void EntangleDialog::OnAbout(wxCommandEvent& WXUNUSED(event))
{
    wxAboutDialogInfo aboutInfo;
    aboutInfo.SetName("Entangle");
    aboutInfo.SetVersion("0.8");
    aboutInfo.SetDescription(_("Simple and user-friendly application\nfor AES-based data encryption"));
    aboutInfo.SetCopyright("(C) Ilya Bizyaev <bizyaev@lyceum62.ru>, 2015");
    aboutInfo.SetWebSite("http://entangle.ucoz.net");
    wxAboutBox(aboutInfo, this);
    wxTheApp->SafeYield(NULL, false);
}

/* File functions */
unsigned long long GetFileSize(wxString & path)
{
    //Check if it does not exist at all
    if(!wxFileName::Exists(path)) return -1;
    //Variable for the result
    wxULongLong result = 0;
    /** If that's folder **/
    if(wxDirExists(path))
    {
        // If that's folder, call GetFileSize() recursively
        // for all the objects inside it.
        wxString searcher = wxFindFirstFile(path+"/*");
        while (!searcher.empty()) //While there are files
        {
            result+=GetFileSize(searcher);
            searcher = wxFindNextFile();
        }
    }
    else /** If that's file **/
    {
        //If file does not exist
        if(!wxFileExists(path)) return -1;
        //Getting file size
        result = wxFileName::GetSize(path);
        //If the file is unavailable
        if(result==wxInvalidSize) return -1;
    }
    //If everything is OK
    return result.GetValue();
}

bool SmartRemove(wxString & path, bool shred)
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

    if(shred) /* If overwriting is required */
    {
        fstream target;
        //Getting file size
        unsigned long long fsize = GetFileSize(path);
        //Opening this file for writing
        target.open(path, ios_base::out | ios_base::binary);
        //Being paranoid
        if(!target.is_open()) return false;
        //Making some preparations
        unsigned long long dleft, multiple;
        dleft = fsize%BUF_SIZE; multiple = fsize-dleft;
        char buffer[BUF_SIZE];
        /**--------------------------------------------**/
        for(int iteration = 0; iteration < 10; ++iteration)
        {
            //Generating random data to write
            rnd.GenerateBlock((byte*)buffer, BUF_SIZE);
            //Moving to the beginning
            target.seekp(0, ios::beg);
            //Overwriting the data
            for(unsigned int i=0; i<multiple; i+=BUF_SIZE)
                target.write(buffer, BUF_SIZE);
            target.write(buffer, dleft);
            //Ensure that the data was written, not cached
            target.flush();
        }
        /**--------------------------------------------**/
        //Closing the file
        target.close();
    }

    //Removing
    if(wxRemoveFile(path)) return true;
    //If fails, returning false.
    return false;
}

void RandTempName(wxString & temp_name)
{
    wxString new_temp_name;
    do //While such file exists
    {
        //Random filename length (1 - 20):
        int length = RandomNumber(1, 20), range = 0;
        //Creating new char buffer for the filename
        char * filename = new char[length+1];
        //Filling the array (a-z, A-Z, 0-9):
        for(int i=0; i<length; ++i)
        {
            range = RandomNumber(1, 3);
            if(range==1) //Number (0-9):
                filename[i] = RandomNumber(48, 57);
            else if(range==2) //Capital letter (A-Z):
                filename[i] = RandomNumber(65, 90);
            else if(range==3) //Small letter (a-z):
                filename[i] = RandomNumber(97, 122);
        }
        //Writing zero character at the end
        filename[length] = '\0';
        //Building the full path
        new_temp_name = temp_name + wxString(filename);
    } while(wxFileExists(new_temp_name));
    //Returning filename and finishing
    temp_name = new_temp_name;
    return;
}

/* UI-based functions */
void EntangleDialog::UpdateTasks() //TODO: Simplify!
{
    GenericDirCtrl1->GetPaths(tasks);
    wxString chosen = ToString(tasks.size());
    wxString dropped = ToString(drop_files.size());
    TasksSelected = true;
    if(!tasks.empty()) //There are files chosen in usual way
    {
        if(!drop_files.empty()) //...and there are dropped ones
            SetText(1, _("Selected ")+chosen+_(", received ")+dropped);
        else //...and no dropped ones
            SetText(1, _("Selected ")+chosen+_(" object(s)"));
    }
    else //No files chosen in usual way
    {
        if(!drop_files.empty())  //...but there are dropped ones
            SetText(1, _("Received ")+dropped+_(" object(s)"));
        else  //...and no dropped ones
        {
            SetText(1, _("Choose files or folders:"));
            TasksSelected = false;
        }
    }
}

void EntangleDialog::AddDropped(wxArrayString filenames)
{
    drop_files.insert(drop_files.end(), filenames.begin(), filenames.end());
}

void EntangleDialog::UpdateProgress()
{
    //Re-calculates current progress
    //and updates the ProgressDialog()
    ShowProgress = (double)NumBytes/Total*100;
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
}

unsigned int RandomNumber(int num_min, int num_max)
{
    unsigned int result; byte * p_num = (byte*)&result;
    rnd.GenerateBlock(p_num, sizeof(int));
    result = result % (num_max-num_min+1) + num_min;
    return result;
}

wxString ToString(int number)
{
    return wxString::FromDouble(number);
}

/* For emergencies */
void AddError(wxString fname, wxString code)
{
    //Producing a human-readable output
    //and pushing the result to the main list.
    wxLogError(fname+" ("+code+")");
    WentWrong = true;
}

void GoodFinish(EFile & In, EFile & Out, wxString & temp_path)
{
    //Checking whether any files are open.
    //If so, closing them.
    if(In.is_open()) In.close();
    if(Out.is_open()) Out.close();
    //Removing the temp file
    SmartRemove(temp_path);
}
