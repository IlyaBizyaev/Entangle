/***************************************************************
 * Name:      EntangleMain.h
 * Purpose:   Defines Application Frame
 * Author:    Ilya Bizyaev (bizyaev.game@yandex.ru)
 * Created:   2015-01-01
 * Copyright: Ilya Bizyaev (utor.ucoz.ru)
 * License:   GNU GPL v3
 **************************************************************/

#ifndef ENTANGLEMAIN_H
#define ENTANGLEMAIN_H

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

#include <fstream>
using std::ios_base;
using std::fstream;


class EntangleDialog: public wxDialog
{
    public:
        EntangleDialog(wxWindow* parent,wxWindowID id = -1);
        virtual ~EntangleDialog();
        void Preprocess();
        void Process(size_t task_index, wxString & password);
        void CleanUp();
        void AddDropped(wxArrayString filenames);
        void UpdateProgress();
        void UpdateTasks();
        void SetText(int line, wxString message);

    private:

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

        wxArrayString UI_files, drop_files, tasks;
        unsigned long long * file_sizes;

        DECLARE_EVENT_TABLE()
};

#endif // ENTANGLEMAIN_H
