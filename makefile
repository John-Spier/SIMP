# Configuration
SRL_MALLOC_MEMORY = 2048000      # How much memory malloc can use (in bytes)
SRL_MAX_TEXTURES = 100          # Number of VDP1 texture slots
SRL_MODE = NTSC                 # Valid options are PAL or NTSC
SRL_HIGH_RES = 0                # 480i mode
SRL_FRAMERATE = 0               # Framerate control (0=dynamic, 1=< 60/value)
SRL_MAX_CD_BACKGROUND_JOBS = 1  # Maximum number of files GFS can open at once
SRL_MAX_CD_FILES = 3000          # Maximum number of files on a CD
SRL_MAX_CD_RETRIES = 5          # Number of times to retry on unsuccessful read

# Sound driver specific configuration
SRL_USE_SGL_SOUND_DRIVER = 1    # Set to 1 to enable the SGL sound API and link LIBSND.A
SRL_ENABLE_FREQ_ANALYSIS = 0    # Set to 1 if you want to enable frequency analysis for CD audio, this will load a DSP program into effect slot 1, SGL sound driver must be enabled

# SGL configuration
SGL_MAX_VERTICES = 2500         # Number of vertices that can be used
SGL_MAX_POLYGONS = 1500         # Number of polygons that can be used
SGL_MAX_EVENTS = 1             # Number of events that can be used
SGL_MAX_WORKS = 1             # Number of works that can be used 

# Disk name
CD_NAME = CD_demo

# Directory build will be placed into
BUILD_DROP = ./BuildDrop

# SRL installation directory
SRL_INSTALL_ROOT ?= ../..

# Find all .c and .cxx files
SOURCES = $(patsubst ./%,%,$(shell find src/ -name '*.c')) 
SOURCES += $(patsubst ./%,%,$(shell find src/ -name '*.cxx'))

# Include shared makefile
SDK_ROOT = $(SRL_INSTALL_ROOT)/saturnringlib
include $(SDK_ROOT)/shared.mk

# Link SBL PCM/ADPCM libraries
LIBS += $(SGLLDIR)/LIBPCM.A $(SGLLDIR)/LIBADP.A


# Override the shared create_iso target so SGL sound support is enabled
# without copying the SGL driver files (SDDRVS.DAT / SDDRVS.TSK) into the ISO.
#create_iso : convert_binary
#@mkdir -p $(MUSIC_DIR)
#@mkdir -p $(ASSETS_DIR)
#@mkdir -p $(BUILD_DROP)
#@test -f $(ASSETS_DIR)/ABS.TXT || echo "NOT Abstracted by SEGA" >> $(ASSETS_DIR)/ABS.TXT
#@test -f $(ASSETS_DIR)/BIB.TXT || echo "NOT Bibliographiced by SEGA" >> $(ASSETS_DIR)/BIB.TXT
#@test -f $(ASSETS_DIR)/CPY.TXT || touch $(ASSETS_DIR)/CPY.TXT
#xorrisofs --norock -quiet -sysid "SEGA SATURN" -volid "SaturnApp" -volset "SaturnApp" \
#-publisher "SEGA ENTERPRISES, LTD." -preparer "SEGA ENTERPRISES, LTD." -appid "SaturnApp" \
#-abstract "$(ASSETS_DIR)/ABS.TXT" -copyright "$(ASSETS_DIR)/CPY.TXT" \
#-biblio "$(ASSETS_DIR)/BIB.TXT" -generic-boot $(IPFILE) \
#-full-iso9660-filenames -o "$(BUILD_ISO)" $(ASSETS_DIR) $(ENTRYPOINT)
