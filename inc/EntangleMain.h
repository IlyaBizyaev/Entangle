/***************************************************************
 * Name:      EntangleMain.h
 * Purpose:   Defines the Entangle class
 * Author:    Ilya Bizyaev (bizyaev.game@yandex.ru)
 * Created:   2015-06-23
 * Copyright: Ilya Bizyaev (utor.ucoz.ru)
 * License:   GNU GPL v3
 **************************************************************/

#ifndef ENTANGLEMAIN_H
#define ENTANGLEMAIN_H

#include "EntangleFrame.h"
#include "EntangleExtras.h"

// Cryptography-responsible class. This is the main part of Entangle.
// This class is defined as singleton, as there should exist only one
// instance of it for the whole program.

class Entangle
{
    public:
        //A method which provides callers with a link to the only instance
        static Entangle& Instance()
        {
            static Entangle OnlyInstance;
            return OnlyInstance;
        }
        //Constructor is private, thus this class needs a method to be initialized
        void Initialize(wxArrayString & g_tasks, wxString & g_password, MODE g_mode, ProgressDisplayer * g_pdisplay);
        //Called by the UI; runs main processing algorithm for each task
        int Process();
        //Check if header's versions matches program's one
        bool CheckHeader(Header & header, wxString & filename);
    private:
        //Making Entangle a singleton
        Entangle() : Initialized(false), file_sizes(NULL) {  };
        Entangle(const Entangle&);
        const Entangle& operator=(const Entangle&);

        //Get file size for each file
        void GetSizes(int & NumFiles);
        //Process one file. MAIN ALGORITHM!
        bool ProcessFile(size_t task_index);
        //Cleaning up after processing
        void CleanUp();

        MODE mode;                      //Mode of operation
        wxArrayString tasks;            //Task array
        wxString password;              //User's password
        ProgressDisplayer * pdisplay;   //Needed to update current progress

        ErrorTracker e_track;

        bool Initialized;
        unsigned long long * file_sizes;
};

#endif // ENTANGLEMAIN_H
