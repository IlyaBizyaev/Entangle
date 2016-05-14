/***************************************************************
 * Name:      EntangleFrame.h
 * Purpose:   Defines the user interface
 * Author:    Ilya Bizyaev (bizyaev@lyceum62.ru)
 * Created:   2015-01-01
 * Copyright: Ilya Bizyaev
 * License:   GNU GPL v3
 **************************************************************/

#ifndef ENTANGLEDIALOG_H
#define ENTANGLEDIALOG_H

#include "extras/Encryption.h"

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

enum {TASKS, HINT};

class EntangleFrame: public wxFrame
{
    public:
        //Constructor and destructor
        EntangleFrame(wxWindow* parent, UserData data, wxWindowID id = -1);
        //Adds files that were dropped onto the dialog
        void AddDropped(const wxArrayString & filenames);

    private:
        //Called to locate and load icons
        void LoadImages();
        //Updates information about tasks
        void UpdateTasks();
        //Changes text of informational lines
        void SetText(int line, const wxString & message);

        //(*Handlers(EntangleFrame)
        void OnAboutButtonClick(wxCommandEvent& event);
        void OnStartButtonClick(wxCommandEvent& event);
        void OnLockClick(wxCommandEvent& event);
        void OnPasswordChange(wxCommandEvent& event);
        void OnFileReselect(wxTreeEvent& event);
        //*)

        //(*Identifiers(EntangleFrame)
        static const long ID_TASKTEXT;
        static const long ID_PASSWORDHINT;
        static const long ID_MODETEXT;
        static const long ID_PASSWORD;
        static const long ID_STARTBUTTON;
        static const long ID_ABOUTBUTTON;
        static const long ID_MODESWITCHER;
        static const long ID_FILESELECTOR;
        static const long ID_PROGRESSDIALOG1;
        //*)

        //(*Declarations(EntangleFrame)
        wxFlexGridSizer* FlexGridSizer1;
        wxFlexGridSizer* FlexGridSizer2;
        wxBoxSizer* BoxSizer1;
        wxPanel* panel;
        wxStaticText* TaskText;
        wxStaticText* PasswordHint;
        wxStaticText* ModeText;
        wxTextCtrl* TextCtrl1;
        wxButton* AboutButton;
        wxButton* StartButton;
        wxBitmapButton* ModeSwitcher;
        wxGenericDirCtrl* FileSelector;
        wxProgressDialog* ProgressDialog1;
        //*)

        // 2 arrays for storing tasks:
        // UI_files - chosen using DirControl
        // received_files - selected in a different way
        wxArrayString UI_files, received_files;
        //Icons that indicate selected mode
        wxImage ui_img[2];
        //Selected mode of operation
        MODE mode;

        DECLARE_EVENT_TABLE()
};

#endif // ENTANGLEDIALOG_H
