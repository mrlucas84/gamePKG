// ---------------------------------------------------------------------
// CaptainCPS-X's gamePKG Tool
// ---------------------------------------------------------------------
#ifndef MAIN_H
#define MAIN_H

#include <cell/dbgfont.h>
#include <cell/cell_fs.h>
#include <sys/paths.h>
#include <sys/process.h>
#include <sys/ppu_thread.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sysutil/sysutil_msgdialog.h>
#include <cell/sysmodule.h>
#include <sys/memory.h>

#include "psgl/FWGLApplication.h"
#include "psgl/FWGLGrid.h"
#include "FWInput.h"
#include "FWDebugFont.h"

class CapApp : public FWGLApplication
{
    public:
        CapApp();

        // sample framework overrides
        virtual bool onInit(int argc, char **argv);
        virtual void onRender();
        virtual void onShutdown();
		virtual bool onUpdate();

		virtual void Input();

		virtual void dbgFontInit(void);
		virtual void dbgFontPut(void);
		virtual void dbgFontDraw(void);
		virtual void dbgFontExit(void);

    protected:
    private:
		FWInputFilter	*mpEnterKey, *mpSpaceKey;
		FWInputFilter   *mpCircle, *mpCross, *mpUp, *mpDown;

		CellDbgFontConsoleId mDbgFontID[2];
		unsigned int	mFrame;
		bool            mIsCirclePressed, mIsCrossPressed;
		bool			mIsUpPressed, mIsDownPressed;
};

extern CapApp app;

#endif // MAIN_H
