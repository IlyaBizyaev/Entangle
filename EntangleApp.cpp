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
    if ( wxsOK )
    {
    	if(m_lang == wxLANGUAGE_UNKNOWN)
            m_lang = (wxLanguage)wxLocale::GetSystemLanguage();
        if(m_lang==wxLANGUAGE_UKRAINIAN||m_lang==wxLANGUAGE_RUSSIAN_UKRAINE||m_lang==wxLANGUAGE_KAZAKH)
            m_lang=wxLANGUAGE_RUSSIAN;
        if (!m_locale.Init(m_lang, wxLOCALE_DONT_LOAD_DEFAULT))
            wxMessageBox("This language is not supported!");
        wxLocale::AddCatalogLookupPathPrefix(".");
        if (!m_locale.AddCatalog("Entangle"))
            wxMessageBox("Couldn't find translation for "+m_locale.GetLanguageName(m_lang));
        #ifdef __LINUX__
            m_locale.AddCatalog("fileutils");
        #endif
    	EntangleDialog Dlg(0);
        #ifdef __WIN32__
            Dlg.SetIcon(wxICON(aaaaa));
        #endif
    	SetTopWindow(&Dlg);
    	Dlg.ShowModal();
    	wxsOK = false;
    }
    //*)
    return wxsOK;

}
