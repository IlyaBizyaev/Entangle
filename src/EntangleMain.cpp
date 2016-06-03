/***************************************************************
 * Name:      EntangleMain.cpp
 * Purpose:   Cryptography
 * Author:    Ilya Bizyaev (bizyaev@lyceum62.ru)
 * Created:   2015-06-23
 * Copyright: Ilya Bizyaev
 * License:   GNU GPL v3
 **************************************************************/

/** ------------ Include files ------------ **/
#include "EntangleMain.h"

#include <climits>              //Maximum variable values
#include <cassert>              //Assertions

#include <wx/log.h>             //Temporary disabling logging
#include <wx/wfstream.h>        //File streams
#include <wx/zipstrm.h>         //Working with ZIP

#include <cryptopp/aes.h>       //AES algorithm
#include <cryptopp/gcm.h>       //AES/GCM mode
#include <cryptopp/pwdbased.h>  //Key derivation from password
#include <cryptopp/sha.h>       //SHA-512 hash function
/** --------------------------------------- **/

/** -------------- Functions -------------- **/
void DeriveKey(ByteArray & key, ByteArray & iv, wxString & password);
void AddTail(BinFile & target);
/** --------------------------------------- **/

using namespace std;
using namespace CryptoPP;

Entangle::Entangle(UserData & udata, EntangleFrame * frame) : u(udata),
progress(Progress(frame)), a(Asker(frame != NULL)) { }

/** Compression and decompression **/
bool Entangle::Compress(bool remove)
{
    //There archive may be big => let the user choose the destination
    wxString archive_path = a.WhereToSave()+wxNow()+".zip";
    //On Windows, filenames cannot contain the ":" character...
    #ifdef _WIN32
    bool has_volume = wxFileName(archive_path).HasVolume();
    archive_path.Replace(":", ".");
    //... except for a volume name.
    if(has_volume)
        archive_path.Replace(".", ":", false);
    #endif // _WIN32
    //Creating the output. Its name is the current data (useful for backups)
    wxFFileOutputStream out(archive_path);
    if(!out.IsOk())
    {
        Issues::Add(_("Cannot create an output file"), archive_path);
        return false;
    }
    wxZipOutputStream zip(out);
    if(!zip.IsOk())
    {
        Issues::Add(_("Cannot create an output file"), archive_path);
        return false;
    }
    //Special entry to somehow recognize that it's not just an archive
    zip.PutNextEntry("encrypted_by_entangle");
    //Notifying about the start
    progress.Start();
    GetSizes();
    for(size_t i=0; i<u.tasks.GetCount(); ++i)
    {
        wxString fname = u.tasks[i];
        if(fname=="SKIP") continue;
        //Make the path absolute and remove ':' in 'X:\'
        wxFileName fn(fname);
        if(fn.IsRelative())
        {
            fn.MakeAbsolute();
            fname = fn.GetFullPath();
        }
        wxString entry = fname;
        #ifdef _WIN32
            entry.Replace(":", ".", false);
        #endif // _WIN32
        zip.PutNextEntry(entry);
        {
            wxFileInputStream in(fname);
            if(!in.IsOk())
            {
                Issues::Add(_("Cannot open an input file"), fname);
                return false;
            }
            progress.SetText(_("Compressing ")+fname);
            zip.Write(in);
        }
        if(remove) wxRemoveFile(fname);
        progress.Increase(file_sizes[i]);
        wxYield();
    }
    zip.Close();

    //Removing empty directories (if left)
    if(remove)
        for(size_t i=0; i<u.dirs.GetCount(); ++i)
            wxFileName::Rmdir(u.dirs[i], wxPATH_RMDIR_FULL);

    //Previous tasks do not exist any more
    u.tasks.Clear(); file_sizes.clear();
    //But now we need to encrypt the archive
    u.tasks.push_back(archive_path);
    progress.Finish(true);
    return true;
}

/* Checks for the special archive entry */
bool Entangle::IsDecompressionNeeded(wxString & fname)
{
    wxFFileInputStream arc(fname);
    if(!arc.IsOk())
    {
        Issues::Add(_("Cannot open an input file"), fname);
        return false;
    }
    wxZipInputStream zip(arc);
    if(!zip.IsOk())
    {
        Issues::Add(_("Cannot open an input file"), fname);
        return false;
    }

    wxLogNull logNo; //Remove unwanted "invalid zip" message
    unique_ptr<wxZipEntry> entry(zip.GetNextEntry());
    if(entry == NULL) return false;
    wxString first_entry = entry->GetName();

    if(first_entry=="encrypted_by_entangle")
        return true;
    return false;
}

bool Entangle::Decompress(wxString & arc_name)
{
    //Let the user choose whether to remove the original files.
    bool remove = a.Ask(_("Remove the archive?"));
    //This code block is needed to call wxFFileInputStream's destructor.
    {
        //Opening the archive for reading
        wxFFileInputStream arc(arc_name);
        if(!arc.IsOk())
        {
            Issues::Add(_("Cannot open an input file"), arc_name);
            return false;
        }
        wxZipInputStream zip(arc);
        if(!zip.IsOk())
        {
            Issues::Add(_("Cannot open an input file"), arc_name);
            return false;
        }

        unique_ptr<wxZipEntry> entry(zip.GetNextEntry());

        ByteArray buffer(BUF_SIZE);

        wxString location = "";

        //For each entry
        while(entry.reset(zip.GetNextEntry()), entry.get() != NULL)
        {
            wxString fname = entry->GetName();
            fname.Replace(".", ":", false);
            if(wxFileName(fname).HasVolume())
            {
                #ifndef _WIN32
                if(location=="")
                    location=a.WhereToSave();
                #endif // not _WIN32
            }
            else
            {
                fname.Replace(":", ".", false);
                #ifdef _WIN32
                if(location=="")
                    location=a.WhereToSave();
                #endif // _WIN32
            }

            fname = location + fname;
            //Creating directory where the file was located before compression
            wxFileName(fname).Mkdir(wxS_DIR_DEFAULT, wxPATH_MKDIR_FULL);
            //Creating the file
            wxFileOutputStream out(fname);
            if(!out.IsOk())
            {
                Issues::Add(_("Cannot create an output file"), fname);
                return false;
            }

            progress.SetText(_("Decompressing ")+fname);

            float compress_ratio = double(entry->GetCompressedSize())/entry->GetSize();

            BlockReader rdr(entry->GetSize());
            do
            {
                zip.Read(buffer, rdr.block_size());
                if(!zip.LastRead())
                {
                    Issues::Add(_("Failed to read an entry"), fname);
                    return false;
                }
                progress.Increase(compress_ratio*rdr.block_size());
                out.Write(buffer, rdr.block_size());
            } while(rdr.next());
            wxYield();
            out.Close();
        }
    }
    if(remove) wxRemoveFile(arc_name);
    return true;
}

/** Called from outside the class **/
int Entangle::Process()
{
    /** Compression (if >200 files were selected for encryption) **/
    if(u.mode==Encrypt&&u.tasks.GetCount()>200)
    {
        if(a.Ask(_("More than 200 files were selected. Compress?")))
        {
            //Let the user choose whether to remove the original files.
            bool remove = a.Ask(_("Remove the original files?"));
            //Suggest to proceed without compression if failed
            if(!Compress(remove))
            {
                Issues::Show();
                progress.Finish(false);
                if(!a.Ask(_("Compression failed. Continue without compression?")))
                    return 0;
            }
        }
    }

    /** Encryption / decryption **/
    //Notifying about the start
    progress.Start();
    //Get file size of each task
    int NumFiles = GetSizes();

    for(size_t i = 0; i < u.tasks.GetCount(); ++i)
        if(u.tasks[i]!="SKIP" && ProcessFile(i))
            ++NumFiles;
    progress.Finish(true);

    /** Decompression (for encrypted ZIPs) **/
    if(!to_decompress.IsEmpty())
    {
        //Get file size of each task
        u.tasks.Clear();
        u.tasks.insert(u.tasks.begin(), to_decompress.begin(), to_decompress.end());
        to_decompress.Clear();
        GetSizes();
        //Notifying about the start
        progress.Start();

        for(size_t i = 0; i < u.tasks.GetCount(); ++i)
            if(!Decompress(u.tasks[i]))
                Issues::Add(_("Failed to decompress"), u.tasks[i]);
        progress.Finish(true);
    }
    return NumFiles;
}


/** Main function **/
bool Entangle::ProcessFile(size_t task_index)
{
    wxString name = u.tasks[task_index], error_text;
    wxString temp_path = RNG::TempName(name);
    Issues::SetFile(name);

    ullong fsize=0; Header head; bool GoodHeader = false;

    /** Opening files **/
    //Opening original file
    BinFile In(name, ios_base::in);
    if(!In.is_open())
    {
        Issues::Add(_("Cannot open an input file")+": "+ In.why_failed());
        return false;
    }
    //Opening a temporary file
    BinFile Out(temp_path, ios_base::out|ios_base::trunc);
    if(!Out.is_open())
    {
        Issues::Add(_("Cannot create an output file")+": " + Out.why_failed());
        return false;
    }

    //For the constants, see extras/Data.h
    static ByteArray key(HEAD_KEY_SIZE), iv(IV_SIZE), buffer(BUF_SIZE);

    /** <<<<<<<<<<<< MAIN PART >>>>>>>>>>>> **/


    try
    {
        /** ENCRYPTION **/
        if(u.mode == Encrypt)
        {
            progress.SetText(_("Encrypting ") + name);
            //Generating and writing the IV
            RNG::GenerateBlock(iv);
            Out.write(iv);
            //Deriving the key
            DeriveKey(key, iv, u.password);
            //Getting file size
            fsize = file_sizes[task_index];
            head.file_size = fsize;

            /** Encrypting and writing the header **/
            //New AES Encryption object
            GCM<AES>::Encryption e;
            //Setting user key and random IV
            e.SetKeyWithIV(key, key.size(), iv, iv.size());
            //Filter with an EntangleSink
            AuthenticatedEncryptionFilter ef(e,
            new EntangleSink(Out), false, TAG_SIZE);
            //Encrypting the new header
            ef.ChannelPut("", (const byte*)&head, sizeof(head));
            ef.ChannelMessageEnd("");

            /** Encrypting the file **/
            GCM<AES>::Encryption gcmEncrypt;
            gcmEncrypt.SetKeyWithIV(head.key, KEY_SIZE, iv, iv.size());

            AuthenticatedEncryptionFilter gcm_f(gcmEncrypt,
            new EntangleSink(Out), false, TAG_SIZE);

            BlockReader rdr(fsize);

            do
            {
                In.read(buffer, rdr.block_size());
                gcm_f.ChannelPut("", buffer, rdr.block_size());
                if(EntangleSink::write_failed())
                {
                    Issues::Add(_("Does not exist"), Out.GetPath());
                    return false;
                }
                progress.Increase(rdr.block_size());
            } while(rdr.next());

            gcm_f.MessageEnd();

            //Ensuring that the encrypted file has proper size
            Out.flush();
            if(Out.size()!=(fsize+IV_SIZE+sizeof(Header)+(TAG_SIZE<<1)))
            {
                Issues::Add(_("Encrypted file has wrong size"));
                Out.remove();
                return false;
            }

            AddTail(Out);
        }
        else /** DECRYPTION **/
        {
            progress.SetText(_("Decrypting ") + name);
            In.read(iv);
            DeriveKey(key, iv, u.password);

            /** ----- Working with header ----- **/
            //New AES Decryption object
            GCM<AES>::Decryption d;
            //Setting key and IV
            d.SetKeyWithIV(key, key.size(), iv, iv.size());
            //Reserving space for header and MAC and reading them
            ByteArray headntag(sizeof(Header)+TAG_SIZE);
            In.read(headntag);
            //Creating new Decryption filter
            AuthenticatedDecryptionFilter df(d, NULL,
                AuthenticatedDecryptionFilter::MAC_AT_END |
                AuthenticatedDecryptionFilter::THROW_EXCEPTION, TAG_SIZE);
            //Putting the decrypted header to the filter
            df.ChannelPut("", headntag, headntag.size());
            //Exceptions most likely occur here
            df.ChannelMessageEnd("");

            //Get data from channel
            df.SetRetrievalChannel("");
            if((size_t)df.MaxRetrievable() != sizeof(Header))
            {
                Issues::Add(_("Incorrect header size"));
                Out.remove();
                return false;
            }
            df.Get((byte*)&head, sizeof(Header));

            /** Comparing cores **/
            if(CheckHeader(head))
                GoodHeader = true;
            else
            {
                Out.remove();
                return false;
            }
            fsize = head.file_size;

            /** ----- Decrypting the file ----- **/
            GCM<AES>::Decryption gcmDecrypt;
            gcmDecrypt.SetKeyWithIV(head.key, KEY_SIZE, iv, iv.size());

            AuthenticatedDecryptionFilter gcm_f(gcmDecrypt,
                new EntangleSink(Out),
                AuthenticatedDecryptionFilter::MAC_AT_END |
                AuthenticatedDecryptionFilter::THROW_EXCEPTION, TAG_SIZE);

            BlockReader rdr(fsize);

            do
            {
                In.read(buffer, rdr.block_size());
                gcm_f.ChannelPut("", buffer, rdr.block_size());
                if(EntangleSink::write_failed())
                {
                    Issues::Add(_("Does not exist"), Out.GetPath());
                    return false;
                }
                progress.Increase(rdr.block_size());
            } while(rdr.next());

            In.read(buffer, TAG_SIZE);
            gcm_f.ChannelPut("", buffer, TAG_SIZE);

            gcm_f.MessageEnd();

            //Ensuring that the decrypted file has the correct size
            Out.flush();
            if(Out.size()!=fsize)
            {
                Issues::Add(_("Decrypted file has wrong size"));
                Out.remove();
                return false;
            }

        }
    }
    catch(CryptoPP::BufferedTransformation::NoChannelSupport& e)
    {
        // The tag must go in to the default channel
        error_text = wxString("NO_CH_SUPPORT: ") + e.what();
    }
    catch(CryptoPP::AuthenticatedSymmetricCipher::BadState& e)
    {
        error_text = wxString("BAD_STATE: ") + e.what();
    }
    catch(CryptoPP::InvalidArgument& e)
    {
        error_text = wxString("INV_ARGUMENT: ") + e.what();
    }
    catch(CryptoPP::HashVerificationFilter::HashVerificationFailed& e)
    {
        error_text = GoodHeader ? _("The file is corrupted")
                                : _("Invalid password or mode");
    }
    catch(ios_base::failure & e)
    {
        Issues::Add(e.what());
        //On read error, shredding what is already
        //decrypted because it may contain
        //private information.
        if(u.mode==Decrypt) Out.shred();
        Out.remove();
        return false;
    }
    catch(...)
    {
        error_text = _("Unknown exception");
    }

    if(!error_text.empty())
    {
        Issues::Add(error_text);
        Out.remove();
        return false;
    }

    /** <<<<<<< FINISHED MAIN PART >>>>>>> **/

    //Shred the original file on encryption
    if(u.mode == Encrypt)
    {
        //Showing a spinning cursor for the lat file
        bool show_spinner = (task_index == u.tasks.size()-1);
        if(show_spinner) wxBeginBusyCursor();
        In.shred();
        if(show_spinner) wxEndBusyCursor();
    }
    //Remove the input file
    In.remove();
    //Renaming the temp file
    if(!Out.rename(name))
    {
        Issues::Add(_("Cannot rename the result")+": "+Out.GetPath());
        return false;
    }

    if(u.mode == Decrypt)
    {
        //For possible compressed encryption
        if(name.EndsWith(".zip")&&IsDecompressionNeeded(name))
            to_decompress.push_back(name);
    }

    return true;
}


/** Helpers **/

/* Retrieves file sizes */
/* Returns the number of empty files */
int Entangle::GetSizes()
{
    int NumFiles=0; ullong fsize, total=0;
    file_sizes.resize(u.tasks.size());

    for(size_t i=0; i<u.tasks.GetCount(); ++i)
    {
        //To ensure that this file has not been processed
        //by this function before
        if(u.tasks[i]=="SKIP") continue;
        //Actual size request
        fsize = GetFileSize(u.tasks[i]);
        //Possible exceptions
        if(!fsize) //Empty file
        {
            //Don't have to process
            u.tasks[i]="SKIP";
            ++NumFiles;
        }
        else if(fsize==ULLONG_MAX) //GetFileSize() went wrong
        {
            Issues::Add(_("Cannot access"), u.tasks[i]);
            u.tasks[i]="SKIP";
        }
        else
        {
            //Everything is OK
            file_sizes[i] = fsize;
            total+=fsize;
        }
    }
    progress.SetTotal(total);
    return NumFiles;
}

/* Simply adds a random piece of data ('tail') to the end of the file */
void AddTail(BinFile & target)
{
    ByteArray tail(RNG::RandomNumber(1, 50));
    RNG::GenerateBlock(tail);
    target.write(tail);
}

/* Derives key from the user's password */
void DeriveKey(ByteArray & key, ByteArray & iv, wxString & password)
{
    //Convert password to UTF-8
    wxCharBuffer cbuff = password.mb_str(wxMBConvUTF8());
    //Derive the key
    PKCS5_PBKDF2_HMAC<SHA512> KeyDeriver;
    KeyDeriver.DeriveKey(key, key.size(), (byte)0, (byte*)cbuff.data(), cbuff.length(), iv, iv.size(), 1);
}
