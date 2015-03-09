/***************************************************************
 * Name:      EntangleMain.h
 * Purpose:   Defines Application Frame
 * Author:    Ilya Bizyaev (bizyaev.game@yandex.ru)
 * Created:   2015-01-06
 * Copyright: Ilya Bizyaev (utor.ucoz.ru)
 * License:   GNU GPL v3
 **************************************************************/

#ifndef ENTANGLEMAIN_H
#define ENTANGLEMAIN_H

//(*Headers(EntangleDialog)
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/msgdlg.h>
#include <wx/dirctrl.h>
#include <wx/progdlg.h>
#include <wx/button.h>
#include <wx/dialog.h>
//*)

#include<secblock.h>

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
        void Process(wxString first, const CryptoPP::SecByteBlock & key);
        //*)

        //(*Identifiers(EntangleDialog)
        static const long ID_STATICTEXT1;
        static const long ID_GENERICDIRCTRL1;
        static const long ID_STATICTEXT2;
        static const long ID_TEXTCTRL1;
        static const long ID_BUTTON2;
        static const long ID_BUTTON1;
        static const long ID_PROGRESSDIALOG1;
        static const long ID_MESSAGEDIALOG1;
        //*)

        //(*Declarations(EntangleDialog)
        wxStaticText* StaticText2;
        wxButton* Button1;
        wxStaticText* StaticText1;
        wxGenericDirCtrl* GenericDirCtrl1;
        wxButton* AboutButton;
        wxTextCtrl* TextCtrl1;
        wxProgressDialog* ProgressDialog1;
        wxMessageDialog* MessageDialog1;
        //*)

        DECLARE_EVENT_TABLE()
};

#endif // ENTANGLEMAIN_H
