// ---------------------------------------------------------------------
// CaptainCPS-X's gamePKG Tool
// ---------------------------------------------------------------------
class c_pkglist
{
public:

	char path[1024];	// full pkg path
	char title[64];		// pkg filename

	bool bInternal;		// HDD / USB PKG
	bool bQueued;		// Queued status

	uint64_t nSize;		// size in bytes
	char szSize[10];	// size hex data

	int nPKGID;

	c_pkglist() 
	{
		bInternal	= false;
		bQueued		= false;
		nSize		= 0;

		nPKGID		= 80000000; // default
	}

	~c_pkglist() { }
};

class c_gamePKG 
{
public:

	int		nSelectedPKG;
	int		nPKGListTop;
	char	szFileIn[256];
	char	szFileOut[256];
	bool	bCancelCopy;
	int		nTotalPKG;
	bool	bDeleting;
	int		nCopyStatus;
	int		nPKGID;

	c_pkglist* pkglst[9000];

	c_gamePKG() 
	{
		nSelectedPKG	= 0;
		nPKGListTop		= 0;
		bCancelCopy		= false;
		nTotalPKG		= 0;
		bDeleting		= false;
		nCopyStatus		= 0;
		nPKGID			= 80000000;
	}

	~c_gamePKG() {
		// ...
	}

	int QueuePKG();
	int CreatePDBFiles();
	int DeletePDBFiles(int nId);
	int RemovePKGDir(int nId);
	void RefreshPKGList();
	uint64_t GetPKGSize(char* szFilePath);
	int ParsePKGList(const char* szDevice);

private:
	// ...
};

extern c_gamePKG* gamePKG;
