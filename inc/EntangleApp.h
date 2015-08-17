/***************************************************************
 * Name:      EntangleApp.h
 * Purpose:   Defines Application Class
 * Author:    Ilya Bizyaev (bizyaev.game@yandex.ru)
 * Created:   2015-01-01
 * Copyright: Ilya Bizyaev (utor.ucoz.ru)
 * License:   GNU GPL v3
 **************************************************************/

#ifndef ENTANGLEAPP_H
#define ENTANGLEAPP_H

#include <wx/app.h>
#include <wx/cmdline.h>

class EntangleApp : public wxApp
{
    public:
        EntangleApp() : m_lang(wxLANGUAGE_UNKNOWN), console_mode(false) {  }
        virtual bool OnInit();
        virtual void OnInitCmdLine(wxCmdLineParser& parser);
        virtual bool OnCmdLineParsed(wxCmdLineParser& parser);
        virtual int OnRun();
    private:
        wxLanguage m_lang;
        wxLocale m_locale;
        bool console_mode;
};

static const wxCmdLineEntryDesc g_cmdLineDesc [] =
{
     { wxCMD_LINE_SWITCH, "h", "help", "displays help on the command line parameters",
          wxCMD_LINE_VAL_NONE, wxCMD_LINE_OPTION_HELP },

     { wxCMD_LINE_OPTION , "p", "password", "used to specify a password", wxCMD_LINE_VAL_STRING },

     { wxCMD_LINE_OPTION, "m", "mode", "mode of operation", wxCMD_LINE_VAL_STRING },

     { wxCMD_LINE_PARAM, "", "", "", wxCMD_LINE_VAL_STRING,
     wxCMD_LINE_PARAM_OPTIONAL | wxCMD_LINE_PARAM_MULTIPLE },

     { wxCMD_LINE_NONE }
};

DECLARE_APP(EntangleApp)

#endif // ENTANGLEAPP_H
