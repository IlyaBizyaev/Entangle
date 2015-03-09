/***************************************************************
 * Name:      EntangleApp.h
 * Purpose:   Defines Application Class
 * Author:    Ilya Bizyaev (bizyaev.game@yandex.ru)
 * Created:   2015-01-06
 * Copyright: Ilya Bizyaev (utor.ucoz.ru)
 * License:   GNU GPL v3
 **************************************************************/

#ifndef ENTANGLEAPP_H
#define ENTANGLEAPP_H

#include <wx/app.h>

class EntangleApp : public wxApp
{
    public:
        EntangleApp() {m_lang = wxLANGUAGE_UNKNOWN;}
        virtual bool OnInit();
    private:
        wxLanguage m_lang;
        wxLocale m_locale;
};

DECLARE_APP(EntangleApp);

#endif // ENTANGLEAPP_H
