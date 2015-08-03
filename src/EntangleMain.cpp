/***************************************************************
 * Name:      EntangleMain.cpp
 * Purpose:   Cryptography
 * Author:    Ilya Bizyaev (bizyaev.game@yandex.ru)
 * Created:   2015-06-23
 * Copyright: Ilya Bizyaev (utor.ucoz.ru)
 * License:   GNU GPL v3
 **************************************************************/

/** ------------ Include files ------------ **/
#include "EntangleMain.h"
#include "EntangleExtras.h"

#include <climits>              //Maximum values
#include <cassert>              //Assertions
#include <wx/filename.h>        //File existence and permissions
#include <wx/dir.h>             //Traversing function

#include <cryptopp/aes.h>       //AES algorithm
#include <cryptopp/gcm.h>       //AES/GCM mode
#include <cryptopp/pwdbased.h>  //Key derivation from password
#include <cryptopp/sha.h>       //SHA-512 hash function
/** --------------------------------------- **/

/** -------------- Functions -------------- **/
//Cryptography
void DeriveKey(byte key[], size_t key_size, byte iv[], size_t iv_size, wxString & password);
void AddTail(BinFile & target);
//File functions
unsigned long long GetFileSize(wxString & path);
bool SmartRemove(wxString & path);
bool Shred(wxString & path);
//Helper functions
bool CheckHeader(Header & header, wxString & filename);
wxArrayString Traverse (wxArrayString & input);
inline void MultipleAndRemainder(ullong number, ullong dividor, ullong & multiple, ullong & remain);
//For emergencies
void EmergencyFinish(BinFile&, BinFile&);
/** --------------------------------------- **/

using namespace std;
using namespace CryptoPP;


void Entangle::Initialize
(wxArrayString & g_tasks, wxString & g_password, MODE g_mode, ProgressDisplayer * g_pdisplay)
{
    //Copying data from GUI
    tasks = Traverse(g_tasks);
    password = g_password;
    mode = g_mode;
    pdisplay = g_pdisplay;

    //Now initialized.
    Initialized = true;
}

int Entangle::Process()
{
    //Ensure that the class is initialized
    assert(Initialized);
    //Cleaning the counters
    int NumFiles = 0;
    //Get file size of each task
    GetSizes(NumFiles);
    /* PROCESSING THE TASKS */
    for(size_t task_index = 0; task_index < tasks.GetCount(); ++task_index)
    {
        if(tasks[task_index]!="SKIP")
        {
            if(ProcessFile(task_index))
                ++NumFiles;
        }
    }
    //Force the caller to re-initialize
    Initialized = false;

    return NumFiles;
}

void Entangle::GetSizes(int & NumFiles)
{
     /* GETTING FILE SIZES */
    file_sizes.resize(tasks.size());
    unsigned long long fsize, total = 0;
    for(size_t i=0; i<tasks.GetCount(); ++i)
    {
        fsize = GetFileSize(tasks[i]);
        //Processing exceptions
        if(fsize==0) //Empty file
        {
            tasks[i]="SKIP";
            ++NumFiles;
            continue;
        }
        if(fsize==ULLONG_MAX) //If GetFileSize() went wrong
        {
            tasks[i]="SKIP";
            ErrorTracker::AddError(tasks[i], _("Cannot access"));
            continue;
        }
        file_sizes[i] = fsize;
        total+=fsize;
    }
    pdisplay->SetTotal(total);
}

/* Main function */
bool Entangle::ProcessFile(size_t task_index)
{
    //Preparing name for temp file
    wxString name = tasks[task_index];
    wxFileName fname(name);

    RandomGenerator rnd;
    wxString temp_path = rnd.RandTempName(fname.GetPath(wxPATH_GET_VOLUME | wxPATH_GET_SEPARATOR));

    //Required variables
    unsigned long long fsize, multiple, remains;
    /** Opening files **/
    //Opening original file
    BinFile In(name, ios_base::in);
    if(!In.is_open())
    {
        //Can't open the input file
        ErrorTracker::AddError(name, _("Cannot open the input file"));
        return false;
    }
    //Opening the temp file
    BinFile Out(temp_path, ios_base::out|ios_base::trunc);
    if(!Out.is_open())
    {
        //Can't open the output file
        EmergencyFinish(In, Out);
        ErrorTracker::AddError(name, _("Cannot create an output file"));
        return false;
    }

    /** <<<<<<< MOST IMPORTANT PART >>>>>>> **/
    byte key[16], iv[16];

    if(mode == Encrypt)
    {
        /** ENCRYPTION **/
        pdisplay->SetText(_("Encrypting ") + fname.GetFullName());
        //Generating and writing the IV
        rnd.GenerateBlock(iv, sizeof(iv));
        Out.write(iv, sizeof(iv), true);
        //Deriving the key
        DeriveKey(key, sizeof(key), iv, sizeof(iv), password);
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
            e.SetKeyWithIV(key, sizeof(key), iv, sizeof(iv));
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
            error_text = wxString("NO_CH_SUPPORT: ") + e.what();
        }
        catch(CryptoPP::AuthenticatedSymmetricCipher::BadState& e)
        {
            error_text = wxString("BAD_STATE:") + e.what();
        }
        catch(CryptoPP::InvalidArgument& e)
        {
            error_text = wxString("INV_ARGUMENT: ") + e.what();
        }
        catch(...)
        {
            error_text = _("Unknown exception");
        }

        if(!error_text.empty())
        {
            ErrorTracker::AddError(name, error_text);
            EmergencyFinish(In, Out);
            return false;
        }

        /** ENCRYPTING THE FILE **/
        GCM<AES>::Encryption gcmEncrypt;
        gcmEncrypt.SetKeyWithIV(MakeHeader.keys, 32, iv, sizeof(iv));

        AuthenticatedEncryptionFilter gcm_f(gcmEncrypt,
        new EntangleSink(Out), false, TAG_SIZE);

        ByteArray buffer(BUF_SIZE);
        MultipleAndRemainder(fsize, BUF_SIZE, multiple, remains);

        for(unsigned long i=0; i<multiple; i+=BUF_SIZE)
        {
            In.read(buffer, BUF_SIZE);
            gcm_f.ChannelPut("", buffer, BUF_SIZE);
            pdisplay->IncreaseCurrent(BUF_SIZE);
        }
        if(remains!=0)
        {
            In.read(buffer, remains);
            gcm_f.ChannelPut("", buffer, remains);
            pdisplay->IncreaseCurrent(remains);
        }

        gcm_f.MessageEnd();
        AddTail(Out);
    }
    else
    {
        /** DECRYPTION **/
        pdisplay->SetText(_("Decrypting ") + fname.GetFullName());
        //Reading the IV
        In.read(iv, sizeof(iv));
        //Deriving the key
        DeriveKey(key, sizeof(key), iv, sizeof(iv), password);
        //Reserving space for retrieved header
        Header DecryptedHeader; bool GoodHeader = false;
        //If something goes wrong, this line gets error text.
        wxString error_text;
        try
        {
            /** ----- Working with header ----- **/
            //New AES Decryption object
            GCM<AES>::Decryption d;
            //Setting key and IV
            d.SetKeyWithIV(key, sizeof(key), iv, sizeof(iv));
            //Reserving space for header and MAC and reading them
            byte head_and_tag[64]; In.read(head_and_tag, 64);
            //Creating new Decryption filter
            AuthenticatedDecryptionFilter df(d, NULL,
                AuthenticatedDecryptionFilter::MAC_AT_END |
                AuthenticatedDecryptionFilter::THROW_EXCEPTION, TAG_SIZE);
            //Putting the decrypted header to the filter
            df.ChannelPut("", head_and_tag, 64);
            //If the object throws, it most likely occurs here
            df.ChannelMessageEnd("");

            //Get data from channel
            df.SetRetrievalChannel("");
            if((size_t)df.MaxRetrievable() != sizeof(Header))
            {
                ErrorTracker::AddError(name, _("Incorrect header size"));
                EmergencyFinish(In, Out);
                return false;
            }
            df.Get((byte*)&DecryptedHeader, sizeof(Header));

            /** Comparing cores **/
            if(CheckHeader(DecryptedHeader, name))
                GoodHeader = true;
            else
            {
                EmergencyFinish(In, Out);
                return false;
            }

            /** ----- Decrypting the very file ----- **/
            GCM<AES>::Decryption gcmDecrypt;
            gcmDecrypt.SetKeyWithIV(DecryptedHeader.keys, 32, iv, sizeof(iv));

            AuthenticatedDecryptionFilter gcm_f(gcmDecrypt,
                new EntangleSink(Out),
                AuthenticatedDecryptionFilter::MAC_AT_END |
                AuthenticatedDecryptionFilter::THROW_EXCEPTION, TAG_SIZE);


            ByteArray buffer(BUF_SIZE);
            fsize = DecryptedHeader.file_size;
            MultipleAndRemainder(fsize, BUF_SIZE, multiple, remains);

            //THE VERY PROCESS
            for(unsigned long i=0; i<multiple; i+=BUF_SIZE)
            {
                In.read(buffer, BUF_SIZE);
                gcm_f.ChannelPut("", buffer, BUF_SIZE);
                pdisplay->IncreaseCurrent(BUF_SIZE);
            }
            if(remains!=0)
            {
                In.read(buffer, remains);
                gcm_f.ChannelPut("", buffer, remains);
                pdisplay->IncreaseCurrent(remains);
            }

            In.read(buffer, TAG_SIZE);
            gcm_f.ChannelPut("", buffer, TAG_SIZE);

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
                error_text = _("The file is corrupted");
            else
                error_text = _("Invalid password or mode");
        }
        catch(...)
        {
            //Unknown exception
            error_text = _("Unknown exception");
        }

        if(!error_text.empty())
        {
            ErrorTracker::AddError(name, error_text);
            EmergencyFinish(In, Out);
            return false;
        }

    }
    /** <<<<<<< FINISHED MAIN PART >>>>>>> **/

    In.close(); Out.close();

    if(mode == Encrypt) Shred(name);
    SmartRemove(name);
    if(!wxRenameFile(temp_path, name))
    {
        //If can`t rename
        ErrorTracker::AddError(name, _("Cannot rename the result"));
        return false;
    }
    return true;
}

/* Cryptography */
void AddTail(BinFile & target)
{
    /** Adding random 'tail' **/
    RandomGenerator rnd;
    int tail_size = rnd.RandomNumber(1, 50);
    ByteArray tail(tail_size);
    rnd.GenerateBlock(tail, tail_size);
    target.write(tail, tail_size);
}

void DeriveKey(byte key[], size_t key_size, byte iv[], size_t iv_size, wxString & password)
{
    /** Deriving a key from password **/
    //Convert password to UTF-8
    wxCharBuffer cbuff = password.mb_str(wxMBConvUTF8());
    //Derive the key
    PKCS5_PBKDF2_HMAC<SHA512> KeyDeriver;
    KeyDeriver.DeriveKey(key, key_size, (byte)0, (byte*)cbuff.data(), cbuff.length(), iv, iv_size, 1);
}

/* File functions */
unsigned long long GetFileSize(wxString & path)
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

bool SmartRemove(wxString & path)
{
    //If there is already no such file, terminate.
    if(!wxFileExists(path)) return true;
    //if read-only, make writable
    if(!wxFileName::IsFileWritable(path))
    {
        wxFileName fname(path);
        if(!fname.IsOk()) return false;
        //Setting read-write permissions
        fname.SetPermissions(wxS_IRUSR | wxS_IWUSR);
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
    unsigned long long multiple, remains;
    MultipleAndRemainder(fsize, BUF_SIZE, multiple, remains);
    ByteArray buffer(BUF_SIZE);
    RandomGenerator rnd;
    /**--------------------------------------------**/
    for(int iteration = 0; iteration < 10; ++iteration)
    {
        //Generating random data to write
        rnd.GenerateBlock(buffer, BUF_SIZE);
        //Moving to the beginning
        if(!target.seek_start()) return false;
        //Overwriting the data
        for(unsigned int i=0; i<multiple; i+=BUF_SIZE)
            target.write(buffer, BUF_SIZE);
        //Ensure that the data was written, not cached
        target.write(buffer, remains, true);
    }
    /**--------------------------------------------**/
    //Closing the file
    target.close();
    return true;
}

/* Helper functions */
bool Entangle::CheckHeader(Header & header, wxString & filename)
{
    int that_core = header.core_version;
    if(that_core==ENTANGLE_CORE)
        return true;
    else
    {
        //The core is newer and the user needs an upgrade (^_^)
        if(that_core > ENTANGLE_CORE)
            ErrorTracker::AddError(filename, _("Requires newer program version"));
        else //The core is older, an outdated version is needed.
            ErrorTracker::AddError(filename, _("Was encrypted by older version"));
        return false;
    }
}

//A function that traverses the path array (expands all paths)
wxArrayString Traverse (wxArrayString & input)
{
    wxArrayString result;
    //For each task
    for(size_t pos = 0; pos < input.size(); ++pos)
    {
        /* If the object does not exist */
        if(!wxFileName::Exists(input[pos]))
        {
            ErrorTracker::AddError(input[pos], _("Does not exist"));
            continue;
        }

		/* Checking object type */
		if(wxDirExists(input[pos]))// If that's folder, scan all subdirectories and files inside it.
			wxDir::GetAllFiles(input[pos], &result);
		else//If that's file, simply push it to the vector.
			result.push_back(input[pos]);
    }
    return result;
}

inline void MultipleAndRemainder(ullong number, ullong dividor, ullong & multiple, ullong & remains)
{
    remains = number % dividor;
    multiple = number - remains;
}

/* For emergencies */
void EmergencyFinish(BinFile & In, BinFile & Out)
{
    //Checking whether any files are open.
    //If so, closing them.
    if(In.is_open()) In.close();
    if(Out.is_open()) Out.close();
    //Removing the temp file
    wxString temp_name = Out.GetName();
    SmartRemove(temp_name);
}
