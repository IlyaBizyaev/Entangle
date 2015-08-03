/***************************************************************
 * Name:      EntangleFrame.h
 * Purpose:   Defines the user interface
 * Author:    Ilya Bizyaev (bizyaev.game@yandex.ru)
 * Created:   2015-01-01
 * Copyright: Ilya Bizyaev (utor.ucoz.ru)
 * License:   GNU GPL v3
 **************************************************************/

#ifndef ENTANGLEDIALOG_H
#define ENTANGLEDIALOG_H


//(*Headers(EntangleFrame)
#include <wx/bmpbuttn.h>
#include <wx/progdlg.h>
#include <wx/frame.h>
#include <wx/sizer.h>
#include <wx/panel.h>
#include <wx/button.h>
#include <wx/dirctrl.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
//*)

#include <wx/string.h>

//Enumerates two possible modes
enum MODE {Encrypt, Decrypt};
//Your cap.

inline wxString ToString(int number) { return wxString::FromDouble(number); }

typedef unsigned long long ullong;

class EntangleFrame: public wxFrame
{
    public:
        //Constructor and destructor (wxSmith)
        EntangleFrame(wxWindow* parent,wxWindowID id = -1);
        virtual ~EntangleFrame();
        //Adds files that were dropped onto the dialog
        void AddDropped(wxArrayString filenames);

    private:

        //Updates information about tasks
        void UpdateTasks();
        //Changes text of informational lines
        void SetText(int line, wxString message);

        //(*Handlers(EntangleFrame)
        void OnAbout(wxCommandEvent& event);
        void OnButton1Click(wxCommandEvent& event);
        void OnLockClick(wxCommandEvent& event);
        void OnPasswordChange(wxCommandEvent& event);
        void OnFileReselect(wxTreeEvent& event);
        //*)

        //(*Identifiers(EntangleFrame)
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

        //(*Declarations(EntangleFrame)
        wxPanel* panel;
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

        // 2 arrays for storing tasks:
        // UI_files - chosen using DirControl
        // drop_files - dropped onto the dialog
        wxArrayString UI_files, drop_files;
        //Required mode of operation
        MODE mode;
        //Tracking user's actions
        bool TasksSelected, PasswordTypedIn;

        DECLARE_EVENT_TABLE()
};

#endif // ENTANGLEDIALOG_H
