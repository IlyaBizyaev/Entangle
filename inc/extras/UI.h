/***************************************************************
 * Name:      UI.h
 * Purpose:   Defines extras for Entangle UI
 * Author:    Ilya Bizyaev (bizyaev@lyceum62.ru)
 * Created:   24.06.2016
 * Copyright: Ilya Bizyaev
 * License:   GNU GPL v3
 **************************************************************/

#ifndef UI_H
#define UI_H

#include "EntangleFrame.h"

/** ------------ Include files ------------ **/
#include <wx/msgdlg.h>          //Message boxes
#include <wx/dnd.h>             //File drag & drop
#include <wx/dir.h>             //Traversing function
/** --------------------------------------- **/

using namespace std;

/** TERMINAL **/

/* Cross-platform console output */
void Write(const wxString & message);


/** GUI **/

/* Accepts dropped files */
class DroppedFilesReciever : public wxFileDropTarget
{
public:
    DroppedFilesReciever(EntangleFrame * frame);
    //Called when something is dropped onto the window
    bool OnDropFiles(wxCoord x, wxCoord y, const wxArrayString & filenames);
private:
    EntangleFrame * m_frame; //Needed to report success
};


/** UNIVERSAL **/

/* Asks yes/no questions and requests path */
class Asker
{
public:
    Asker(bool has_gui) : HasGUI(has_gui) { }
    bool Ask(const wxString & question);
    wxString WhereToSave();
private:
    bool HasGUI;
};


/* Constructs messages with processing results */
wxString ResultString(int NumFiles, MODE mode);

/* Encapsulates displaying progress (GUI or terminal) */
class Progress
{
public:
    Progress(EntangleFrame * frame = NULL);
    ~Progress();
    //Triggers
    void Start(); void Finish(bool success);
    //Setters
    void SetTotal(ullong & total);
    void SetText(const wxString & text);
    //Called to update the progress
    void Increase(const ullong & to_add);
private:
    bool HasGUI;  //Triggers behavior
    void Calc();  //Actual calculator
    void UpdateDialog(const wxString & text = "");
    //NULL in console mode
    wxProgressDialog * dialog;
    //Data to display
    wxString m_text;
    ullong current, m_total; int progress;
};

/* Operates errors */
class Issues
{
public:
    Issues() {   }
    static void ConsoleMode() { console = true; }
    //Changes filename (useful when reading logs)
    static void SetFile(const wxString & filename);
    //Adds error to the log
    static void Add(const wxString & message, const wxString & filename = "");
    //Manipulate issues
    static bool Exist();
    static void Show();
private:
    static bool console;
    static wxString m_fname;
    static wxArrayString errors;
    static int count;
};

#endif // UI_H
