# Makefile for parabot
# Copyright © nekonomicon 2016

MODULE =	parabot
CC ?=		gcc
CXX ?=		g++

CFLAGS +=	-O2 -pipe -m32 -DNDEBUG \
		-fsigned-char -fPIC

DEFINES =	-Dstricmp=strcasecmp -D_stricmp=strcasecmp \
		-Dstrnicmp=strncasecmp -D_snprintf=snprintf \
		-Wno-write-strings -Wno-invalid-offsetof

INCLUDES =	-I$(PREFIX)/include \
		-I. \
		-I../bot \
		-I../utils \
		-I../hlsdk/common \
		-I../hlsdk/dlls \
		-I../hlsdk/engine \
		-I../hlsdk/public \
		-I../hlsdk/pm_shared \
		-I../hlsdk/game_shared

SRC =		../bot/bot.cpp \
		../bot/bot_client.cpp \
		commands.cpp \
		dll.cpp \
		engine.cpp \
		h_export.cpp \
		linkfunc.cpp \
		startframe.cpp \
		util.cpp \
		../bot/marker.cpp \
		../bot/parabot.cpp \
		../bot/pb_action.cpp \
		../bot/pb_cell.cpp \
		../bot/pb_chat.cpp \
		../bot/pb_combat.cpp \
		../bot/pb_combatgoals.cpp \
		../bot/pb_configuration.cpp \
		../bot/pb_focus.cpp \
		../bot/pb_global.cpp \
		../bot/pb_goalfinder.cpp \
		../bot/pb_goals.cpp \
		../bot/pb_journey.cpp \
		../bot/pb_kills.cpp \
		../bot/pb_mapcells.cpp \
		../bot/pb_mapgraph.cpp \
		../bot/pb_mapimport.cpp \
		../bot/pb_navpoint.cpp \
		../bot/pb_needs.cpp \
		../bot/pb_observer.cpp \
		../bot/pb_path.cpp \
		../bot/pb_perception.cpp \
		../bot/pb_roaming.cpp \
		../bot/pb_sectors.cpp \
		../bot/pb_vistable.cpp \
		../bot/pb_weapon.cpp \
		../bot/pb_weaponhandling.cpp \
		../bot/pbt_dynarray.cpp \
		../bot/pbt_priorityqueue.cpp \
		../bot/sounds.cpp \
		../bot/utilityfuncs.cpp \
		../hlsdk/dlls/animation.cpp

LDFLAGS +=	-shared -Wl,--no-undefined

SYS = $(shell $(CC) -dumpmachine)
ifeq (, $(findstring linux, $(SYS)))
LDFLAGS += -ldl
endif

OBJ = $(SRC:%.cpp=%.o)

all: $(MODULE).so

$(MODULE).so: $(OBJ)
	$(CXX) $(OBJ) $(LDFLAGS) -o $(MODULE).so

.cpp.o:
	$(CXX) $(CFLAGS) $(DEFINES) $(INCLUDES) -c $< -o $@

.PHONY: all clean

clean:
	rm -rf $(OBJ)
	rm $(MODULE).so
