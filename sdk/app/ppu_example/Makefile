PROJECT := $(notdir $(CURDIR))

EXPORT_LIST = 1

#define ON_AFTER_BUILD
#cp $(B8ROM_IMAGE) $(TOOL_TOP)/beep8_app_player/src/b8.bin/appplayer.bin
#endef

SPAGE = -s https://beep8.org:9443/b8/beep8.electron.html
HTTP_PORT = 9443
API_PORT = 9082
EXPORT_LIST = 1

include	../makefile.app
