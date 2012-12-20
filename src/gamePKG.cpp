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
	
	C) Write structured data files to "/dev_hdd0/vsh/game_pkg/800000..":
		"d0.pdb"
		"d1.pdb"
		"f0.pdb"
		"ICON_FILE"

*/
#include "main.h"
#include "misc.h"
#include "gamePKG.h"

c_gamePKG* gamePKG;
void thread_FileCopy(uint64_t arg);

// -------------------------------------------------------
// This is template data, it should not be used as is.
// Check "int c_gamePKG::CreatePDBFiles()" for reference.
// -------------------------------------------------------
extern unsigned char d0pdb_ttxt_hdr[149];
extern unsigned char d0pdb_ttxt1[13];
extern unsigned char d0pdb_ttxt2[2];
extern unsigned char d0pdb_fn_hdr[12];
extern unsigned char d0pdb_iconpath_hdr[38];
extern unsigned char d1pdb_iconpath_hdr1[38];
extern unsigned char d1pdb_iconpath_hdr2[12];
extern unsigned char iconfile_data[106703];
// -------------------------------------------------------

void cbFileCopyDlg(int buttonType, void */*userData*/)
{

    switch( buttonType ) 
	{
		case CELL_MSGDIALOG_BUTTON_ESCAPE:
			gamePKG->bCancelCopy = true;
			break;

		default:
			break;
    }
}

void callbackfunction(int buttonType, void */*userData*/)
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
					pkglst[nTotalPKG]->nSize = this->GetPKGSize(pkglst[nTotalPKG]->path);
					
					// PKG HEX data
					sprintf(pkglst[nTotalPKG]->szSize, "%010X", (unsigned int)pkglst[nTotalPKG]->nSize);

					nTotalPKG++;
					nPKGID++;

				}
			}
		}
		cellFsClosedir(fd);

	} else {
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
		//::cellMsgDialogOpen(CELL_MSGDIALOG_DIALOG_TYPE_NORMAL, msg, callbackfunction, NULL, NULL);		
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

	ParsePKGList("/dev_usb000/pkg");
	ParsePKGList("/dev_usb000/package");

	ParsePKGList("/dev_usb001/pkg");
	ParsePKGList("/dev_usb001/package");

	ParsePKGList("/dev_usb002/pkg");
	ParsePKGList("/dev_usb002/package");

	ParsePKGList("/dev_usb003/pkg");
	ParsePKGList("/dev_usb003/package");

	ParsePKGList("/dev_usb004/pkg");
	ParsePKGList("/dev_usb004/package");

	ParsePKGList("/dev_hdd0/pkg");
	ParsePKGList("/dev_hdd0/package");
}

//int c_gamePKG::DeletePDBFiles()
//{
//	bDeleting = true;
//
//	// PKG
//	int nFNLen = strlen(pkglst[nSelectedPKG]->title) + 1; // filename length
//
//	char* pszFN = NULL;
//	pszFN = (char*)malloc(sizeof(char) * nFNLen);
//	memcpy(pszFN, pkglst[nSelectedPKG]->title, nFNLen);
//
//	char szPKGDir[256] = "";
//	sprintf(szPKGDir, "/dev_hdd0/vsh/game_pkg/%d", pkglst[nSelectedPKG]->nPKGID);
//
//	// delete files
//	char szFile[4][256];
//
//	char szPDB[4][256] = {
//		"d0.pdb", 
//		"d1.pdb", 
//		"f0.pdb", 
//		"ICON_FILE"
//	};
//
//	for(int x = 0; x < 4; x++) 
//	{
//		sprintf(szFile[x], "%s/%s", szPKGDir, szPDB[x]);
//		FILE* fp = fopen(szFile[x], "r");
//		if(fp){
//			fclose(fp);
//			fp = NULL;
//			cellFsUnlink(szFile[x]);
//		}
//	}
//
//	pkglst[nSelectedPKG]->bQueued = false;
//
//	bDeleting = false;
//
//	return 1;
//}

uint64_t c_gamePKG::GetPKGSize(char* szFilePath)
{
	CellFsStat sb;
	cellFsStat(szFilePath, &sb);

	return sb.st_size;
}

// ---------------------------------------------------------------------
/*
 Note: All the data handled here was carefully evaluated by myself
 by looking into binary data via HEX. 

 There is no official documentation of these PDB file structures.
 If for any reason some of this data is wrongly structured, the
 PKG files will not appear correctly on the XMB after they have been
 put in queue.

 * Detailed information of my findings will be documented here as 
 I get free time to work on this :)

 */
// ---------------------------------------------------------------------
int c_gamePKG::CreatePDBFiles()
{
	// PKG
	int nFNLen = strlen(pkglst[nSelectedPKG]->title) + 1; // filename length

	char* pszFN = NULL;
	pszFN = (char*)malloc(sizeof(char) * nFNLen);
	memcpy(pszFN, pkglst[nSelectedPKG]->title, nFNLen);

	// Create directory structure
	char szPKGDir[256] = "";
	sprintf(szPKGDir, "/dev_hdd0/vsh/game_pkg/%d", pkglst[nSelectedPKG]->nPKGID);

	// Create files
	FILE* fpFile[4];
	char szFile[4][256];

	char szPDB[4][256] = {
		"d0.pdb", 
		"d1.pdb", 
		"f0.pdb", 
		"ICON_FILE"
	};

	for(int x = 0; x < 4; x++) 
	{
		sprintf(szFile[x], "%s/%s", szPKGDir, szPDB[x]);
		fpFile[x] = fopen(szFile[x], "wb");
	}

	if(!fpFile[0] || !fpFile[1] || !fpFile[2] || !fpFile[3]) {		
		return 0;
	}

	// Prepare data for [d0.pdb] & [d1.pdb]
	
	unsigned char sizeData[5];
	bytes_from_hex_string(sizeData, pkglst[nSelectedPKG]->szSize);

	// PKG size 1st entry
	d0pdb_ttxt_hdr[0x70] = sizeData[0];
	d0pdb_ttxt_hdr[0x71] = sizeData[1];
	d0pdb_ttxt_hdr[0x72] = sizeData[2];
	d0pdb_ttxt_hdr[0x73] = sizeData[3];
	d0pdb_ttxt_hdr[0x74] = sizeData[4];

	// PKG size 2nd entry
	d0pdb_ttxt_hdr[0x84] = sizeData[0];
	d0pdb_ttxt_hdr[0x85] = sizeData[1];
	d0pdb_ttxt_hdr[0x86] = sizeData[2];
	d0pdb_ttxt_hdr[0x87] = sizeData[3];
	d0pdb_ttxt_hdr[0x88] = sizeData[4];

	// TITLE string length entry
	d0pdb_ttxt_hdr[0x94]	= nFNLen + 0x0D + 1;
	d0pdb_ttxt_hdr[0x94-4]	= nFNLen + 0x0D + 1;
	
	// Filename string length entry
	d0pdb_fn_hdr[0x0B]		= nFNLen;
	d0pdb_fn_hdr[0x0B-4]		= nFNLen;
	
	// [d0.pdb] & [d1.pdb] TITLE header
	char *title_hdr = NULL;
	title_hdr = (char*)malloc(sizeof(char) * 0x95);
	memcpy(title_hdr, d0pdb_ttxt_hdr, 0x95);

	// [d0.pdb] & [d1.pdb] TITLE starting text
	char *title_txt1 = NULL;
	title_txt1 = (char*)malloc(sizeof(char) * 0x0D);
	memcpy(title_txt1, d0pdb_ttxt1, 0x0D);

	// [d0.pdb] & [d1.pdb] TITLE ending text
	char *title_txt2 = NULL;
	title_txt2 = (char*)malloc(sizeof(char) * 0x02);
	memcpy(title_txt2, d0pdb_ttxt2, 0x02);

	// [d0.pdb] & [d1.pdb] FILENAME header
	char *filename_hdr = NULL;
	filename_hdr = (char*)malloc(sizeof(char) * 0x0C);
	memcpy(filename_hdr, d0pdb_fn_hdr, 0x0C);

	// [d0.pdb] ICON_FILE path d0.pdb header
	char *iconpath_hdr = NULL;
	iconpath_hdr = (char*)malloc(sizeof(char) * 0x26);
	memcpy(iconpath_hdr, d0pdb_iconpath_hdr, 0x26);

	// [d1.pdb][1st] ICON_FILE path header
	char *iconpath_d1_hdr = NULL;
	iconpath_d1_hdr = (char*)malloc(sizeof(char) * 0x26);
	memcpy(iconpath_d1_hdr, d1pdb_iconpath_hdr1, 0x26);

	// [d1.pdb][2nd] ICON_FILE path header
	char *iconpath_d1_hdr2 = NULL;
	iconpath_d1_hdr2 = (char*)malloc(sizeof(char) * 0x0C);
	memcpy(iconpath_d1_hdr2, d1pdb_iconpath_hdr2, 0x0C);

	// [d0.pdb] & [d1.pdb] ICON_FILE path string
	char szIconPath[0x2A] = "";
	sprintf(szIconPath, "/dev_hdd0/vsh/game_pkg/%d/ICON_FILE\0", pkglst[nSelectedPKG]->nPKGID);
	szIconPath[0x2A] = 0;

	// =========================================================
	// WRITE --> d0.pdb
	// =========================================================

	// write header + length of TITLE
	fwrite(title_hdr, sizeof(char), 0x95, fpFile[0]);				

	// write TITLE + NULL char
	fwrite(title_txt1, sizeof(char), 0x0D, fpFile[0]);
	fwrite(pkglst[nSelectedPKG]->title, sizeof(char), nFNLen-1, fpFile[0]);
	fwrite(title_txt2, sizeof(char), 0x02, fpFile[0]);

	// write header + length of FILENAME
	fwrite(filename_hdr, sizeof(char), 0x0C, fpFile[0]);

	// write FILENAME + NULL char
	fwrite(pkglst[nSelectedPKG]->title, sizeof(char), nFNLen, fpFile[0]);

	// write header + length of ICON path
	fwrite(iconpath_hdr, sizeof(char), 0x26, fpFile[0]);

	// write ICON path + NULL char
	fwrite(szIconPath, sizeof(char), 0x2A, fpFile[0]);

	// =========================================================
	// WRITE --> d1.pdb
	// =========================================================

	// write TITLE segment header + length of string
	fwrite(title_hdr, sizeof(char), 0x95, fpFile[1]);				

	// write TITLE segment
	fwrite(title_txt1, sizeof(char), 0x0D, fpFile[1]);
	fwrite(pkglst[nSelectedPKG]->title, sizeof(char), nFNLen-1, fpFile[1]);
	fwrite(title_txt2, sizeof(char), 0x02, fpFile[1]);

	// write FILENAME segment header + length of string
	fwrite(filename_hdr, sizeof(char), 0x0C, fpFile[1]);

	// write FILENAME + NULL char
	fwrite(pkglst[nSelectedPKG]->title, sizeof(char), nFNLen, fpFile[1]);

	// [1st] write ICON_FILE path segment header + length of string
	fwrite(iconpath_d1_hdr, sizeof(char), 0x26, fpFile[1]);

	// [1st] write ICON_FILE path + NULL char
	fwrite(szIconPath, sizeof(char), 0x2A, fpFile[1]);

	// [2nd] write ICON_FILE path segment header + length of string
	fwrite(iconpath_d1_hdr2, sizeof(char), 0x0C, fpFile[1]);

	// [2nd] write ICON path + NULL char
	fwrite(szIconPath, sizeof(char), 0x2A, fpFile[1]);

	// =========================================================
	// WRITE --> ICON_FILE
	// =========================================================

	fwrite(iconfile_data, sizeof(char), 106703, fpFile[3]);

	// cleanup
	fclose(fpFile[0]);
	fclose(fpFile[1]);
	fclose(fpFile[2]);
	fclose(fpFile[3]);

	return 1;
}

void thread_FileCopy(uint64_t arg)
{	
	gamePKG->nCopyStatus = 1;

	int running = 0;
	sys_ppu_thread_t id;
	sys_ppu_thread_get_id(&id);	
	
	sys_ppu_thread_stack_t stackinfo;
	sys_ppu_thread_get_stack_information(&stackinfo);

	FILE *filer, *filew;
	int numr, numw;
	char buffer[2352]; // 1 sector as in CD-ROM

	filer = fopen(gamePKG->szFileIn,"rb");
	filew = fopen(gamePKG->szFileOut,"wb");

	if(filer != NULL && filew != NULL)
	{
		while(feof(filer) == 0)
		{
			if(gamePKG->bCancelCopy) 
			{
				//fclose(filer);
				//fclose(filew);
				//cellFsUnlink(szFileOut); // delete file
				//DeletePDBFiles();
				//while(bDeleting)
				//{
				//	// ...wait
				//}
				//bCancelCopy = false;
				//nCopyStatus = 10;
				//sys_ppu_thread_exit(0);
				//break;
			}

			sys_ppu_thread_yield();		
			running++;

			if((numr = fread(buffer, 1, 2352, filer)) != 2352) {
				if(ferror(filer) != 0) 
				{
					fclose(filer);
					fclose(filew);
					cellFsUnlink(gamePKG->szFileOut); // delete file
					break;
				}
				else if(feof(filer) != 0);
			}
			if((numw = fwrite(buffer, 1, numr, filew)) != numr) 
			{
				fclose(filer);
				fclose(filew);
				cellFsUnlink(gamePKG->szFileOut); // delete file
				break;
			}
		}
		fclose(filer);
		fclose(filew);
		
		gamePKG->nCopyStatus = 2;

	} else {

		// TODO: Add cleanup here
		// delete created files and directory
		gamePKG->nCopyStatus = 10;

		sys_ppu_thread_exit(0);
	}

	gamePKG->nCopyStatus = 2;
	sys_ppu_thread_exit(0);
}

int c_gamePKG::QueuePKG()
{
	char szPKGDir[256] = "";
	sprintf(szPKGDir, "/dev_hdd0/vsh/game_pkg/%d", pkglst[nSelectedPKG]->nPKGID);

	cellFsMkdir(szPKGDir, CELL_FS_DEFAULT_CREATE_MODE_1); // perms 700

	if(!CreatePDBFiles())
	{
		::cellMsgDialogOpen(
			CELL_MSGDIALOG_DIALOG_TYPE_NORMAL, 
			"[Error] Could not create / write needed files to HDD.", 
			callbackfunction, NULL, NULL
		);
		return 0;
	}

	sprintf(szFileIn, "%s", pkglst[nSelectedPKG]->path); 
	sprintf(szFileOut, "%s/%s", szPKGDir, pkglst[nSelectedPKG]->title);	

	sys_ppu_thread_t	id;
	uint64_t			thread_arg		= 0x1337;
	uint64_t			priority		= 1500;
	size_t				stack_size		= 0x1000;
	char				thread_name[]	= "File Copy";
	
	int s;
	s = sys_ppu_thread_create(&id, thread_FileCopy, thread_arg, priority, stack_size, SYS_PPU_THREAD_CREATE_JOINABLE, thread_name);

	return 1;
}