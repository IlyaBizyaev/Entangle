/***************************************************************
 * Name:      EntangleApp.cpp
 * Purpose:   Code for Application Class
 * Author:    Ilya Bizyaev (bizyaev.game@yandex.ru)
 * Created:   2015-01-06
 * Copyright: Ilya Bizyaev (utor.ucoz.ru)
 * License:   GNU GPL v3
 **************************************************************/

#include "EntangleApp.h"
#include <wx/msgdlg.h>

//(*AppHeaders
#include "EntangleMain.h"
//*)

IMPLEMENT_APP(EntangleApp);

bool EntangleApp::OnInit()
{
    //(*AppInitialize
    bool wxsOK = true;
    wxInitAllImageHandlers();
    if (wxsOK)
    {
    	//If language is not defined yet, request system default one.
    	if(m_lang == wxLANGUAGE_UNKNOWN)
            m_lang = (wxLanguage)wxLocale::GetSystemLanguage();
        //For Russian-speaking countries, set the laungage to Russian.
        if(m_lang==wxLANGUAGE_RUSSIAN_UKRAINE||m_lang==wxLANGUAGE_KAZAKH)
            m_lang=wxLANGUAGE_RUSSIAN;

        if (!m_locale.Init(m_lang, wxLOCALE_DONT_LOAD_DEFAULT))
            wxMessageBox("This language is not supported!");

        //Search for translation files
        wxLocale::AddCatalogLookupPathPrefix("./lang");
        if (!m_locale.AddCatalog("Entangle"))
            wxMessageBox("Couldn't find translation for "+m_locale.GetLanguageName(m_lang));

        //Creating a dialog
    	EntangleFrame* Frame = new EntangleFrame(0);
    	//On Windows, setting an icon.
        #ifdef __WIN32__
            Frame->SetIcon(wxICON(aaaaa));
        #endif
        //Showing the dialog
    	Frame->Show();
    	SetTopWindow(Frame);
    }
    //*)
    return wxsOK;
}
