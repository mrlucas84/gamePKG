PROJECT_VERSION		:= 01.00
SRC_EXTENSIONS		:= c cpp cc

# usually, you don't have to edit below here

CELL_MK_DIR = $(CELL_SDK)/samples/mk
include $(CELL_MK_DIR)/sdk.makedef.mk

MOD_ELF 		= bin/modELF
MAKE_SELF_NPDRM = bin/make_self_npdrm
PSN_PKG_NPDRM 	= bin/psn_package_npdrm

CONTENT_ID		= GP4PS3-GPKG00123_00-AAAABBBBCCCCDDDD

PPU_SRCS		= $(wildcard $(patsubst %,src/*.%,$(SRC_EXTENSIONS)))
PPU_OPTIMIZE_LV	= -O2
PPU_INCDIRS		+= -I$(CELL_FW_DIR)/include -Iinclude
PPU_CPPFLAGS	+=  -DPSGL
PPU_CXXSTDFLAGS	+= -fno-exceptions
PPU_TARGET		= gamePKG.elf
PPU_LIBS		+=  $(CELL_FW_DIR)/libfw.a
PPU_LDLIBDIR	+=  -L$(CELL_FW_DIR) -L$(PPU_PSGL_LIBDIR)
PPU_LDLIBS		+=  -lPSGL -lPSGLU -lPSGLFX -lm \
		-lresc_stub -lgcm_cmd -lgcm_sys_stub \
        -lnet_stub -lio_stub -lsysutil_stub -lsysmodule_stub -lfs_stub -ldbgfont -lfont_stub -lfontFT_stub -lfreetype_stub

include $(CELL_MK_DIR)/sdk.target.mk

gamePKG: $(PPU_TARGET)
	@$(PPU_STRIP) -s $(PPU_TARGET) -o $(OBJS_DIR)/$(PPU_TARGET)
	@$(MAKE_FSELF) $(OBJS_DIR)/$(PPU_TARGET) EBOOT.BIN
	@$(MAKE_FSELF_NPDRM) $(OBJS_DIR)/$(PPU_TARGET) release/PS3_GAME/USRDIR/EBOOT.BIN

pkg: $(PPU_TARGET)
	@$(PSN_PKG_NPDRM) release/package.conf release/PS3_GAME/ > nul
	mv *.pkg release/gamePKG.pkg	
