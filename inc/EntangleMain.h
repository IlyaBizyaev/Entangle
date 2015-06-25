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

#include "EntangleDialog.h"
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
        //Constructor is private, thus this class has needs a method to be initialized
        void Initialize(EntangleDialog * g_dialog, wxArrayString & g_tasks, wxString & password, MODE g_mode);
        //Called by the UI; runs main processing algorithm for each task
        int Process();
        //Check if header's versions matches program's one
        bool CheckHeader(Header & header, wxString & filename);
    private:
        //Making Entangle a singleton
        Entangle() : Initialized(false) {  };
        Entangle(const Entangle&);
        const Entangle& operator=(const Entangle&);

        //Get file size for each file
        void GetSizes(int & NumFiles);
        //Process one file. MAIN ALGORITHM!
        void ProcessFile(size_t task_index);
        //Cleaning up after processing
        void CleanUp();

        MODE mode;                  //Mode of operation
        wxArrayString tasks;        //Task array
        wxString password;          //User's password
        EntangleDialog * dialog;    //Pointer to the GUI

        ErrorTracker e_track;

        bool Initialized;
        unsigned long long * file_sizes;
        unsigned long long NumBytes, Total;
};

#endif // ENTANGLEMAIN_H
