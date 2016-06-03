/***************************************************************
 * Name:      EntangleApp.cpp
 * Purpose:   Code for Application Class
 * Author:    Ilya Bizyaev (bizyaev@lyceum62.ru)
 * Created:   2015-01-06
 * Copyright: Ilya Bizyaev
 * License:   GNU GPL v3
 **************************************************************/

#include "EntangleApp.h"
#include "EntangleMain.h"
#include <wx/msgdlg.h>

IMPLEMENT_APP(EntangleApp)

bool EntangleApp::OnInit()
{
    if(!wxApp::OnInit())
        return false;

    if(console_mode) return true;

    //Initializing image handlers
    wxImage::AddHandler(new wxPNGHandler);
    #ifdef _WIN32
    wxImage::AddHandler(new wxICOHandler);
    #endif // _WIN32

    //Creating and showing the dialog
    EntangleFrame* Frame = new EntangleFrame(0, data);
    Frame->Show();
    SetTopWindow(Frame);

    return true;
}

void EntangleApp::GetTranslations()
{
    //Request system language
    m_lang = (wxLanguage)wxLocale::GetSystemLanguage();
    //For Russian-speaking countries, set the language to Russian.
    if(m_lang==wxLANGUAGE_RUSSIAN_UKRAINE||m_lang==wxLANGUAGE_KAZAKH)
        m_lang=wxLANGUAGE_RUSSIAN;

    if(m_locale.Init(m_lang))
    {
        //Search for translation files
        wxLocale::AddCatalogLookupPathPrefix("./lang");
        #ifdef __linux__ 
        wxLocale::AddCatalogLookupPathPrefix("/usr/share/locale/");
        #endif //__linux__
        m_locale.AddCatalog("Entangle");
        //No warning if the translation was not found.
    }
}


void EntangleApp::OnInitCmdLine(wxCmdLineParser& parser)
{
    parser.SetDesc(g_cmdLineDesc);
    parser.SetSwitchChars (wxT("-"));
    parser.SetLogo(wxS("Entangle v.1.0"));
}

bool EntangleApp::OnCmdLineParsed(wxCmdLineParser& parser)
{
    //Print version and exit
    if(parser.Found("v"))
    {
        Write("1.0\n");
        return false;
    }

    GetTranslations();

    short data_provided=0;
    wxString password, mode;

    //Password
    if(parser.Found("p", &password))
    {
        ++data_provided;
        data.password = password;
    }

    //Encryption mode
    if(parser.Found("m", &mode))
    {
        ++data_provided;
        mode.MakeLower();

        //Aliases are available
        if(mode=="encryption"||mode=="encrypt"||mode=="e")
            data.mode = Encrypt;
        else if(mode=="decryption"||mode=="decrypt"||mode=="d")
            data.mode = Decrypt;
        else
        {
            Write(_("Invalid mode.\n"));
            return false;
        }
    }

    //Files to process
    if(parser.GetParamCount())
    {
        ++data_provided;
        for(size_t i=0; i < parser.GetParamCount(); ++i)
            data.tasks.Add(parser.GetParam(i));
    }

    //If ALL the arguments are provided, no GUI is needed.
    if(data_provided==3)
    {
        //Connecting main classes
        UserData m_data(data.tasks, data.password, data.mode);
        Issues::ConsoleMode();
        Entangle E(m_data);

        //Processing
        int NumFiles = E.Process();
        //Reporting the result
        Write(ResultString(NumFiles, data.mode)+"\n");
        if(Issues::Exist())
        {
            Issues::Show();
            return false;
        }
        return true;
    }

    console_mode = false;
    return true;
}

int EntangleApp::OnRun()
{
    return console_mode ? 0 : wxApp::OnRun();
}
