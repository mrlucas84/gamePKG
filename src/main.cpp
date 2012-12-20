// ----------------------------------------------------------------------------
// CaptainCPS-X's gamePKG Tool
// ----------------------------------------------------------------------------
/*

Tested on: Rogero CFW 4.30 v2.03

TODO: 

- support re-scaning devices for new PKG files
- verify HDD for previously queued files
- display a Progress bar
- support split PKG files (for now)
- support removing Queued PKG (for now)

- Clean code

*/
// ----------------------------------------------------------------------

#include "main.h"
#include "misc.h"
#include "gamePKG.h"

CapApp app;

void callbackfunction(int buttonType, void */*userData*/);
void cbFileCopyDlg(int buttonType, void */*userData*/);

bool CapApp::onInit(int argc, char **argv) 
{
	gamePKG = new c_gamePKG();

    // always call the superclass's method
    ::FWGLApplication::onInit(argc, argv);

	dbgFontInit();

	::FWInputDevice *pPad = FWInput::getDevice(FWInput::DeviceType_Pad, 0);

	if(pPad != NULL)
	{
		mpCircle = pPad->bindFilter();
		mpCircle->setChannel(FWInput::Channel_Button_Circle);
		
		mpCross = pPad->bindFilter();
		mpCross->setChannel(FWInput::Channel_Button_Cross);
		
		mpUp = pPad->bindFilter();
		mpUp->setChannel(FWInput::Channel_Button_Up);

		mpDown = pPad->bindFilter();
		mpDown->setChannel(FWInput::Channel_Button_Down);
	}

	gamePKG->RefreshPKGList();

    // return true to indicate success
    return true;
}

CapApp::CapApp() : 
	mFrame(0),
	mIsCirclePressed(false),
	mIsCrossPressed(false),
	mIsUpPressed(false),
	mIsDownPressed(false)
{
    // set up mStartupInfo if necessary
}

void CapApp::dbgFontInit(void)
{
	// initialize debug font library, then open 2 consoles
	int ret;

	CellDbgFontConfig cfg;
	memset(&cfg, 0, sizeof(CellDbgFontConfig));
	
	cfg.bufSize      = 2048;
	cfg.screenWidth  = mDispInfo.mWidth;
	cfg.screenHeight = mDispInfo.mHeight;
	
	ret = cellDbgFontInit(&cfg);

	if (ret != CELL_OK) 
	{
		cellMsgDialogOpen(CELL_MSGDIALOG_DIALOG_TYPE_NORMAL, "cellDbgFontInit() failed", callbackfunction, NULL, NULL);
		return;
	}

	CellDbgFontConsoleConfig ccfg[2];
	memset(ccfg, 0, sizeof(ccfg));

	ccfg[0].posLeft     = 0.1f;
	ccfg[0].posTop      = 0.8f;
	ccfg[0].cnsWidth    = 32;
	ccfg[0].cnsHeight   = 4;
	ccfg[0].scale       = 1.0f;

	// ABGR -> orange
	ccfg[0].color       = 0xff0080ff;

	//mDbgFontID[0] = cellDbgFontConsoleOpen(&ccfg[0]);

	if (mDbgFontID[0] < 0) 
	{
		cellMsgDialogOpen(CELL_MSGDIALOG_DIALOG_TYPE_NORMAL, "cellDbgFontConsoleOpen() failed", callbackfunction, NULL, NULL);
		return;
	}

	ccfg[1].posLeft     = 0.25f;
	ccfg[1].posTop      = 0.2f;
	ccfg[1].cnsWidth    = 256;
	ccfg[1].cnsHeight   = 64;
	ccfg[1].scale       = 0.5f;

	// ABGR -> pale blue
	ccfg[1].color       = 0xffff8080;

	if (mDbgFontID[1] < 0) 
	{
		cellMsgDialogOpen(CELL_MSGDIALOG_DIALOG_TYPE_NORMAL, "cellDbgFontConsoleOpen() failed", callbackfunction, NULL, NULL);
		return;
	}
}

void CapApp::dbgFontPut(void)
{
	float xPos = 0.05f;
	float yPos = 0.05f;
	float yPosDiff = 0.03f;	
	float nFontSize = 0.6f;

	::cellDbgFontPuts(xPos, yPos, nFontSize, 0xffffffff, "----------------------------------------------------------------------" );
	yPos += yPosDiff;	
	::cellDbgFontPuts(xPos, yPos, nFontSize, 0xffffffff, " gamePKG Tool v1.1 - by CaptainCPS-X [2012] [Yes the UI sucks...LOL]");
	yPos += yPosDiff;
	::cellDbgFontPuts(xPos, yPos, nFontSize, 0xffffffff, "----------------------------------------------------------------------" );
	yPos += yPosDiff;
	::cellDbgFontPuts(xPos, yPos, nFontSize, 0xffffffff, "PRESS -(O)- TO EXIT AND RETURN TO XMB");
	yPos += yPosDiff;
	::cellDbgFontPuts(xPos, yPos, nFontSize, 0xffffffff, "PRESS -(X)- TO QUEUE PKG ON XMB");
	yPos += yPosDiff;
	//::cellDbgFontPuts(xPos, yPos, nFontSize, 0xffffffff, "PRESS -(/\)- TO VIEW FILE INFO");
	//yPos += yPosDiff;
	//::cellDbgFontPuts(xPos, yPos, nFontSize, 0xffffffff, "PRESS -([ ])- TO REMOVE FROM QUEUE");
	//yPos += yPosDiff;
	//::cellDbgFontPuts(xPos, yPos, nFontSize, 0xffffffff, "PRESS -[SELECT]- TO WIPE ALL QUEUES & REFRESH DEVICES");
	//yPos += yPosDiff;
	::cellDbgFontPuts(xPos, yPos, nFontSize, 0xffffffff, "----------------------------------------------------------------------" );
	yPos += yPosDiff;

	// Main menu
	uint32_t block_size = 0;
	uint64_t free_block_count = 0;
	
	::cellFsGetFreeSize("/dev_hdd0", &block_size, &free_block_count);

	uint64_t nFreeHDDSpace = block_size * free_block_count;
	
	::cellDbgFontPrintf(xPos, yPos, nFontSize, 0xffffffff, "Free Space on /dev_hdd0/: %.2f %s (%lld bytes)", 
		GetByteUnit(nFreeHDDSpace), 
		GetByteUnitStr(nFreeHDDSpace),
		nFreeHDDSpace
	);
	yPos += yPosDiff;

	//::cellDbgFontPrintf(xPos, yPos, nFontSize, 0xffffffff, "Free Space on /dev_usb000/: %.2f %s (%lld bytes)", 
	//	GetByteUnit(nFreeUSB000Space), 
	//	GetByteUnitStr(nFreeUSB000Space),
	//	nFreeUSB000Space
	//	//GetNumWithCommas(nFreeUSB000Space)
	//);
	//yPos += yPosDiff;

	::cellDbgFontPrintf(xPos, yPos, nFontSize, 0xffffffff, 
		"Total PKG found on all scanned devices: %d", gamePKG->nTotalPKG);

	yPos += yPosDiff;

	::cellDbgFontPuts(xPos, yPos, nFontSize, 0xffffffff, "----------------------------------------------------------------------" );
	yPos += yPosDiff;

	int nPKGListMax = 14;

	if(gamePKG->nSelectedPKG > nPKGListMax || gamePKG->nPKGListTop > 0)
	{
		if(gamePKG->nPKGListTop < (gamePKG->nSelectedPKG - nPKGListMax))
		{
			gamePKG->nPKGListTop = gamePKG->nSelectedPKG - nPKGListMax;
		}

		if(gamePKG->nPKGListTop > 0) 
		{
			if(gamePKG->nSelectedPKG < gamePKG->nPKGListTop)
			{
				gamePKG->nPKGListTop = gamePKG->nSelectedPKG;
			}
		} else {
			gamePKG->nPKGListTop = gamePKG->nSelectedPKG - nPKGListMax;
		}
	} else {
		gamePKG->nPKGListTop = gamePKG->nSelectedPKG - nPKGListMax;
	}

	if(gamePKG->nPKGListTop < 0) gamePKG->nPKGListTop = 0;

	int nPKG = gamePKG->nPKGListTop;
	
	while(nPKG <= (gamePKG->nPKGListTop + nPKGListMax))
	{
		if(nPKG == gamePKG->nTotalPKG) break;

		uint32_t nColor	= 0xffffffff;	// white
		nFontSize = 0.6f;

		// PKG QUEUED
		if(gamePKG->pkglst[nPKG]->bQueued) 
		{
			nColor = 0xff00ff00;		// green
		}

		// PKG SELECTED
		if(nPKG == gamePKG->nSelectedPKG) 
		{
			nColor = 0xff00ccff;		// yellow
			nFontSize = 0.8f;

			if(gamePKG->pkglst[nPKG]->bQueued) {
				nColor = 0xff0000ff;	// red
			}
		}

		::cellDbgFontPrintf(xPos, yPos, nFontSize, nColor, "[%d] %s [%.2f %s] %s %s", 
			nPKG+1, 
			gamePKG->pkglst[nPKG]->title,
			GetByteUnit(gamePKG->pkglst[nPKG]->nSize), GetByteUnitStr(gamePKG->pkglst[nPKG]->nSize),
			gamePKG->pkglst[nPKG]->bInternal ? "[HDD]" : "[USB]",
			gamePKG->pkglst[nPKG]->bQueued ? "[Queued]" : " "
		);

		yPos += yPosDiff;

		nPKG++;
	}

	nFontSize = 0.6f;

	::cellDbgFontPuts(xPos, yPos, nFontSize, 0xffffffff, "----------------------------------------------------------------------" );
	yPos += yPosDiff;
}

void CapApp::dbgFontDraw(void)
{
	::cellDbgFontDraw();
}

void CapApp::dbgFontExit(void)
{
	::cellDbgFontExit();
}

void CapApp::Input() 
{
	bool circlePressedNow	= mpCircle->getBoolValue();
	bool crossPressedNow	= mpCross->getBoolValue();
	bool upPressedNow		= mpUp->getBoolValue();
	bool downPressedNow		= mpDown->getBoolValue();

	static int nSelInputFrame = 0;

	// Navigation UP/DOWN with no delay
	if( !mIsUpPressed && upPressedNow)
	{
		if(gamePKG->nSelectedPKG > 0 && gamePKG->nSelectedPKG <= gamePKG->nTotalPKG) 
		{
			gamePKG->nSelectedPKG--;
		}
		nSelInputFrame = 0;
	} 
	
	if( !mIsDownPressed && downPressedNow)
	{
		if(gamePKG->nSelectedPKG >= 0 && gamePKG->nSelectedPKG < gamePKG->nTotalPKG-1) 
		{
			gamePKG->nSelectedPKG++;
		}
		nSelInputFrame = 0;
	}	
	
	// Navigation UP/DOWN with delay
	if(((mFrame + nSelInputFrame) - mFrame) == 5)
	{
		if( mIsUpPressed && upPressedNow)
		{
			if(gamePKG->nSelectedPKG > 0 && gamePKG->nSelectedPKG <= gamePKG->nTotalPKG) 
			{
				gamePKG->nSelectedPKG--;
			}			
		}
		if( mIsDownPressed && downPressedNow)
		{		
			if(gamePKG->nSelectedPKG >= 0 && gamePKG->nSelectedPKG < gamePKG->nTotalPKG-1) 
			{
				gamePKG->nSelectedPKG++;
			}
		}
		nSelInputFrame = 0;
	}
	nSelInputFrame++;

	if ( !mIsCirclePressed && circlePressedNow ) 
	{
		this->onShutdown();
		exit(0);
	}

	if ( !mIsCrossPressed && crossPressedNow ) 
	{
		if(gamePKG->pkglst[gamePKG->nSelectedPKG]->bQueued) 
		{
			// already queued...
			::cellMsgDialogOpen(CELL_MSGDIALOG_DIALOG_TYPE_NORMAL, "Sorry, you already queued this PKG.", callbackfunction, NULL, NULL);
		} else {
			gamePKG->pkglst[gamePKG->nSelectedPKG]->bQueued = true;
			gamePKG->QueuePKG();	
		}
	}

	mIsCirclePressed = circlePressedNow;
	mIsCrossPressed  = crossPressedNow;
	mIsUpPressed = upPressedNow;
	mIsDownPressed = downPressedNow;	
}

bool CapApp::onUpdate()
{
	mFrame++;

	Input();

	dbgFontPut();

	// COPY SUCCESS
	if(gamePKG->nCopyStatus == 3) 
	{
		char szMsg[256] = "";
		sprintf(szMsg, "Successfully added \"%s\" to queue.", gamePKG->szFileIn);

		::cellMsgDialogOpen2(
			CELL_MSGDIALOG_DIALOG_TYPE_NORMAL | CELL_MSGDIALOG_TYPE_BUTTON_TYPE_OK | CELL_MSGDIALOG_TYPE_DISABLE_CANCEL_ON, 
			szMsg, 
			callbackfunction, NULL, NULL);

		gamePKG->nCopyStatus = 0;
	}

	// COPY STARTED
	if(gamePKG->nCopyStatus == 1)
	{
		char szMsg[256] = "";
		sprintf(szMsg, "Processing \"%s\" [%.2f %s]...\n\nPlease wait, this could take a while depending on the size of the PKG. Do not turn off the system.", 
			gamePKG->szFileIn,
			GetByteUnit(gamePKG->pkglst[gamePKG->nSelectedPKG]->nSize), GetByteUnitStr(gamePKG->pkglst[gamePKG->nSelectedPKG]->nSize)
		);

		::cellMsgDialogOpen2(
			CELL_MSGDIALOG_BUTTON_TYPE_NONE | CELL_MSGDIALOG_TYPE_DISABLE_CANCEL_ON, // canceling not implemented yet 
			szMsg, 
			cbFileCopyDlg, NULL, NULL);

		gamePKG->nCopyStatus = 0; // avoid loop
	}

	// COPY DONE [OK]
	if(gamePKG->nCopyStatus == 2) 
	{
		cellMsgDialogClose(500.0f);
		
		sys_timer_sleep(1);

		gamePKG->nCopyStatus = 3;
	}

	// COPY ERROR MSG
	if(gamePKG->nCopyStatus == 11)
	{
		char szMsg[256] = "";
		sprintf(szMsg, "Error while processing \"%s\".", gamePKG->szFileIn);

		::cellMsgDialogOpen2(
			CELL_MSGDIALOG_DIALOG_TYPE_NORMAL | CELL_MSGDIALOG_TYPE_BUTTON_TYPE_OK | CELL_MSGDIALOG_TYPE_DISABLE_CANCEL_OFF, 
			szMsg, 
			callbackfunction, NULL, NULL);

		gamePKG->nCopyStatus = 0;
	}

	// COPY DONE [ERROR]
	if(gamePKG->nCopyStatus == 10) 
	{
		cellMsgDialogClose(500.0f);

		sys_timer_sleep(1);

		gamePKG->nCopyStatus = 11;
	}

	return FWGLApplication::onUpdate();
}

void CapApp::onRender() 
{
    // again, call the superclass method
    FWGLApplication::onRender();

	dbgFontDraw();

}

void CapApp::onShutdown() 
{
	FWInputDevice *pPad = FWInput::getDevice(FWInput::DeviceType_Pad, 0);

	if(pPad != NULL)
	{
		pPad->unbindFilter(mpCircle);
		pPad->unbindFilter(mpCross);
	}

    FWGLApplication::onShutdown();
}
