/***************************************************************
 * Name:      EntangleExtras.cpp
 * Purpose:   Implements Entangle's UI extras
 * Author:    Ilya Bizyaev (bizyaev@lyceum62.ru)
 * Created:   24.06.2016
 * Copyright: Ilya Bizyaev
 * License:   GNU GPL v3
 **************************************************************/


#include "extras/UI.h"

#include <wx/log.h>             //Error log in GUI mode

#ifdef _WIN32
#include <windows.h>		//Windows console functions
#endif // _WIN32


using namespace std;


/** TERMINAL **/

/* Cross-platform console output */
void Write(const wxString & message)
{
    #ifdef _WIN32
        static HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
        static bool attached = false;

        if(!attached)
        {
            if (hStdOut == NULL) return; //Console is not available
            //This is required in order to use the parent process' console..
            AttachConsole((DWORD)-1); //-1 = parent process
            attached = true;
        }

        wxScopedCharBuffer ascii = message.ToAscii();
        DWORD len = ascii.length();
        WriteFile(hStdOut, (char*)ascii.data(), len, &len, NULL);
    #else
        wcout << message.ToStdWstring();
    #endif // _WIN32
}

/** GUI **/

/* DroppedFilesReceiver's methods */
//Constructor
DroppedFilesReciever::DroppedFilesReciever(EntangleFrame * frame) : m_frame(frame) { }

//Called when something is dropped onto the window
bool DroppedFilesReciever::OnDropFiles(wxCoord WXUNUSED(x), wxCoord WXUNUSED(y), const wxArrayString &filenames)
{
    //Copy paths to the dialog
    m_frame->AddDropped(filenames);
    return true;
}


/** UNIVERSAL **/

/* Asker's methods */
bool Asker::Ask(const wxString & question)
{
    if(HasGUI)
        return wxMessageBox(question, _("Question"), wxYES_NO | wxICON_QUESTION) == wxYES;
    else
    {
        Write(question+" [y/N]");
        wxChar input;
        if(!wxFgets(&input, sizeof(wxChar), stdin)) return false;
        return (wxStrcmp(input, "y")||wxStrcmp(input, "Y"));
    }
}

wxString Asker::WhereToSave()
{
    if(HasGUI)
    {
        wxDirDialog dlg(NULL, _("Where to save the result?"));
        dlg.ShowModal();
        return dlg.GetPath();
    }
    else
    {
        wxString res; wxChar input[256];
        do
        {
            Write(_("Where to save the result?")+" ");
            if(!wxFgets(input, WXSIZEOF(input), stdin))
                Issues::Add(_("Input failed"));
            else
            {
                res = wxString(input);
                res.Trim();
            }
        }
        while(!wxDirExists(res));
        return res;
    }
}

/* Constructs messages with processing results */
wxString ResultString(int NumFiles, MODE mode)
{
    wxString res;
    if(mode == Encrypt)
        res = wxString::Format(wxPLURAL("Encrypted %i file", "Encrypted %i files", NumFiles), NumFiles);
    else
        res = wxString::Format(wxPLURAL("Decrypted %i file", "Decrypted %i files", NumFiles), NumFiles);
    return res;
}

/* Progress's methods */
//Constructor
Progress::Progress(EntangleFrame * frame)
: m_text(_("Starting...")), current(0), m_total(0), progress(0)
{
    HasGUI = (frame != NULL);
    if(HasGUI)
    {
        dialog = new wxProgressDialog(_("Progress"), _("Starting..."), 100, frame);
        dialog->CenterOnParent();
        dialog->Show();
        dialog->Update(0);
    }
}

//Destructor
Progress::~Progress() { if(HasGUI) delete dialog; }

//Triggers
void Progress::Start()
{
    if(HasGUI)
    {
        dialog->Show();
        dialog->Update(0);
    }
    else
        Write("\n"+m_text);
}

void Progress::Finish(bool success)
{
    if(HasGUI)
        dialog->Update(100, success ? _("Done!") : _("Error"));
    else
        Write("\r"+m_text+"... 100%"+"\n");
    m_text=_("Starting..."); current=0; m_total=0; progress=0;
}

//Setters
void Progress::SetTotal(ullong & total) { m_total = total; }
void Progress::SetText(const wxString & text)
{
    if(HasGUI)
        UpdateDialog(text);
    else
    {
        Write("\r");
        if(m_text!=_("Starting..."))
            Write(m_text+"... 100%"+"\n");
        Write(text+"... 0%");
    }
    m_text = text;
}

void Progress::Increase(const ullong & to_add)
{
    current+=to_add;
    Calc();
    //Updating the progress
    if(HasGUI)
        UpdateDialog();
    else
       Write("\r"+m_text+"... "+ToString(progress)+"%");
}

//Actual progress calculator
void Progress::Calc()
{
    //Dividing by zero is a crime!
    assert(m_total);
    progress = (double)current/m_total*100;
    if(progress>100) progress=100;
}

void Progress::UpdateDialog(const wxString & show_str)
{
    dialog->Update(progress, show_str);
    wxYield();
}


/* Issues's static variables and methods */
bool Issues::console = false;
wxString Issues::m_fname;
wxArrayString Issues::errors;
int Issues::count = 0;

void Issues::SetFile(const wxString & filename)  { m_fname = filename; }

void Issues::Add(const wxString & message, const wxString & filename)
{
    //Producing a human-readable output
    //and pushing the result to the main list.
    wxString info = (filename==""?m_fname:filename)+" ("+message+")";
    if(console)
        errors.push_back(info);
    else
        wxLogError(info);
    ++count;
}

bool Issues::Exist() { return count; }

void Issues::Show()
{
    if(Exist())
    {
        wxString err = wxString::Format(wxPLURAL("%u error:", "%u errors:", count), count);
        if(console)
        {
            Write(err+"\n");
            for(size_t i=0; i < errors.GetCount(); ++i)
                Write(ToString(i+1) + ": " + wxString(errors[i]) + "\n");
        }
        else
        {
            wxLogError(err);
            wxLog::FlushActive();
        }
        count = 0;
    }
}
