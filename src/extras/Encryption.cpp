/***************************************************************
 * Name:      Encryption.cpp
 * Purpose:   Implements extras, used for encryption
 * Author:    Ilya Bizyaev (bizyaev@lyceum62.ru)
 * Created:   26.04.2016
 * Copyright: Ilya Bizyaev
 * License:   GNU GPL v3
 **************************************************************/

#include "extras/Encryption.h"
#include "extras/UI.h"

using namespace std;


/* Header's constructor */
Header::Header(ullong fsize) : file_size(fsize), core_version(ENTANGLE_CORE)
{
    //Generate random keys
    RNG::GenerateBlock(key, KEY_SIZE);
}

/* Helper functions */
bool CheckHeader(Header & header)
{
    int that_core = header.core_version;
    if(that_core==ENTANGLE_CORE)
        return true;
    if(that_core > ENTANGLE_CORE)
        Issues::Add(_("Requires newer program version"));
    else //The core is older, an outdated version is needed.
        Issues::Add(_("Was encrypted by older version"));
    return false;
}


/* EntangleSink's static variable and 2 methods */
bool EntangleSink::m_fail = false;
//Function which accepts data from AES/GCM and writes to the file
size_t EntangleSink::Put2(const byte *inString, size_t length, int, bool)
{
	static int check = 0;
	if(!inString || !length) return length;
	//Writing the data
	assert(output.is_open());
	if(check==3200)
    {
        check=0;
        if(!wxFileExists(output.GetPath()))
        {
            m_fail = true;
            return 1;
        }

    }
	if(!output.write(inString, length))
    {
        Issues::Add(output.why_failed());
        return 1;
    }
    ++check;
	return 0;
}

bool EntangleSink::write_failed()
{
    bool res = m_fail;
    m_fail = false;
    return res;
}


/* RandomGenerator's methods */
AutoSeededRandomPool RNG::rnd;

void RNG::GenerateBlock(byte * output, size_t size)
{
    rnd.GenerateBlock(output, size);
}

unsigned int RNG::RandomNumber(int min_val, int max_val)
{
    unsigned int result;
    GenerateBlock((byte*)&result, sizeof(unsigned int));
    result = result % (max_val-min_val+1) + min_val;
    return result;
}

wxString RNG::TempName(const wxString & orig_file)
{
    wxString temp_name = wxFileName(orig_file).GetPathWithSep();
    //Random filename length (1 - 20):
    int length = RandomNumber(5, 20);
    //Creating the name char by char (a-z, A-Z, 0-9):
    for(int i=0; i<length||wxFileExists(temp_name); ++i)
    {
        int range = RandomNumber(1, 3);
        if(range==1) //Number (0-9):
            temp_name += (char)RandomNumber(48, 57);
        else if(range==2) //Capital letter (A-Z):
            temp_name += (char)RandomNumber(65, 90);
        else if(range==3) //Small letter (a-z):
            temp_name += (char)RandomNumber(97, 122);
    }
    return temp_name;
}
