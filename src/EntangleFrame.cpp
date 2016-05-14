/***************************************************************
 * Name:      EntangleFrame.cpp
 * Purpose:   Implements the user interface
 * Author:    Ilya Bizyaev (bizyaev@lyceum62.ru)
 * Created:   2015-01-01
 * Copyright: Ilya Bizyaev
 * License:   GNU GPL v3
 **************************************************************/

/** ------------ Include files ------------ **/
#include "EntangleFrame.h"
#include "EntangleMain.h"
#include "extras/UI.h"

#include <wx/aboutdlg.h>        //"About the program" dialog
#include <wx/msgdlg.h>          //Displaying informational messages
#include <wx/artprov.h>         //Stock replacements for images
/** --------------------------------------- **/

using namespace std;


//(*InternalHeaders(EntangleFrame)
#include <wx/string.h>
#include <wx/intl.h>
#include <wx/bitmap.h>
#include <wx/image.h>
//*)

//(*IdInit(EntangleFrame)
const long EntangleFrame::ID_TASKTEXT = wxNewId();
const long EntangleFrame::ID_PASSWORDHINT = wxNewId();
const long EntangleFrame::ID_MODETEXT = wxNewId();
const long EntangleFrame::ID_PASSWORD = wxNewId();
const long EntangleFrame::ID_STARTBUTTON = wxNewId();
const long EntangleFrame::ID_ABOUTBUTTON = wxNewId();
const long EntangleFrame::ID_MODESWITCHER = wxNewId();
const long EntangleFrame::ID_FILESELECTOR = wxNewId();
const long EntangleFrame::ID_PROGRESSDIALOG1 = wxNewId();
//*)

BEGIN_EVENT_TABLE(EntangleFrame,wxFrame)
    //(*EventTable(EntangleFrame)
    //*)
END_EVENT_TABLE()

/* Frame constructor */
EntangleFrame::EntangleFrame(wxWindow* parent, UserData data, wxWindowID id) : mode(data.mode)
{
    //(*Initialize(EntangleFrame)

    //Frame and panel for widgets
    Create(parent, id, _("Entangle"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_FRAME_STYLE & ~(wxRESIZE_BORDER | wxMAXIMIZE_BOX), _T("id"));
    SetFocus();
    panel = new wxPanel(this, wxID_ANY);

    //Main sizer and file selector
    FlexGridSizer1 = new wxFlexGridSizer(5, 1, 0, 0);
    TaskText = new wxStaticText(panel, ID_TASKTEXT, _("Choose files or folders:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_TASKTEXT"));
    FileSelector = new wxGenericDirCtrl(panel, ID_FILESELECTOR, wxEmptyString, wxDefaultPosition, wxSize(190,190), wxDIRCTRL_MULTIPLE, wxEmptyString, 0, _T("ID_FILESELECTOR"));
    FileSelector->SetToolTip(_("You can drop files or folders here"));
    FlexGridSizer1->Add(TaskText, 1, wxTOP|wxLEFT|wxRIGHT|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer1->Add(FileSelector, 1, wxTOP|wxLEFT|wxRIGHT|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, wxDLG_UNIT(this,wxSize(5,0)).GetWidth());

    LoadImages(); //"Encrypt" / "decrypt" images

    //Password input and mode selector
    FlexGridSizer2 = new wxFlexGridSizer(2, 2, 0, 0);
    PasswordHint = new wxStaticText(panel, ID_PASSWORDHINT, _("Enter the password:"), wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE, _T("ID_PASSWORDHINT"));
    TextCtrl1 = new wxTextCtrl(panel, ID_PASSWORD, wxEmptyString, wxDefaultPosition, wxSize(150,28), 0, wxDefaultValidator, _T("ID_PASSWORD"));
    ModeText = new wxStaticText(panel, ID_MODETEXT, _("Mode"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_MODETEXT"));
    ModeSwitcher = new wxBitmapButton(panel, ID_MODESWITCHER, wxBitmap(ui_img[mode]), wxDefaultPosition, wxSize(35,35), wxBU_AUTODRAW, wxDefaultValidator, _T("ID_MODESWITCHER"));

    //Connecting widgets and sizers
    FlexGridSizer2->Add(PasswordHint, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer2->Add(ModeText, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer2->Add(TextCtrl1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer2->Add(ModeSwitcher, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);

    FlexGridSizer1->Add(FlexGridSizer2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);

    BoxSizer1 = new wxBoxSizer(wxHORIZONTAL);
    //Main action buttons
    AboutButton = new wxButton(panel, ID_ABOUTBUTTON, _("About"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_ABOUTBUTTON"));
    StartButton = new wxButton(panel, ID_STARTBUTTON, _("Start"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_STARTBUTTON"));
    StartButton->SetFocus();
    BoxSizer1->Add(AboutButton, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    BoxSizer1->Add(StartButton, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);

    FlexGridSizer1->Add(BoxSizer1, 1, wxBOTTOM|wxLEFT|wxRIGHT|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    panel->SetSizer(FlexGridSizer1);
    FlexGridSizer1->Fit(panel);
    FlexGridSizer1->SetSizeHints(this);
    Center();

    //Events
    Connect(ID_PASSWORD, wxEVT_COMMAND_TEXT_UPDATED, (wxObjectEventFunction)&EntangleFrame::OnPasswordChange);
    Connect(ID_MODESWITCHER, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&EntangleFrame::OnLockClick);
    Connect(ID_STARTBUTTON, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&EntangleFrame::OnStartButtonClick);
    Connect(ID_ABOUTBUTTON, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&EntangleFrame::OnAboutButtonClick);
    Bind(wxEVT_DIRCTRL_SELECTIONCHANGED, &EntangleFrame::OnFileReselect, this, ID_FILESELECTOR);
    //*)

    //Enable Drag & Drop
    SetDropTarget(new DroppedFilesReciever(this));
    //Access the data provided as command line arguments / dropped onto the executable
    received_files = data.tasks;
    TextCtrl1->SetValue(data.password);
    //Collapse the file selector and unselect all
    FileSelector->CollapseTree();
    FileSelector->UnselectAll();
    UpdateTasks();
}

/* Locating and loading images */
void EntangleFrame::LoadImages()
{
    //Main application icon
    #ifdef __WIN32__
        SetIcon(wxICON(aaaaa));
    #else
        wxIcon ui_icon;
        //Portable version
        if(wxFileExists("./img/icon.png"))
            ui_icon.LoadFile("./img/icon.png");
        //Global installation
        else if(wxFileExists("/usr/share/pixmaps/entangle.png"))
            ui_icon.LoadFile("/usr/share/pixmaps/entangle.png");

        if(ui_icon.IsOk())
            SetIcon(ui_icon);
        else
            wxMessageBox(_("Cannot load an icon: ")+wxS("icon.png"));
    #endif

    wxString portable[] = {"./img/Encryption.png", "./img/Decryption.png"};
    wxString global[] = {"/usr/share/Entangle/img/Encryption.png",
                       "/usr/share/Entangle/img/Decryption.png"};

    for(int i = Encrypt; i<=Decrypt; ++i) //For both modes
    {
        //Portable version (default)
        if(wxFileExists(portable[i]))
            ui_img[i].LoadFile(portable[i]);
        //Global installation
        else if(wxFileExists(global[i]))
            ui_img[i].LoadFile(global[i]);
        //The file is not provided
        if(!ui_img[i].IsOk())
        {
            wxMessageBox(_("Cannot load an icon: ") + portable[i]);
            ui_img[i] = wxArtProvider::GetBitmap(wxART_ERROR).ConvertToImage();
        }

    }
}

/** Event processors **/

/* UI's process activation */
void EntangleFrame::OnStartButtonClick(wxCommandEvent& WXUNUSED(event))
{
    //Checking user input
    if(UI_files.IsEmpty()&&received_files.IsEmpty())
    {
        SetText(TASKS, _("No tasks selected!"));
        return;
    }
    if(!TextCtrl1->GetLineLength(0))
    {
        SetText(HINT, _("Enter password first!"));
        return;
    }
    //Get the password
    wxString password = TextCtrl1->GetLineText(0);
    //Join task arrays
    wxArrayString tasks;
    tasks.insert(tasks.end(), UI_files.begin(), UI_files.end());
    tasks.insert(tasks.end(), received_files.begin(), received_files.end());

    //Set the UI to the operational mode
    SetText(HINT, _("Processing...")); wxTheApp->Yield();

    //Connecting main classes
    UserData data(tasks, password, mode);
    Entangle E(data, this);

    //Calling the Entangle's processing method.
    int NumFiles = E.Process();
    //Yeah, that's all (^_^)

    //Check if there were any problems during processing
    if(Issues::Exist())
    {
        //If it is so, show them to the user
        SetText(HINT, _("Went wrong :("));
        Issues::Show();
    }
    else
    {
        wxMessageBox(ResultString(NumFiles, mode), _("Done!"));
        SetText(HINT, _("Done!"));
    }
    //User should reselect files to process them again.
    received_files.Clear();
    UpdateTasks();
}

void EntangleFrame::OnFileReselect(wxTreeEvent& WXUNUSED(event))
{
    UpdateTasks();
}

void EntangleFrame::OnLockClick(wxCommandEvent& WXUNUSED(event))
{
    mode = (MODE)(1-mode); //Swap the mode
    //Update the indicator
    ModeSwitcher->SetBitmap(ui_img[mode]);
}

void EntangleFrame::OnPasswordChange(wxCommandEvent& WXUNUSED(event))
{
    int length = TextCtrl1->GetLineLength(0);
    if(!length)
        SetText(HINT, _("Enter the password:"));
    else if(length > 16)
         SetText(HINT, _("Good")+" (>16)");
    else if(length == 16)
        SetText(HINT, _("Good")+" (16/16)");
    else if(length > 10)
        SetText(HINT, _("Medium")+" ("+ToString(length)+"/16)");
    else
        SetText(HINT, _("Short")+" ("+ToString(length)+"/16)");
    wxTheApp->Yield();
}

void EntangleFrame::OnAboutButtonClick(wxCommandEvent& WXUNUSED(event))
{
    wxAboutDialogInfo about;
    about.SetName("Entangle");
    about.SetVersion("1.0");
    about.SetDescription(_("Simple and user-friendly application\nfor data encryption"));
    about.SetCopyright("(C) Ilya Bizyaev <bizyaev@lyceum62.ru>, 2015-2016");
    about.SetWebSite("GitHub.com/IlyaBizyaev/Entangle");

    //Thanks (^_^)
    about.AddDeveloper("Author: Ilya Bizyaev.\n"
                       "Thanks to...\n"
                       "...doublemax and DenDev for answering my wxQuestions :)\n"
                       "...Jean-Pierre Münch, Jeffrey Walton and Mobile Mouse for help with the encryption.\n"
                       "...RostakaGmfun for testing\n"
                       );

    about.AddTranslator("RostakaGmfun (Ukrainian)");
    about.AddTranslator("Alina Krasnikova (initial German)");

    wxAboutBox(about, this);
    wxTheApp->SafeYield(NULL, false);
}

/** UI-based functions **/
/* Update task counters */
void EntangleFrame::UpdateTasks()
{
    //Get paths from the file tree
	FileSelector->GetPaths(UI_files);

	int chosen = UI_files.size(), dropped = received_files.size();

    wxString first_half, second_half, result;
    if(chosen)
        result = wxString::Format(wxPLURAL("Selected %d", "Selected %d", chosen), chosen);
    second_half = wxString::Format(wxPLURAL("Received %d", "Received %d", dropped), dropped);

    if(!chosen && !dropped)
		result = _("Choose files or folders:");
	if(chosen && !dropped)
		result += wxPLURAL(" object", " objects", chosen);
	else if(dropped && !chosen)
		result = second_half + wxPLURAL(" object", " objects", dropped);
	else if(chosen && dropped)
		result += wxT(", ") + second_half.MakeLower();

	SetText(TASKS, result);
}

void EntangleFrame::AddDropped(const wxArrayString & filenames)
{
    received_files.insert(received_files.end(), filenames.begin(), filenames.end());
    UpdateTasks();
}

void EntangleFrame::SetText(int line, const wxString & message)
{
    if(line==TASKS)
        TaskText->SetLabelText(message);
    else if(line==HINT)
        PasswordHint->SetLabelText(message);
}
