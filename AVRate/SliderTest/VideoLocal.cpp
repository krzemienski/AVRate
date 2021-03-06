#include "StdAfx.h"
#include "VideoLocal.h"

#include <stdio.h>
#include <windows.h>
#include <winuser.h>

using namespace System;
using namespace System::Windows;
using namespace System::Windows::Forms;

[System::Runtime::InteropServices::DllImport("user32.dll")]
extern int ShowWindow(IntPtr hWnd, int nCmdShow);

[System::Runtime::InteropServices::DllImport("user32.dll")] 
extern int SetActiveWindow(IntPtr hWnd);

[System::Runtime::InteropServices::DllImport("user32.dll")]
extern int SystemParametersInfo(int uAction , int uParam , System::String ^lpvParam , int fuWinIni);

[System::Runtime::InteropServices::DllImport("user32.dll")] 
extern int SetForegroundWindow(IntPtr hWnd);

VideoLocal::VideoLocal(Controller ^c, LocalPlayer l, String^ pp) {
	

	// overloaded constructor needed for FormSoftwareConst (at least) to be able to communicate via controller
	controller = c;
	playerPath = pp;
	localPlayer = l;
	currentmediawidth=0;
	currentmediaheight=0;
	startthreaded = false;
}
void VideoLocal::PreparePlay(PlaylistItemPlayable^ playitem) {
	/* can be activated when the threading issue with wmp could be solved - which probably never will due to ActiveX architecture
	if (localPlayer == LocalPlayer::WMP) {
		String^ fileName = playitem->getName();
		WMPStarterPrepare(fileName);
	}*/
}

void VideoLocal::Play(PlaylistItemPlayable^ playitem) {
	videoFileName = playitem->getName();	

	if (localPlayer == LocalPlayer::WMP) 
	{
		WMPStarterPrepare(videoFileName);			// see VideoLocal::PreparePlay
		//System::Threading::Thread::Sleep(controller->generalSettings->sleepTime);
		WMPStarterPlay();
	}
	else if (localPlayer == LocalPlayer::VLC10)
	{
		//System::Threading::Thread::Sleep(controller->generalSettings->sleepTime);
		VLC10StartProcess(videoFileName);
	}
	else if (localPlayer == LocalPlayer::MPlayer) 
	{
		//System::Threading::Thread::Sleep(controller->generalSettings->sleepTime);
		MPlayerStarterPlay(videoFileName);
	}
	else if (localPlayer == LocalPlayer::STEREOSCOPIC_PLAYER_NVIDIA)
	{
		StereoscopicPlayerNVidiaStarterPlay(videoFileName);
	}
	else if (localPlayer == LocalPlayer::STEREOSCOPIC_PLAYER_SBS)
	{
		StereoscopicPlayerSbSStarterPlay(videoFileName);
	}
	else if (localPlayer == LocalPlayer::COMMAND_LINE)
	{
		CommandLinePlayerStarterPlay(videoFileName);
	}
}
void VideoLocal::PlayThreaded(PlaylistItemPlayable^ playitem) {
	startthreaded = true;
	videoFileName = playitem->getName();	
	backgroundWorker1 = (gcnew System::ComponentModel::BackgroundWorker());

	if (localPlayer == LocalPlayer::WMP) 
	{
		WMPStarterPrepare(videoFileName);			// see VideoLocal::PreparePlay
		WMPStarterPlay();
	}
	else if (localPlayer == LocalPlayer::VLC10)
	{
		backgroundWorker1->DoWork += gcnew System::ComponentModel::DoWorkEventHandler(this, &VideoLocal::VLC10StartProcess);
		VLC10StarterPlay(videoFileName);
	}
	else if (localPlayer == LocalPlayer::MPlayer) 
	{
		backgroundWorker1->DoWork += gcnew System::ComponentModel::DoWorkEventHandler(this, &VideoLocal::MPlayerStartProcess);
		MPlayerStarterPlay(videoFileName);
	}
	else if (localPlayer == LocalPlayer::STEREOSCOPIC_PLAYER_NVIDIA) 
	{
		backgroundWorker1->DoWork += gcnew System::ComponentModel::DoWorkEventHandler(this, &VideoLocal::StereoscopicPlayerNVidiaStartProcess);
		StereoscopicPlayerNVidiaStarterPlay(videoFileName);
	}
	else if (localPlayer == LocalPlayer::STEREOSCOPIC_PLAYER_SBS) 
	{
		backgroundWorker1->DoWork += gcnew System::ComponentModel::DoWorkEventHandler(this, &VideoLocal::StereoscopicPlayerSbSStartProcess);
		StereoscopicPlayerSbSStarterPlay(videoFileName);
	}
	else if (localPlayer == LocalPlayer::COMMAND_LINE) 
	{
		backgroundWorker1->DoWork += gcnew System::ComponentModel::DoWorkEventHandler(this, &VideoLocal::CommandLinePlayerStartProcess);
		CommandLinePlayerStarterPlay(videoFileName);
	}
	//startthreaded = false;	// check if interacts
}
void VideoLocal::ProcessStopped(void) {
	controller->invokeEndPlayEvent(videoFileName);
	controller->setPlayStatus(false);
	controller->setKeytrackStatus(false);
	EndPlay();
}
void VideoLocal::ProcessStopped(System::Object ^ obj, System::ComponentModel::RunWorkerCompletedEventArgs ^ args) {
	ProcessStopped();
}
void VideoLocal::VLC10StarterPlay(String^ fileName) 
{
	if(startthreaded)
		backgroundWorker1->RunWorkerAsync(fileName);
	else
		VLC10StartProcess(fileName);
}
void VideoLocal::VLC10StartProcess(System::Object ^ obj, System::ComponentModel::DoWorkEventArgs ^ args) {	 
	VLC10StartProcess(videoFileName);
}

// -------------****----------------****------------------***-----------
// -------------****----------------****------------------***-----------


void VideoLocal::VLC10StartProcess(String^ fileName) {	 
	targetposx = System::Windows::Forms::Screen::PrimaryScreen->Bounds.Right;// new version
	targetposy = System::Windows::Forms::Screen::PrimaryScreen->Bounds.Bottom;// new version
	String^ xscr= String::Format("{0}",targetposx);
	String^ yscr= String::Format("{0}",targetposy);

	// Draw a grey background form
	myForm = gcnew Form();
	myForm->ControlBox = false;	 	 
	myForm->StartPosition = FormStartPosition::Manual;
	myForm->Left = 0;
	myForm->Top = 0;
	myForm->Width =  System::Windows::Forms::Screen::PrimaryScreen->Bounds.Right;//->WorkingArea.Width;
	myForm->Height = System::Windows::Forms::Screen::PrimaryScreen->Bounds.Bottom;//->WorkingArea.Height;
	//myForm->TopMost = true;
	myForm->ShowInTaskbar = false;
	myForm->Show(); 

	// Prepare and start the VLC Process
	System::Diagnostics::ProcessStartInfo^ startInfo;
	System::Diagnostics::Process^ p;
	startInfo = gcnew System::Diagnostics::ProcessStartInfo(playerPath);
	startInfo->Arguments = 
	"--no-drop-late-frames " +	// do not drop frames. slow computer looses sync.
	"--no-skip-frames " +		// 
	"--no-overlay " +			// overlay
	"--no-video-deco " + 
	"--high-priority " + 
	"--no-video-title-show " +		
//	"--no-show-intf " + 
	//"--no-embedded-video " +  // used when --video-x is used
	"--width=" + xscr + " " +	// for new version
	"--height=" + yscr + " " +	// for new version 
	//"--video-x=\"" + xscr + "\" " +	// for wmp based version
	//"--video-y=\"" + yscr + "\" " +	// for wmp based version 
	"--intf=\"dummy\" " +	// intf=rc doesnt work, rejects closing
	"--dummy-quiet " +		// no command line window
	"--no-autoscale " +		// new version: important when window size bigger than video
	videoFileName + " " +
	// non-used command line options :
	//"--video-title-timeout=3000 " +	// 
	//"--video-on-top " +			// not needed for now, shows up on top anyway
	//"--no-skins2-taskbar " +	// should remove taskbar icon but still shows up
	//"--height=-1 " +		// skaliert das video neu
	//"--width=-1 " +		// skaliert das video neu
	//"--fullscreen " +			//skaliert das Bild neu
	//"--mouse-hide-timeout=1 " +		// doesnt work
	//"--align=/{0/} " +			// {0 (Zentriert=Standard), 1 (Links), 2 (Rechts), 4 (Oben), 8 (Unten), 5 (Oben links), 6 (Oben rechts), 9 (Unten links), 10 (Unten rechts)}
	//"vlc://pause:2 " +	// pausiert ohne videoausgabe (wie thread:sleep)
	//"--quiet-synchro" +		// unbekante auswirkung
	"vlc://quit " ;		// special playlist item, to quit after playing the item
	startInfo->UseShellExecute = false;
	startInfo->WindowStyle = System::Diagnostics::ProcessWindowStyle::Normal;
	//startInfo->CreateNoWindow = true;//führt zu nix wegen Username/pw properties
	System::Threading::Thread::Sleep(controller->generalSettings->sleepTime);
	controller->setPlayStatus(true);
	p = System::Diagnostics::Process::Start(startInfo);
	controller->setKeytrackStatus(true);
	p->WaitForExit();
	ProcessStopped();

	myForm->Close();
}



// -------------****----------------****------------------***-----------
// -------------****----------------****------------------***-----------



void VideoLocal::StereoscopicPlayerSbSStartProcess(System::Object ^ obj, System::ComponentModel::DoWorkEventArgs ^ args) {	 
	StereoscopicPlayerSbSStarterPlay(videoFileName);
}
void VideoLocal::StereoscopicPlayerSbSStarterPlay(String^ fileName) {	 
	targetposx = System::Windows::Forms::Screen::PrimaryScreen->Bounds.Right;// new version
	targetposy = System::Windows::Forms::Screen::PrimaryScreen->Bounds.Bottom;// new version
	String^ xscr= String::Format("{0}",targetposx);
	String^ yscr= String::Format("{0}",targetposy);

	// Draw a grey background form
	myForm = gcnew Form();
	myForm->ControlBox = false;	 	 
	myForm->StartPosition = FormStartPosition::Manual;
	myForm->Left = 0;
	myForm->Top = 0;
	myForm->Width =  System::Windows::Forms::Screen::PrimaryScreen->Bounds.Right;//->WorkingArea.Width;
	myForm->Height = System::Windows::Forms::Screen::PrimaryScreen->Bounds.Bottom;//->WorkingArea.Height;
	//myForm->TopMost = true;
	myForm->ShowInTaskbar = false;
	myForm->BackColor = System::Drawing::Color::Black;
	myForm->FormBorderStyle = System::Windows::Forms::FormBorderStyle::None;
	myForm->Show(); 

	// Prepare and start the VLC Process
	System::Diagnostics::ProcessStartInfo^ startInfo;
	System::Diagnostics::Process^ p;
	startInfo = gcnew System::Diagnostics::ProcessStartInfo(playerPath);
	startInfo->WindowStyle = System::Diagnostics::ProcessWindowStyle::Minimized;
	startInfo->Arguments = 
	"-file:"+videoFileName +  		// input file name
		" -il:SideBySideLF " +			// Side by side Left view first
		"-ol:SideBySide3DTV " +			// output to side by side for 3DTV
		"-fss " +						// Full Screen
		"-nl "	+						// Do not display stereoscopic player logo at startup
		"-play " +						// play immediately
		"-fp " +
		"-termend ";					// close after the end of the video

	startInfo->UseShellExecute = false;
	startInfo->WindowStyle = System::Diagnostics::ProcessWindowStyle::Normal;
	//startInfo->CreateNoWindow = true;//führt zu nix wegen Username/pw properties
	System::Threading::Thread::Sleep(controller->generalSettings->sleepTime);
	controller->setPlayStatus(true);
	p = System::Diagnostics::Process::Start(startInfo);


	controller->setKeytrackStatus(true);
	p->WaitForExit(); 
	ProcessStopped();
}


// -------------****----------------****------------------***-----------
// -------------****----------------****------------------***-----------


void VideoLocal::StereoscopicPlayerNVidiaStartProcess(System::Object ^ obj, System::ComponentModel::DoWorkEventArgs ^ args) {	 
	StereoscopicPlayerNVidiaStarterPlay(videoFileName);
}
void VideoLocal::StereoscopicPlayerNVidiaStarterPlay(String^ fileName) {	 
	targetposx = System::Windows::Forms::Screen::PrimaryScreen->Bounds.Right;// new version
	targetposy = System::Windows::Forms::Screen::PrimaryScreen->Bounds.Bottom;// new version
	String^ xscr= String::Format("{0}",targetposx);
	String^ yscr= String::Format("{0}",targetposy);

	// Draw a grey background form
	myForm = gcnew Form();
	myForm->ControlBox = false;	 	 
	myForm->StartPosition = FormStartPosition::Manual;
	myForm->Left = 0;
	myForm->Top = 0;
	myForm->Width =  System::Windows::Forms::Screen::PrimaryScreen->Bounds.Right;//->WorkingArea.Width;
	myForm->Height = System::Windows::Forms::Screen::PrimaryScreen->Bounds.Bottom;//->WorkingArea.Height;
	//myForm->TopMost = true;
	myForm->ShowInTaskbar = false;
	myForm->Show(); 

	// Prepare and start the VLC Process
	System::Diagnostics::ProcessStartInfo^ startInfo;
	System::Diagnostics::Process^ p;
	startInfo = gcnew System::Diagnostics::ProcessStartInfo(playerPath);
	startInfo->WindowStyle = System::Diagnostics::ProcessWindowStyle::Minimized;
	startInfo->Arguments = 
	"-file:"+videoFileName +  		// input file name
		" -il:SideBySideLF " +			// Side by side Left view first
		"-ol:NVIDIA " +					// use NVidia 3D Vision
		"-fss " +						// Full Screen
		"-nl "	+						// Do not display stereoscopic player logo at startup
		"-play " +						// play immediately
		"" +
		"-termend ";					// close after the end of the video

	startInfo->UseShellExecute = false;
	startInfo->WindowStyle = System::Diagnostics::ProcessWindowStyle::Normal;
	//startInfo->CreateNoWindow = true;//führt zu nix wegen Username/pw properties
	System::Threading::Thread::Sleep(controller->generalSettings->sleepTime);
	controller->setPlayStatus(true);
	p = System::Diagnostics::Process::Start(startInfo);
	controller->setKeytrackStatus(true);
	p->WaitForExit();
	ProcessStopped();

	myForm->Close();
}


// -------------****----------------****------------------***-----------
// -------------****----------------****------------------***-----------


void VideoLocal::CommandLinePlayerStarterPlay(String^ fileName) 
{
	targetposx = System::Windows::Forms::Screen::PrimaryScreen->Bounds.Right;// new version
	targetposy = System::Windows::Forms::Screen::PrimaryScreen->Bounds.Bottom;// new version
	String^ xscr= String::Format("{0}",targetposx);
	String^ yscr= String::Format("{0}",targetposy);




	// Prepare and start the Process
	System::Diagnostics::ProcessStartInfo^ startInfo;
	System::Diagnostics::Process^ p;
	startInfo = gcnew System::Diagnostics::ProcessStartInfo(playerPath);
	startInfo->WindowStyle = System::Diagnostics::ProcessWindowStyle::Hidden;
	startInfo->Arguments = controller->generalSettings->playerArguments->Replace("%1", fileName);

	startInfo->UseShellExecute = false;
	startInfo->WindowStyle = System::Diagnostics::ProcessWindowStyle::Hidden;
	
	System::Threading::Thread::Sleep(controller->generalSettings->sleepTime);
	controller->setPlayStatus(true);
	p = System::Diagnostics::Process::Start(startInfo);

	controller->setKeytrackStatus(true);


	// Draw a grey background form
	myForm = gcnew Form();
	myForm->ControlBox = false;	 	 
	myForm->StartPosition = FormStartPosition::Manual;
	myForm->Left = 0;
	myForm->Top = 0;
	myForm->Width =  System::Windows::Forms::Screen::PrimaryScreen->Bounds.Right;//->WorkingArea.Width;
	myForm->Height = System::Windows::Forms::Screen::PrimaryScreen->Bounds.Bottom;//->WorkingArea.Height;
	//myForm->TopMost = true;
	myForm->ShowInTaskbar = false;
	myForm->BackColor = System::Drawing::Color::Black;
	myForm->FormBorderStyle = System::Windows::Forms::FormBorderStyle::None;
	myForm->Show(); 


	p->WaitForExit(); 
	ProcessStopped();

	myForm->Close();
}
void VideoLocal::CommandLinePlayerStartProcess(System::Object ^ obj, System::ComponentModel::DoWorkEventArgs ^ args) {	 
	CommandLinePlayerStarterPlay(videoFileName);
}

// -------------****----------------****------------------***-----------
// -------------****----------------****------------------***-----------


void VideoLocal::wmpForm_Shown(Object^ obj, EventArgs^ e)
{
	// The WMP Play command is not sent before the Form has really shown up
	//myWMP->Ctlcontrols->play(); // starts somehow threaded, won't block!
}

void VideoLocal::WMPStarterPrepare(String^ fileName) {
	// 
	 myWMP = gcnew AxWMPLib::AxWindowsMediaPlayer();

	 myForm = gcnew Form();
	 myForm->Shown += gcnew System::EventHandler( this, &VideoLocal::wmpForm_Shown);
	 myForm->ControlBox = false;
	 myForm->FormBorderStyle = System::Windows::Forms::FormBorderStyle::None;
	 myForm->StartPosition = FormStartPosition::Manual;
	 myForm->Left = 0;//System::Windows::Forms::Screen::PrimaryScreen->WorkingArea.Width;
	 myForm->Top = 0;//System::Windows::Forms::Screen::PrimaryScreen->WorkingArea.Height; 
     myForm->Width =  System::Windows::Forms::Screen::PrimaryScreen->Bounds.Right;
     myForm->Height = System::Windows::Forms::Screen::PrimaryScreen->Bounds.Bottom;
	 myForm->Controls->Add(myWMP);
			//myForm->Cursor = myForm->Parent->Cursor;// 
			//myForm->Cursor->Clip = myForm->Parent->Cursor->Clip;
	 myForm->ShowInTaskbar = false;
	 myForm->Show();
	// myForm->Hide();	// dont hide WMP when it is used as Video Player

	 myWMP->Left = 0;
	 myWMP->Top = 0;
	 myWMP->uiMode = "none";	// no controls
	 myWMP->PlayStateChange += gcnew AxWMPLib::_WMPOCXEvents_PlayStateChangeEventHandler(this, &VideoLocal::playstateChangedWMP);
	 //myWMP->OpenStateChange += gcnew AxWMPLib::_WMPOCXEvents_OpenStateChangeEventHandler(this, &VideoLocal::openstateChangedWMP);
	 myWMP->settings->autoStart = false;
	 myWMP->URL = fileName;
	 myWMP->windowlessVideo = true;
	 myWMP->stretchToFit = true;// needed?
}
void VideoLocal::WMPStarterPlay() {	 
	System::Threading::Thread::Sleep(controller->generalSettings->sleepTime);
	controller->setPlayStatus(true);// moved to here
	controller->setKeytrackStatus(true);
	myWMP->Ctlcontrols->pause(); 
	//myWMP->Ctlcontrols->play(); // starts somehow threaded, won't block!
}

void VideoLocal::playstateChangedWMP(System::Object ^,AxWMPLib::_WMPOCXEvents_PlayStateChangeEvent ^e) {
	// The order is as follows: (to reduce frame slippage)
	// play -> pause -> resize window -> goto previous (goto 0) -> play -> stop when finished 
	static bool startedOnce = false;

	if(e->newState == int(WMPLib::WMPPlayState::wmppsReady))
	{
		if(!startedOnce)
		{
			startedOnce = true;
			myWMP->Width = 	myWMP->Ctlcontrols->currentItem->imageSourceWidth;
			myWMP->Height = 	myWMP->Ctlcontrols->currentItem->imageSourceHeight;
			myWMP->Ctlcontrols->play();
		}
	}
	if(e->newState == int(WMPLib::WMPPlayState::wmppsPlaying))
	{
		//controller->setPlayStatus(true);// moved to WMPStarterPlay (causes inconsitencies)
		int xscr = System::Windows::Forms::Screen::PrimaryScreen->Bounds.Right;
		int yscr = System::Windows::Forms::Screen::PrimaryScreen->Bounds.Bottom;
		currentmediawidth = myWMP->Ctlcontrols->currentItem->imageSourceWidth;
		currentmediaheight = myWMP->Ctlcontrols->currentItem->imageSourceHeight;
		myWMP->Width = currentmediawidth;
		myWMP->Height = currentmediaheight;
		if(currentmediawidth > xscr)			myWMP->Left =0;
		else							myWMP->Left = (xscr - currentmediawidth) / 2;
		if(currentmediaheight > yscr)			myWMP->Top = 0;
		else							myWMP->Top = (yscr - currentmediaheight) / 2;
	}
	if (e->newState == int(WMPLib::WMPPlayState::wmppsStopped))
	{
		controller->setPlayStatus(false);
		controller->setKeytrackStatus(false);
		ProcessStopped();
		myWMP->close();
		myForm->Controls->Remove(myWMP);
		myForm->Close();
		startedOnce = false;
	}
}
void VideoLocal::MPlayerStarterPlay(String^ fileName) 
{
		if(startthreaded)
			backgroundWorker1->RunWorkerAsync(fileName);
		else
			MPlayerStartProcess(fileName);
}
void VideoLocal::MPlayerStartProcess(System::Object ^ obj, System::ComponentModel::DoWorkEventArgs ^ args) {	 
	MPlayerStartProcess(videoFileName);
}
void VideoLocal::MPlayerStartProcess(String^ fileName) {

	System::Diagnostics::ProcessStartInfo^ startInfo;
	System::Diagnostics::Process^ p;
	
	startInfo = gcnew System::Diagnostics::ProcessStartInfo(playerPath);
	startInfo->Arguments = 
			L" −geometry 50%:50% -noborder -really-quiet " + 
			fileName;
	//startInfo->UserName = "";
	startInfo->UseShellExecute = true;	// False opens up Shell window, true asks for execution (only first time)
	startInfo->WindowStyle = System::Diagnostics::ProcessWindowStyle::Hidden;

	 myForm = gcnew Form();
	 myForm->ControlBox = false;	  
	 myForm->StartPosition = FormStartPosition::Manual;
	 myForm->Left = 0;//System::Windows::Forms::Screen::PrimaryScreen->WorkingArea.Width;
	 myForm->Top = 0;//System::Windows::Forms::Screen::PrimaryScreen->WorkingArea.Height; 
	 myForm->Width =  System::Windows::Forms::Screen::PrimaryScreen->Bounds.Right;
	 myForm->Height = System::Windows::Forms::Screen::PrimaryScreen->Bounds.Bottom;
	 //myForm->TopMost = true;	// hides also the player screen
	 myForm->ShowInTaskbar = false;
	 //myForm->ShowWithoutActivation = true;
	 myForm->Show();
	 //myForm->TopLevel = false;
	System::Threading::Thread::Sleep(controller->generalSettings->sleepTime);// threaded and non-threaded, checked.
	controller->setPlayStatus(true);
	p = System::Diagnostics::Process::Start(startInfo);
	controller->setKeytrackStatus(true);
	p->WaitForExit();
	ProcessStopped();

	myForm->Close();
}
void VideoLocal::myTimerProc(Object^ , EventArgs^ ) {
/*
	if (myWMP->playState == WMPLib::WMPPlayState::wmppsPlaying) {
		WMPLib::IWMPMedia^ m = (WMPLib::IWMPMedia^)myWMP->currentMedia;


		bool fs = false;
		if (fs) myWMP->fullScreen = true;
		else {
			myForm->Width = m->imageSourceWidth;
			myForm->Height = m->imageSourceHeight;
			myWMP->Width = m->imageSourceWidth;
			myWMP->Height = m->imageSourceHeight;

			int width = System::Windows::Forms::Screen::PrimaryScreen->WorkingArea.Width;
			int height = System::Windows::Forms::Screen::PrimaryScreen->WorkingArea.Height;
			myForm->Left = width / 2 - myWMP->Width / 2;
			myForm->Top = height / 2 - myWMP->Height / 2;
		}
		t->Stop();
	}
	*/
}

void VideoLocal::EndPlay() { }
