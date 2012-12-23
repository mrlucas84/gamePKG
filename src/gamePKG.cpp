// ---------------------------------------------------------------------
// CaptainCPS-X's gamePKG Tool
// ---------------------------------------------------------------------
/*

 This application work as follows:

 1 - Scan hdd / usb directories for PKG files
 2 - Store each found entry info to "gamePKG->pkglst[x]"
 3 - When (X) is pressed:

	A) Create directory at "/dev_hdd0/vsh/game_pkg/" named as "gamePKG->pkglst[gamePKG->nSelectedPKG]->nPKGID"
		
		Ex. "/dev_hdd0/vsh/game_pkg/80000000"

	B) Copy PKG file "gamePKG->pkglst[gamePKG->nSelectedPKG]->path" to "/dev_hdd0/vsh/game_pkg/800000.."
	
	C) Write structured data file to "/dev_hdd0/vsh/game_pkg/800000..":
		"d0.pdb"
		"ICON_FILE"

*/
#include "main.h"
#include "misc.h"
#include "gamePKG.h"

c_gamePKG* gamePKG;
void thread_FileCopy(uint64_t arg);

extern unsigned char iconfile_data[0x1A0CF];

// -------------------------------------------------------

void DlgCallbackFunction(int buttonType, void */*userData*/)
{
    switch( buttonType ) 
	{
		case CELL_MSGDIALOG_BUTTON_YES:
		case CELL_MSGDIALOG_BUTTON_NO:		
		case CELL_MSGDIALOG_BUTTON_ESCAPE:
		case CELL_MSGDIALOG_BUTTON_NONE:
			break;

		default:
			break;
    }
}

// Initialize gamePKG module
c_gamePKG::c_gamePKG()
{
	nSelectedPKG	= 0;
	nPKGListTop		= 0;
	nTotalPKG		= 0;
	bDeleting		= false;
	nCopyStatus		= 0;
	nPKGID			= 80000000;
}

c_gamePKG::~c_gamePKG()
{
	// ...
	if(nTotalPKG > 0)
	{
		for(int nPKG = 0; nPKG < nTotalPKG; nPKG++)
		{
			if(pkglst[nPKG]) 
			{
				delete [] pkglst[nPKG];
				pkglst[nPKG] = NULL;
			}
		}
		nTotalPKG = 0;
	}
	nSelectedPKG	= 0;
	nPKGListTop		= 0;
	bDeleting		= false;
	nCopyStatus		= 0;
	nPKGID			= 80000000;
}

void c_gamePKG::Frame()
{
	DisplayFrame();

	InputFrame();

	DlgDisplayFrame();
}

void c_gamePKG::DisplayFrame()
{
	float xPos		= 0.05f;
	float yPos		= 0.05f;
	float yPosDiff	= 0.03f;	
	float nFontSize = 0.6f;

	::cellDbgFontPuts(xPos, yPos, nFontSize, 0xffffffff, "----------------------------------------------------------------------" );
	yPos += yPosDiff;	
	::cellDbgFontPuts(xPos, yPos, nFontSize, 0xffffffff, " gamePKG Tool v1.1 - by CaptainCPS-X [2012]");
	yPos += yPosDiff;
	::cellDbgFontPuts(xPos, yPos, nFontSize, 0xffffffff, "----------------------------------------------------------------------" );
	yPos += yPosDiff;
	::cellDbgFontPuts(xPos, yPos, nFontSize, 0xffffffff, "PRESS -(O)- TO EXIT AND RETURN TO XMB");
	yPos += yPosDiff;
	::cellDbgFontPuts(xPos, yPos, nFontSize, 0xffffffff, "PRESS -(X)- TO QUEUE PKG ON XMB");
	yPos += yPosDiff;
	//::cellDbgFontPuts(xPos, yPos, nFontSize, 0xffffffff, "PRESS -(/\)- TO VIEW FILE INFO");
	//yPos += yPosDiff;
	::cellDbgFontPuts(xPos, yPos, nFontSize, 0xffffffff, "PRESS -([ ])- TO REMOVE FROM QUEUE");
	yPos += yPosDiff;
	//::cellDbgFontPuts(xPos, yPos, nFontSize, 0xffffffff, "PRESS -[SELECT]- TO WIPE ALL QUEUES & REFRESH DEVICES");
	//yPos += yPosDiff;
	::cellDbgFontPuts(xPos, yPos, nFontSize, 0xffffffff, "----------------------------------------------------------------------" );
	yPos += yPosDiff;

	// Main menu
	uint32_t block_size			= 0;
	uint64_t free_block_count	= 0;
	
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
		"Total PKG found on all scanned devices: %d", nTotalPKG);

	yPos += yPosDiff;

	::cellDbgFontPuts(xPos, yPos, nFontSize, 0xffffffff, "----------------------------------------------------------------------" );
	yPos += yPosDiff;

	int nPKGListMax = 14;

	if(nSelectedPKG > nPKGListMax || nPKGListTop > 0)
	{
		if(nPKGListTop < (nSelectedPKG - nPKGListMax))
		{
			nPKGListTop = nSelectedPKG - nPKGListMax;
		}

		if(nPKGListTop > 0) 
		{
			if(nSelectedPKG < nPKGListTop)
			{
				nPKGListTop = nSelectedPKG;
			}
		} else {
			nPKGListTop = nSelectedPKG - nPKGListMax;
		}
	} else {
		nPKGListTop = nSelectedPKG - nPKGListMax;
	}

	if(nPKGListTop < 0) nPKGListTop = 0;

	int nPKG = nPKGListTop;
	
	while(nPKG <= (nPKGListTop + nPKGListMax))
	{
		if(nPKG == nTotalPKG) break;

		uint32_t nColor	= 0xffffffff;		// white
		nFontSize		= 0.6f;

		// PKG QUEUED
		if(pkglst[nPKG]->bQueued) 
		{
			nColor		= 0xff00ff00;		// green
		}

		// PKG SELECTED
		if(nPKG == nSelectedPKG) 
		{
			nColor		= 0xff00ccff;		// yellow
			nFontSize	= 0.8f;

			if(pkglst[nPKG]->bQueued) 
			{
				nColor	= 0xff0000ff;		// red
			}
		}

		::cellDbgFontPrintf(xPos, yPos, nFontSize, nColor, "[%d] %s [%.2f %s] %s %s", 
			nPKG+1, 
			pkglst[nPKG]->title,
			GetByteUnit(pkglst[nPKG]->nSize), GetByteUnitStr(pkglst[nPKG]->nSize),
			pkglst[nPKG]->bInternal ? "[HDD]" : "[USB]",
			pkglst[nPKG]->bQueued ? "[Queued]" : " "
		);

		yPos += yPosDiff;

		nPKG++;
	}

	nFontSize = 0.6f;

	::cellDbgFontPuts(xPos, yPos, nFontSize, 0xffffffff, "----------------------------------------------------------------------" );
	yPos += yPosDiff;
}

void c_gamePKG::InputFrame()
{
	static int nSelInputFrame = 0;

	// Navigation UP/DOWN with no delay
	if( !app.mIsUpPressed && app.upPressedNow)
	{
		if(nSelectedPKG > 0 && nSelectedPKG <= nTotalPKG) 
		{
			nSelectedPKG--;
		}
		nSelInputFrame = 0;
	} 
	
	if( !app.mIsDownPressed && app.downPressedNow)
	{
		if(nSelectedPKG >= 0 && nSelectedPKG < nTotalPKG-1)
		{
			nSelectedPKG++;
		}
		nSelInputFrame = 0;
	}	
	
	// Navigation UP/DOWN with delay
	if(((app.mFrame + nSelInputFrame) - app.mFrame) == 5)
	{
		if( app.mIsUpPressed && app.upPressedNow)
		{
			if(nSelectedPKG > 0 && nSelectedPKG <= nTotalPKG) 
			{
				nSelectedPKG--;
			}			
		}
		if( app.mIsDownPressed && app.downPressedNow)
		{		
			if(nSelectedPKG >= 0 && nSelectedPKG < nTotalPKG-1) 
			{
				nSelectedPKG++;
			}
		}
		nSelInputFrame = 0;
	}
	nSelInputFrame++;

	if ( !app.mIsCirclePressed && app.circlePressedNow ) 
	{
		app.onShutdown();
		exit(0);
	}

	if ( !app.mIsCrossPressed && app.crossPressedNow ) 
	{
		if(pkglst[nSelectedPKG]->bQueued) 
		{
			// already queued...
			::cellMsgDialogOpen2(
				CELL_MSGDIALOG_DIALOG_TYPE_NORMAL, 
				"Sorry, this PKG is already queued.",
				DlgCallbackFunction, NULL, NULL
			);
		} else {
			QueuePKG();
		}
	}
}

void c_gamePKG::DlgDisplayFrame()
{
	// COPY STARTED
	if(nCopyStatus == 1)
	{
		char szMsg[256] = "";
		sprintf(
			szMsg, 
			"Processing \"%s\" [%.2f %s]...\n\nPlease wait, this could take a while depending on the size of the PKG. Do not turn off the system.", 
			szFileIn,
			GetByteUnit(pkglst[nSelectedPKG]->nSize), GetByteUnitStr(pkglst[nSelectedPKG]->nSize)
		);

		::cellMsgDialogOpen2(
			CELL_MSGDIALOG_BUTTON_TYPE_NONE | CELL_MSGDIALOG_TYPE_DISABLE_CANCEL_ON,
			szMsg,
			DlgCallbackFunction, NULL, NULL
		);

		nCopyStatus = 0; // avoid loop
	}

	// COPY [OK]
	if(nCopyStatus == 2) 
	{
		cellMsgDialogAbort();

		char szMsg[256] = "";
		sprintf(szMsg, "Successfully added \"%s\" to queue.", szFileIn);

		::cellMsgDialogOpen2(
			CELL_MSGDIALOG_DIALOG_TYPE_NORMAL | CELL_MSGDIALOG_TYPE_BUTTON_TYPE_OK | CELL_MSGDIALOG_TYPE_DISABLE_CANCEL_ON, 
			szMsg, 
			DlgCallbackFunction, NULL, NULL
		);

		pkglst[nSelectedPKG]->bQueued = true;
		nCopyStatus = 0;
	}

	// COPY [ERROR]
	if(nCopyStatus == 10) 
	{
		cellMsgDialogAbort();

		char szMsg[256] = "";
		sprintf(szMsg, "Error while processing \"%s\".", szFileIn);

		::cellMsgDialogOpen2(
			CELL_MSGDIALOG_DIALOG_TYPE_NORMAL | CELL_MSGDIALOG_TYPE_BUTTON_TYPE_OK | CELL_MSGDIALOG_TYPE_DISABLE_CANCEL_OFF, 
			szMsg, 
			DlgCallbackFunction, NULL, NULL
		);

		pkglst[nSelectedPKG]->bQueued = false;

		DeletePDBFiles(pkglst[nSelectedPKG]->nPKGID);
		RemovePKGDir(pkglst[nSelectedPKG]->nPKGID);

		nCopyStatus = 0;
	}
}

int c_gamePKG::ParsePKGList(const char* szDevice)
{
	int fd;
	CellFsDirent dir;
	
	char szDirectory[256] = "";
	sprintf(szDirectory, "%s", szDevice);

	CellFsErrno res = cellFsOpendir(szDirectory, &fd);
	
	if (res == CELL_FS_SUCCEEDED) 
	{				
		uint64_t nread;

		while (::cellFsReaddir(fd, &dir, &nread) == CELL_FS_SUCCEEDED) 
		{
			if(nread == 0) break;

			if (dir.d_type == CELL_FS_TYPE_DIRECTORY)
			{
				// DIRECTORY
			} else {
				
				if(dir.d_type != CELL_FS_TYPE_REGULAR) break;
				
				// FILE

				int namelen = strlen(dir.d_name);
				if (namelen < 5) continue;
				
				// PKG
				int nFNLen = strlen(dir.d_name) + 1; // filename length

				char* pszFN = NULL;
				pszFN = (char*)malloc(sizeof(char) * nFNLen);
				memcpy(pszFN, dir.d_name, nFNLen);

				if(strstr(toLowerCase(pszFN), ".pkg")) 
				{

					// Add entry values to game list
					pkglst[nTotalPKG] = new c_pkglist();
				
					if(!strncmp(szDevice, "/dev_hdd0", strlen("/dev_hdd0")))
					{
						pkglst[nTotalPKG]->bInternal = true;
					}

					// PKG Path
					sprintf(pkglst[nTotalPKG]->path, "%s/%s", szDevice, dir.d_name);				
					
					// PKG Filename
					sprintf(pkglst[nTotalPKG]->title, "%s", dir.d_name);
					pkglst[nTotalPKG]->title[63] = 0;

					// PKG directory ID (ex. 80000000)
					pkglst[nTotalPKG]->nPKGID = nPKGID;

					// PKG Size in bytes
					pkglst[nTotalPKG]->nSize = GetPKGSize(pkglst[nTotalPKG]->path);
					
					nTotalPKG++;
					nPKGID++;

				}
			}
		}
		cellFsClosedir(fd);

	} else {
		
		// DEBUG
/*
		const char *msg = NULL;
		switch(res)
		{
			case CELL_FS_ENOTMOUNTED:	// File system corresponding to path is not mounted
				msg = "CELL_FS_ENOTMOUNTED";
				break;
			case CELL_FS_ENOENT:		// The specified directory does not exist
				msg = "CELL_FS_ENOENT";
				break;
			case CELL_FS_ENOTDIR:		// The specified path is not a directory, Components in path contain something other than a directory
				msg = "CELL_FS_ENOTDIR";
				break;
			case CELL_FS_EMFILE:		// The number of file descriptors that can be opened at the same time has reached the maximum
				msg = "CELL_FS_EMFILE";
				break;
			case CELL_FS_EIO:			// I/O error has occurred
				msg = "CELL_FS_EIO";
				break;
			case CELL_FS_ENOMEM:		// Memory is insufficient, File specified by path does not exist
				msg = "CELL_FS_ENOMEM";
				break;
			case CELL_FS_ENAMETOOLONG:	// path or components in path exceed the maximum length
				msg = "CELL_FS_EFAULT";
				break;
			case CELL_FS_EFSSPECIFIC:	// File system specific internal error has occurred
				msg = "CELL_FS_EFSSPECIFIC";
				break;
			case CELL_FS_EPERM:			// Permission is invalid
				msg = "CELL_FS_EPERM";
				break;
			case CELL_FS_EFAULT:		// path or fd is NULL
				msg = "CELL_FS_EFAULT";
				break;
		}		
		::cellMsgDialogOpen2(CELL_MSGDIALOG_DIALOG_TYPE_NORMAL, msg, DlgCallbackFunction, NULL, NULL);
*/
		return 0;
	}

	return 1;
}

void c_gamePKG::RefreshPKGList()
{
	if(nTotalPKG > 0)
	{
		for(int nPKG = 0; nPKG < nTotalPKG; nPKG++)
		{
			if(pkglst[nPKG]) 
			{
				delete [] pkglst[nPKG];
				pkglst[nPKG] = NULL;
			}
		}
		nTotalPKG = 0;
	}

	ParsePKGList("/dev_hdd0/pkg");
	ParsePKGList("/dev_hdd0/package");

	ParsePKGList("/dev_usb000");
	ParsePKGList("/dev_usb000/pkg");
	ParsePKGList("/dev_usb000/package");

	ParsePKGList("/dev_usb001");
	ParsePKGList("/dev_usb001/pkg");
	ParsePKGList("/dev_usb001/package");

	ParsePKGList("/dev_usb002");
	ParsePKGList("/dev_usb002/pkg");
	ParsePKGList("/dev_usb002/package");

	ParsePKGList("/dev_usb003");
	ParsePKGList("/dev_usb003/pkg");
	ParsePKGList("/dev_usb003/package");

	ParsePKGList("/dev_usb004");
	ParsePKGList("/dev_usb004/pkg");
	ParsePKGList("/dev_usb004/package");

	ParsePKGList("/dev_sd");
	ParsePKGList("/dev_sd/pkg");
	ParsePKGList("/dev_sd/package");

	// todo: add micro sd support

}

int c_gamePKG::RemovePKGDir(int nId)
{
	char szPKGDir[256] = "";
	sprintf(szPKGDir, "/dev_hdd0/vsh/game_pkg/%d", nId);
	
	CellFsErrno ret = cellFsRmdir(szPKGDir);
	
	if(ret != CELL_FS_SUCCEEDED) 
	{
		// couldn't delete the directory...
		return 0;
	}

	return 1;
}

int c_gamePKG::DeletePDBFiles(int nId)
{
	bDeleting = true;

	char szPDB[256] = "";
	char szIconFile[256] = "";
	sprintf(szPDB, "/dev_hdd0/vsh/game_pkg/%d/d0.pdb", nId);
	sprintf(szIconFile, "/dev_hdd0/vsh/game_pkg/%d/ICON_FILE", nId);

	FILE *fpPDB, *fpIcon;
	
	if((fpPDB = fopen(szPDB, "r")))
	{
		fclose(fpPDB);
		fpPDB = NULL;
		cellFsUnlink(szPDB);
	}

	if((fpIcon = fopen(szIconFile, "r")))
	{
		fclose(fpIcon);
		fpIcon = NULL;
		cellFsUnlink(szIconFile);
	}

	bDeleting = false;
	
	return 1;
}

uint64_t c_gamePKG::GetPKGSize(char* szFilePath)
{
	CellFsStat sb;
	memset(&sb, 0, sizeof(CellFsStat));

	cellFsStat(szFilePath, &sb);
	
	return sb.st_size;
}

/* ---------------------------------------------------------------------

UPDATE [2012-12-21]:
	
- Rewrote whole function [Thanks to Dean & Deroad for the PDB structure info]

 --------------------------------------------------------------------- */
int c_gamePKG::CreatePDBFiles()
{
	// Create files	
	char szPDBFile[256] = "";
	char szIconFile[256] = "";
	sprintf(szPDBFile, "/dev_hdd0/vsh/game_pkg/%d/d0.pdb", pkglst[nSelectedPKG]->nPKGID);
	sprintf(szIconFile, "/dev_hdd0/vsh/game_pkg/%d/ICON_FILE", pkglst[nSelectedPKG]->nPKGID);
	
	FILE *fpPDB, *fpIcon;
	fpPDB	= fopen(szPDBFile, "wb");
	fpIcon	= fopen(szIconFile, "wb");

	if(!fpPDB || !fpIcon) 
	{
		// failed to create files
		return 0;
	}

	// ------------------------------------------------------------------------
	// write - d0.pdb
	//

	// 00000000 - Header
	char header[4] = {
		0x00, 0x00, 0x00, 0x00
	};
	fwrite(header, 4, 1, fpPDB);

	// 00000065 - Unknown
	char unknown1[16] = {
		0x00, 0x00, 0x00, 0x65,
		0x00, 0x00, 0x00, 0x04,
		0x00, 0x00, 0x00, 0x04,
		0x00, 0x00, 0x00, 0x00
	};
	fwrite(unknown1, 16, 1, fpPDB);

	// 0000006B - Unknown
	char unknown2[16] = {
		0x00, 0x00, 0x00, 0x6B,
		0x00, 0x00, 0x00, 0x04,
		0x00, 0x00, 0x00, 0x04,
		0x00, 0x00, 0x00, 0x03
	};
	fwrite(unknown2, 16, 1, fpPDB);

	// 00000068 - Status of download
	char dl_status[16] = {
		0x00, 0x00, 0x00, 0x68,
		0x00, 0x00, 0x00, 0x04,
		0x00, 0x00, 0x00, 0x04,
		0x00, 0x00, 0x00, 0x00
	};
	fwrite(dl_status, 16, 1, fpPDB);

	// 000000D0 - Download current size (in bytes)
	char dl_progress[12] = {
		0x00, 0x00, 0x00, 0xD0,
		0x00, 0x00, 0x00, 0x08,
		0x00, 0x00, 0x00, 0x08
	};
	fwrite(dl_progress, 12, 1, fpPDB);
	fwrite((char*) &pkglst[nSelectedPKG]->nSize, 8, 1, fpPDB);

	// 000000CE - Download expected size (in bytes)
	char dl_size[12] = {
		0x00, 0x00, 0x00, 0xCE,
		0x00, 0x00, 0x00, 0x08,
		0x00, 0x00, 0x00, 0x08
	};
	fwrite(dl_size, 12, 1, fpPDB);
	fwrite((char*) &pkglst[nSelectedPKG]->nSize, 8, 1, fpPDB);

	// 00000069 - Display title
	char title[4] = {
		0x00, 0x00, 0x00, 0x69
	};
	fwrite(title, 4, 1, fpPDB);
	
	char title_str[256] = "";
	sprintf(title_str, "\xE2\x98\x85 Install \x22%s\x22", pkglst[nSelectedPKG]->title);
	
	unsigned int title_len = strlen(title_str) + 1;
	fwrite((char*) &title_len, 4, 1, fpPDB);
	fwrite((char*) &title_len, 4, 1, fpPDB);
	fwrite(title_str, title_len, 1, fpPDB);

	// 000000CB - PKG file name
	char filename[4] = {
		0x00, 0x00, 0x00, 0xCB
	};
	fwrite(filename, 4, 1, fpPDB);
	
	unsigned int filename_len = strlen(pkglst[nSelectedPKG]->title) + 1;
	fwrite((char*) &filename_len, 4, 1, fpPDB);
	fwrite((char*) &filename_len, 4, 1, fpPDB);
	fwrite(pkglst[nSelectedPKG]->title, filename_len, 1, fpPDB);

	// 0000006A - Icon location / path (PNG w/o extension) 
	char iconpath[4] ={
		0x00, 0x00, 0x00, 0x6A
	};
	fwrite(iconpath, 4, 1, fpPDB);

	unsigned int iconpath_len = strlen(szIconFile) + 1;
	fwrite((char*) &iconpath_len, 4, 1, fpPDB);
	fwrite((char*) &iconpath_len, 4, 1, fpPDB);
	fwrite(szIconFile, iconpath_len, 1, fpPDB);

	fclose(fpPDB);

	// ------------------------------------------------------------------------
	// write - ICON_FILE
	//

	fwrite(iconfile_data, 0x1A0CF, 1, fpIcon);

	fclose(fpIcon);

	return 1;
}

#define BUFF_SIZE	0x300000 // 3MB

void thread_FileCopy(uint64_t /*arg*/)
{	
	gamePKG->nCopyStatus = 1;

	FILE *filer, *filew;
	int numr, numw;	
	char *buffer;

	buffer = (char*)malloc(sizeof(char) * BUFF_SIZE);
	memset(buffer, 0, sizeof(char) * BUFF_SIZE);

	filer = fopen(gamePKG->szFileIn,"rb");
	filew = fopen(gamePKG->szFileOut,"wb");

	if(filer != NULL && filew != NULL)
	{
		bool bCopyError = false;

		while(feof(filer) == 0)
		{
			if((numr = fread(buffer, 1, BUFF_SIZE, filer)) != BUFF_SIZE) 
			{
				if(ferror(filer) != 0) 
				{
					bCopyError = true;
					break;
				}
				else if(feof(filer) != 0)
				{
					// ...
				}
			}
			if((numw = fwrite(buffer, 1, numr, filew)) != numr) 
			{
				bCopyError = true;
				break;
			}
		}

		if(filer) fclose(filer);
		if(filew) fclose(filew);
		
		if(bCopyError)
		{
			cellFsUnlink(gamePKG->szFileOut); // delete file

			gamePKG->nCopyStatus = 10;
			sys_ppu_thread_exit(0);
			return;
		}

	} else {

		if(buffer) free(buffer);
		buffer = NULL;

		if(filer) fclose(filer);
		if(filew) 
		{
			fclose(filew);		
			cellFsUnlink(gamePKG->szFileOut); // delete file
		}

		gamePKG->nCopyStatus = 10;
		sys_ppu_thread_exit(0);
		return;
	}
	
	if(buffer) free(buffer);
	buffer = NULL;

	gamePKG->nCopyStatus = 2;
	sys_ppu_thread_exit(0);
}

int c_gamePKG::QueuePKG()
{
	char pszPKGDir[256] ="";
	sprintf(pszPKGDir, "/dev_hdd0/vsh/game_pkg/%d", pkglst[nSelectedPKG]->nPKGID);

	CellFsErrno ret;
	ret = cellFsMkdir("/dev_hdd0/vsh/game_pkg", CELL_FS_S_IFDIR | 0777);
	ret = cellFsMkdir(pszPKGDir, CELL_FS_S_IFDIR | 0777);

	if(ret != CELL_FS_SUCCEEDED)
	{
		::cellMsgDialogOpen2(
			CELL_MSGDIALOG_DIALOG_TYPE_NORMAL, 
			"[Error] Could not create the required directory on HDD.", 
			DlgCallbackFunction, NULL, NULL
		);
		pkglst[nSelectedPKG]->bQueued = false;
		return 0;
	}

	if(!CreatePDBFiles())
	{
		::cellMsgDialogOpen2(
			CELL_MSGDIALOG_DIALOG_TYPE_NORMAL, 
			"[Error] Could not create / write needed files to HDD.", 
			DlgCallbackFunction, NULL, NULL
		);
		pkglst[nSelectedPKG]->bQueued = false;
		return 0;
	}

	sprintf(szFileIn, "%s", pkglst[nSelectedPKG]->path); 
	sprintf(szFileOut, "%s/%s", pszPKGDir, pkglst[nSelectedPKG]->title);	

	sys_ppu_thread_t thread_id;

	sys_ppu_thread_create(
		&thread_id,
		thread_FileCopy,				// callback function
		0x1337,							// arg
		1500,							// priority
		0x1000,							// stack size
		SYS_PPU_THREAD_CREATE_JOINABLE, //
		"File Copy"						// name
	);
	
	return 1;
}