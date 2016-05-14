/***************************************************************
 * Name:      EntangleMain.h
 * Purpose:   Defines the Entangle class
 * Author:    Ilya Bizyaev (bizyaev@lyceum62.ru)
 * Created:   2015-06-23
 * Copyright: Ilya Bizyaev
 * License:   GNU GPL v3
 **************************************************************/

#ifndef ENTANGLEMAIN_H
#define ENTANGLEMAIN_H

#include "EntangleFrame.h"
#include "extras/Encryption.h"
#include "extras/UI.h"

#include <vector>

//Cryptography-responsible class. This is the main part of Entangle.
class Entangle
{
    public:
        Entangle(UserData & udata, EntangleFrame * frame = NULL);
        //Called by the UI; runs main processing algorithm for each task
        int Process();
    private:
        //Get file size for each file
        int GetSizes();
        //Called if the user requests compression
        bool Compress(bool remove);
        bool IsDecompressionNeeded(wxString & fname);
        bool Decompress(wxString & arc_name);
        //Process one file. MAIN ALGORITHM!
        bool ProcessFile(size_t task_index);

        /* Required data */
        UserData & u;
        wxArrayString to_decompress;  //Archives that are not just archives :)
        Progress progress;            //Needed to update current progress
        Asker a;                      //Asks yes/no questions
        vector<ullong> file_sizes;
};

#endif // ENTANGLEMAIN_H
