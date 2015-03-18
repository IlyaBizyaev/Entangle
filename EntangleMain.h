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
#include <wx/msgdlg.h>
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
        EntangleDialog(wxWindow* parent,wxWindowID id = -1);
        virtual ~EntangleDialog();
        friend class DroppedFilesReciever;
        void UpdateProgress();

    private:

        //(*Handlers(EntangleDialog)
        void OnQuit(wxCommandEvent& event);
        void OnAbout(wxCommandEvent& event);
        void OnButton1Click(wxCommandEvent& event);
        void Process(wxString first, byte key[]);
        //*)

        //(*Identifiers(EntangleDialog)
        static const long ID_STATICTEXT1;
        static const long ID_GENERICDIRCTRL1;
        static const long ID_STATICTEXT2;
        static const long ID_TEXTCTRL1;
        static const long ID_BUTTON2;
        static const long ID_BUTTON1;
        static const long ID_MESSAGEDIALOG1;
        static const long ID_PROGRESSDIALOG1;
        //*)

        //(*Declarations(EntangleDialog)
        wxProgressDialog* ProgressDialog1;
        wxGenericDirCtrl* GenericDirCtrl1;
        wxButton* Button1;
        wxStaticText* StaticText1;
        wxButton* AboutButton;
        wxMessageDialog* MessageDialog1;
        wxTextCtrl* TextCtrl1;
        wxStaticText* StaticText2;
        //*)

        DECLARE_EVENT_TABLE()
};

#endif // ENTANGLEMAIN_H
