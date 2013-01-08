// ---------------------------------------------------------------------
// CaptainCPS-X's gamePKG Tool
// ---------------------------------------------------------------------
struct c_pkglist
{
	char		path[256];	// full pkg path
	char		title[256];	// pkg filename
	bool		bInternal;	// HDD / USB PKG
	bool		bQueued;	// Queued status
	uint64_t	nSize;		// size in bytes
	int			nPKGID;		// Ex. 80000000
};

#define STATUS_NORMAL			0
#define STATUS_COPY_START		1
#define STATUS_COPY_OK			2
#define STATUS_COPY_ERROR		10
#define STATUS_RM_QUEUE_START	4
#define STATUS_RM_QUEUE_OK		5

class c_gamePKG 
{
public:

	c_gamePKG();

	int		nSelectedPKG;
	int		nPKGListTop;
	int		nTotalPKG;
	bool	bDeleting;	
	int		nPKGID;

	int		nStatus;
	
	c_pkglist	*pkglst;

	int			QueuePKG();
	void		RemoveFromQueue();
	void		RemovePKG(int nId);
	int			CreatePDBFiles();
	int			DeletePDBFiles(int nId);
	int			RemoveAllDirFiles(char* szDirectory);
	int			RemovePKGDir(int nId);
	int			GetPKGDirId();
	void		RefreshPKGList();
	uint64_t	GetPKGSize(char* szFilePath);
	int			ParsePKGList(char* szDirectory);

	void		Frame();
	void		DisplayFrame();
	void		InputFrame();
	void		DlgDisplayFrame();

private:
	// ...
};

extern c_gamePKG* gamePKG;
