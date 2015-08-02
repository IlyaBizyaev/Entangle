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

IMPLEMENT_APP(EntangleApp)

bool EntangleApp::OnInit()
{
    //(*AppInitialize

    if(!wxApp::OnInit())
        return false;

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


void EntangleApp::OnInitCmdLine(wxCmdLineParser& parser)
{
    parser.SetDesc (g_cmdLineDesc);
    // must refuse '/' as parameter starter or cannot use "/path" style paths
    parser.SetSwitchChars (wxT("-"));
    parser.SetLogo(wxS("Entangle v.0.9.3"));
}

bool EntangleApp::OnCmdLineParsed(wxCmdLineParser& parser)
{
    wxString password, mode;
    bool got_password = parser.Found("p", &password);
    bool got_mode = parser.Found("m", &mode);

    if(got_password && got_mode) //Console mode
        console_mode = true;
    else if(!got_password&&!got_mode) //GUI mode
    {
        console_mode = false;
        return true;
    }
    else //Only one option is specified
    {
        Write("You should specify BOTH password and mode!\n");
        return false;
    }

    mode.MakeLower();

    MODE e_mode;
    if(mode!="encryption"&&mode!="decryption")
    {
        Write("Invalid mode specified.\n");
        return false;
    }
    else if(mode == "encryption")
        e_mode = Encrypt;
    else
        e_mode = Decrypt;

    if(!parser.GetParamCount())
    {
        Write("No tasks specified.\n");
        return false;
    }

    // Getting files to process
    wxArrayString tasks;
    for(size_t i = 0; i < parser.GetParamCount(); i++)
        tasks.Add(parser.GetParam(i));

    ProgressDisplayer pdisplay;
    ErrorTracker e_track;
    e_track.SetConsoleMode();

    //Getting a link to the Entangle singleton
    Entangle& eInst = Entangle::Instance();
    //Initializing it
    eInst.Initialize(tasks, password, e_mode, &pdisplay);

    int NumFiles = eInst.Process();
    pdisplay.Done();
    Write("Processed "+ToString(NumFiles)+"\n");
    if(e_track.HasIssues())
    {
        e_track.ShowIssues();
        return false;
    }

    return true;
}

int EntangleApp::OnRun()
{
    if(console_mode)
        return 0;
    else
        return wxApp::OnRun();
}
