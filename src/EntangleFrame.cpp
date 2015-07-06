/***************************************************************
 * Name:      EntangleFrame.cpp
 * Purpose:   Implements the user interface
 * Author:    Ilya Bizyaev (bizyaev.game@yandex.ru)
 * Created:   2015-01-01
 * Copyright: Ilya Bizyaev (utor.ucoz.ru)
 * License:   GNU GPL v3
 **************************************************************/

/** ------------ Include files ------------ **/
#include "EntangleMain.h"
#include "EntangleFrame.h"
#include "EntangleApp.h"
#include "EntangleExtras.h"

#include <wx/filename.h>        //File existence and permissions
#include <wx/aboutdlg.h>        //"About the program" dialog
#include <wx/msgdlg.h>          //Displaying informational messages
/** --------------------------------------- **/

using namespace std;
using namespace CryptoPP;


//(*InternalHeaders(EntangleFrame)
#include <wx/string.h>
#include <wx/intl.h>
#include <wx/bitmap.h>
#include <wx/image.h>
//*)

//(*IdInit(EntangleFrame)
const long EntangleFrame::ID_STATICTEXT1 = wxNewId();
const long EntangleFrame::ID_GENERICDIRCTRL1 = wxNewId();
const long EntangleFrame::ID_STATICTEXT2 = wxNewId();
const long EntangleFrame::ID_STATICTEXT3 = wxNewId();
const long EntangleFrame::ID_TEXTCTRL1 = wxNewId();
const long EntangleFrame::ID_BITMAPBUTTON1 = wxNewId();
const long EntangleFrame::ID_BUTTON2 = wxNewId();
const long EntangleFrame::ID_BUTTON1 = wxNewId();
const long EntangleFrame::ID_PROGRESSDIALOG1 = wxNewId();
//*)

BEGIN_EVENT_TABLE(EntangleFrame,wxFrame)
    //(*EventTable(EntangleFrame)
    //*)
END_EVENT_TABLE()

/* Window constructor and destructor */
//wxDIRCTRL_MULTIPLE
EntangleFrame::EntangleFrame(wxWindow* parent,wxWindowID id)
{
    //(*Initialize(wxFramerFrame)
    wxFlexGridSizer* FlexGridSizer1;
    wxFlexGridSizer* FlexGridSizer2;
    wxBoxSizer* BoxSizer1;

    Create(parent, id, _("Entangle"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_FRAME_STYLE & ~(wxRESIZE_BORDER | wxMAXIMIZE_BOX | wxMINIMIZE_BOX), _T("id"));
    SetFocus();
    panel = new wxPanel(this, wxID_ANY);
    FlexGridSizer1 = new wxFlexGridSizer(5, 1, 0, 0);
    StaticText1 = new wxStaticText(panel, ID_STATICTEXT1, _("Choose files or folders:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
    FlexGridSizer1->Add(StaticText1, 1, wxTOP|wxLEFT|wxRIGHT|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    GenericDirCtrl1 = new wxGenericDirCtrl(panel, ID_GENERICDIRCTRL1, wxEmptyString, wxDefaultPosition, wxSize(190,190), 0, wxEmptyString, 0, _T("ID_GENERICDIRCTRL1"));
    FlexGridSizer1->Add(GenericDirCtrl1, 1, wxTOP|wxLEFT|wxRIGHT|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, wxDLG_UNIT(this,wxSize(5,0)).GetWidth());
    FlexGridSizer2 = new wxFlexGridSizer(2, 2, 0, 0);
    StaticText2 = new wxStaticText(panel, ID_STATICTEXT2, _("Enter the password:"), wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE, _T("ID_STATICTEXT2"));
    FlexGridSizer2->Add(StaticText2, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText3 = new wxStaticText(panel, ID_STATICTEXT3, _("Mode"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT3"));
    FlexGridSizer2->Add(StaticText3, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    TextCtrl1 = new wxTextCtrl(panel, ID_TEXTCTRL1, wxEmptyString, wxDefaultPosition, wxSize(150,28), 0, wxDefaultValidator, _T("ID_TEXTCTRL1"));
    FlexGridSizer2->Add(TextCtrl1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    BitmapButton1 = new wxBitmapButton(panel, ID_BITMAPBUTTON1, wxBitmap(wxImage(_T("./img/Encryption.png"))), wxDefaultPosition, wxSize(35,35), wxBU_AUTODRAW, wxDefaultValidator, _T("ID_BITMAPBUTTON1"));
    FlexGridSizer2->Add(BitmapButton1, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer1->Add(FlexGridSizer2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    BoxSizer1 = new wxBoxSizer(wxHORIZONTAL);
    AboutButton = new wxButton(panel, ID_BUTTON2, _("About"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON2"));
    BoxSizer1->Add(AboutButton, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Button1 = new wxButton(panel, ID_BUTTON1, _("Start"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON1"));
    Button1->SetFocus();
    BoxSizer1->Add(Button1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer1->Add(BoxSizer1, 1, wxBOTTOM|wxLEFT|wxRIGHT|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    panel->SetSizer(FlexGridSizer1);
    FlexGridSizer1->Fit(panel);
    FlexGridSizer1->SetSizeHints(this);
    Center();

    Connect(ID_TEXTCTRL1,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&EntangleFrame::OnPasswordChange);
    Connect(ID_BITMAPBUTTON1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&EntangleFrame::OnLockClick);
    Connect(ID_BUTTON2,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&EntangleFrame::OnAbout);
    Connect(ID_BUTTON1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&EntangleFrame::OnButton1Click);
    //*)

    //Enable Drag & Drop
    SetDropTarget(new DroppedFilesReciever(this));
    //Collapse the file choosing tree
    GenericDirCtrl1->CollapseTree();
    //Remove any default selections
    GenericDirCtrl1->UnselectAll();
    //Set event processor for GenericDirControl
    Bind(wxEVT_DIRCTRL_SELECTIONCHANGED, &EntangleFrame::OnFileReselect, this, ID_GENERICDIRCTRL1);
    //Set default mode
    mode = Encrypt;
    //Nothing is yet done ;)
    TasksSelected = false; PasswordTypedIn = false;
}

EntangleFrame::~EntangleFrame()
{
    //(*Destroy(EntangleFrame)
    //*)
}

/* Event processors */
void EntangleFrame::OnButton1Click(wxCommandEvent& WXUNUSED(event))
{
    /* Preparations */
    //Ensure that tasks are selected and the password is typed in
    if(!TasksSelected)
    {
        SetText(1, _("No tasks selected!"));
        return;
    }
    if(!PasswordTypedIn)
    {
        SetText(2, _("Enter password first!"));
        return;
    }
    //Get the password
    wxString password = TextCtrl1->GetLineText(0);
    //Join task arrays
    tasks.insert(tasks.end(), UI_files.begin(), UI_files.end());
    tasks.insert(tasks.end(), drop_files.begin(), drop_files.end());

    //Set the UI to the operational mode
    SetText(2, _("Processing...")); wxTheApp->Yield();
    ProgressDialog1 = new wxProgressDialog(_("Progress"), _("Starting..."), 100, this);
    ProgressDialog1->CenterOnParent();
    ProgressDialog1->Show();

    ProgressDisplayer pdisplay(this);

    //Getting a link to the Entangle singleton
    Entangle& eInst = Entangle::Instance();
    //Initializing it
    eInst.Initialize(tasks, password, mode, &pdisplay);
    //Creating an error tracker
    ErrorTracker e_track;

    //Calling the Entangle's processing method.
    int NumFiles = eInst.Process();
    //Yeah, that's all (^_^)

    //Close the ProgressDialog
    ProgressDialog1->Update(100, _("Done!"));

    //Check if there were any problems during processing
    if(e_track.HasIssues())
    {
        //If it is so, show them to the user
        SetText(2, _("Went wrong :("));
        e_track.ShowIssues();
    }
    else
    {
        wxString text;
        if(mode == Encrypt)
            text = wxString::Format(wxPLURAL("Encrypted %i file", "Encrypted %i files", NumFiles), NumFiles);
        else
            text = wxString::Format(wxPLURAL("Decrypted %i file", "Decrypted %i files", NumFiles), NumFiles);
        wxMessageBox(text, _("Done!")); SetText(2, _("Done!"));
    }
    //Clean task arrays
    drop_files.Clear(); tasks.Clear();
    UpdateTasks();
}

void EntangleFrame::OnFileReselect(wxTreeEvent& WXUNUSED(event))
{
    this->UpdateTasks();
}

void EntangleFrame::OnLockClick(wxCommandEvent& WXUNUSED(event))
{
    /* Reverses current mode */
    if(mode==Decrypt)
        BitmapButton1->SetBitmap(wxImage("./img/Encryption.png"));
    else
        BitmapButton1->SetBitmap(wxImage("./img/Decryption.png"));
    mode = (MODE)(1-(int)mode);
}

void EntangleFrame::OnPasswordChange(wxCommandEvent& WXUNUSED(event))
{
    wxString wxpassword = TextCtrl1->GetLineText(0);
    int length = wxpassword.length();
    PasswordTypedIn = true;
    if(!length)
    {
        SetText(2, _("Enter the password:"));
        PasswordTypedIn = false;
    }
    else if(length > 16)
         SetText(2, _("Good")+" (>16)");
    else if(length == 16)
        SetText(2, _("Good")+" (16/16)");
    else if(length > 10)
        SetText(2, _("Medium")+" ("+ToString(length)+"/16)");
    else
        SetText(2, _("Short")+" ("+ToString(length)+"/16)");
    wxTheApp->Yield();
}

void EntangleFrame::OnAbout(wxCommandEvent& WXUNUSED(event))
{
    wxAboutDialogInfo aboutInfo;
    aboutInfo.SetName("Entangle");
    aboutInfo.SetVersion("0.9.2");
    aboutInfo.SetDescription(_("Simple and user-friendly application\nfor AES-based data encryption"));
    aboutInfo.SetCopyright("(C) Ilya Bizyaev <bizyaev@lyceum62.ru>, 2015");
    aboutInfo.SetWebSite("http://entangle.ucoz.net");
    wxAboutBox(aboutInfo, this);
    wxTheApp->SafeYield(NULL, false);
}

/* UI-based functions */
void EntangleFrame::UpdateTasks()
{
    //Get paths from the file tree
	GenericDirCtrl1->GetPaths(UI_files);

	wxString first_half, second_half, result;
	bool chosen = !UI_files.empty(), dropped = !drop_files.empty();
	size_t num_chosen = UI_files.size(), num_dropped = drop_files.size();

	if(chosen||dropped)
		TasksSelected = true;
	else
		TasksSelected = false;

	if(chosen)
        first_half = wxString::Format(wxPLURAL("Selected %lu", "Selected %lu", num_chosen), num_chosen);

	if(dropped)
        second_half = wxString::Format(wxPLURAL("Received %lu", "Received %lu", num_dropped), num_dropped);

	if(chosen && !dropped)
		result = first_half + wxPLURAL(" object", " objects", num_chosen);
	else if(dropped && !chosen)
		result = second_half + wxPLURAL(" object", " objects", num_dropped);
	else if(chosen && dropped)
		result = first_half + wxT(", ") + second_half.MakeLower();
	else if(!chosen && !dropped)
		result = _("Choose files or folders:");

	SetText(1, result);
}

void EntangleFrame::AddDropped(wxArrayString filenames)
{
    //Adding tasks to the drop_files array
    drop_files.insert(drop_files.end(), filenames.begin(), filenames.end());
    //Update the UI
    UpdateTasks();
}

void EntangleFrame::UpdateProgress(int progress, wxString show_str)
{
    ProgressDialog1->Update(progress, show_str);
    wxTheApp->Yield();
}

void EntangleFrame::SetText(int line, wxString message)
{
    if(line==1)
        StaticText1->SetLabelText(message);
    else if(line==2)
        StaticText2->SetLabelText(message);
}
