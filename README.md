gamePKG [by CaptainCPS-X, 2012]
----------------------------------------------

Information:
----------------------------------------------

gamePKG is an application for the PS3 that allows
the user to queue PKG files for later installation
from the XMB "without restarting the PS3 at all".

Main objective of this application is to help
users on Rogero CFW 4.30 (v2.03) to install
PKGs without wasting much time.

Technical Info:
----------------------------------------------

Rogero CFW 4.30 (v2.03) @ 1080p resolution [HDMI]

Should work on 3.40+ CFW

The application will scan the following directories
for PKG files:

- /dev_hdd0/pkg/
- /dev_hdd0/package/
- /dev_usb000/
- /dev_usb000/pkg/
- /dev_usb000/package/
- /dev_usb001/
- /dev_usb001/pkg/
- /dev_usb001/package/
- /dev_usb002/
- /dev_usb002/pkg/
- /dev_usb002/package/
- /dev_usb003/
- /dev_usb003/pkg/
- /dev_usb003/package/
- /dev_cf/
- /dev_cf/pkg
- /dev_cf/package
- /dev_cf/packages
- /dev_sd/
- /dev_sd/pkg
- /dev_sd/package
- /dev_sd/packages
- /dev_ms/
- /dev_ms/pkg
- /dev_ms/package
- /dev_ms/packages

NOTES: 
-----------------------------------------------
The application handles big PKG files without problem, 
remember that these take some time to copy, make sure to 
have at least double the space on the HDD.

- The application doesn't support re-scaning devices for new PKG files (for now)
- The application doesn't display a Progress bar (for now)
- The application doesn't support split PKG files (for now)
- The application doesn't have any special GUI (don't know how to, sorry lol) 

Usage:
----------------------------------------------

- Use UP and DOWN to select PKGs.
- Press (X) to Queue the selected PKG.
- Press (O) to Quit and return to XMB.
- Press ([ ]) to Remove the selected PKG from queue.

Changelog:
----------------------------------------------

1.1

- Added feature to remove previously queued PKG by pressing ([ ]) square.
- Added scan of PKG files on root of USB devices as well (from "/dev_usb000/" to "/dev_usb003/")
- Added scan of PKG files on "/dev_cf/..." (compact flash), "/dev_sd/..." (sd card), "/dev_ms/..." (memory stick)
- Improved application performance by cleaning up, organizing and simplifying lot of code.
- Copy of PKG files is faster now.
- Re-wrote module to write the PDB & Icon files from scratch.
- Other source code improvements.

1.0

- Initial "Test" Release

Thanks to:
----------------------------------------------
 - aldostools - your ps3tools for windows are the best =)
 - deank - multiMAN helped me research / debug my application
 - fail0verfl0w, GeoHot, etc - for all you have done for the PS3 scene  
 - Annonymous - for releasing Sony SDKs
 
 Thanks to anyone else I forgot that in some way contributed
 to the PS3 scene and as a result I was able to do this 
 homebrew application.
