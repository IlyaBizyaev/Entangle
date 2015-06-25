/***************************************************************
 * Name:      EntangleDialog.h
 * Purpose:   Defines the user interface
 * Author:    Ilya Bizyaev (bizyaev.game@yandex.ru)
 * Created:   2015-01-01
 * Copyright: Ilya Bizyaev (utor.ucoz.ru)
 * License:   GNU GPL v3
 **************************************************************/

#ifndef ENTANGLEDIALOG_H
#define ENTANGLEDIALOG_H

//Enumerates two possible modes
enum MODE {Encrypt, Decrypt};
//Your cap.

typedef unsigned long long ullong;

//(*Headers(EntangleDialog)
#include <wx/bmpbuttn.h>
#include <wx/progdlg.h>
#include <wx/dialog.h>
#include <wx/sizer.h>
#include <wx/button.h>
#include <wx/dirctrl.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
//*)


class EntangleDialog: public wxDialog
{
    public:
        //Constructor and destructor (wxSmith)
        EntangleDialog(wxWindow* parent,wxWindowID id = -1);
        virtual ~EntangleDialog();
        //Adds files that were dropped onto the dialog
        void AddDropped(wxArrayString filenames);
        //Updates the displayed progress
        void UpdateProgress(ullong & current, ullong & total, wxString show_str = wxEmptyString);

    private:

        //Joins and traverses task arrays
        void Preprocess();
        //Updates information about tasks
        void UpdateTasks();
        //Changes text of informational lines
        void SetText(int line, wxString message);

        //(*Handlers(EntangleDialog)
        void OnAbout(wxCommandEvent& event);
        void OnButton1Click(wxCommandEvent& event);
        void OnLockClick(wxCommandEvent& event);
        void OnPasswordChange(wxCommandEvent& event);
        void OnFileReselect(wxTreeEvent& event);
        //*)

        //(*Identifiers(EntangleDialog)
        static const long ID_STATICTEXT1;
        static const long ID_GENERICDIRCTRL1;
        static const long ID_STATICTEXT2;
        static const long ID_STATICTEXT3;
        static const long ID_TEXTCTRL1;
        static const long ID_BITMAPBUTTON1;
        static const long ID_BUTTON2;
        static const long ID_BUTTON1;
        static const long ID_PROGRESSDIALOG1;
        //*)

        //(*Declarations(EntangleDialog)
        wxProgressDialog* ProgressDialog1;
        wxGenericDirCtrl* GenericDirCtrl1;
        wxButton* Button1;
        wxStaticText* StaticText1;
        wxButton* AboutButton;
        wxStaticText* StaticText3;
        wxTextCtrl* TextCtrl1;
        wxStaticText* StaticText2;
        wxBitmapButton* BitmapButton1;
        //*)

        // 3 arrays for storing tasks:
        // UI_files - chosen using DirControl
        // drop_files - dropped onto the dialog
        // tasks - resulting array of the Preprocess() function
        wxArrayString UI_files, drop_files, tasks;
        //Required mode of operation
        MODE mode;
        //Tracking user's actions
        bool TasksSelected, PasswordTypedIn;

        DECLARE_EVENT_TABLE()
};

#endif // ENTANGLEDIALOG_H
