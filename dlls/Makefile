# Makefile for parabot
# Copyright © nekonomicon 2016

CC ?=		gcc
CXX ?=		g++

CPPFLAGS =	

SYS =		$(shell $(CXX) -dumpmachine)

MODULE =	parabot

ifneq ($(PB_64BIT),1)
ifneq (, $(findstring x86_64, $(SYS)))
CFLAGS +=	-m32
LDFLAGS +=	-m32 -Wl,-z,notext
endif
else
ifneq (, $(findstring x86_64, $(SYS)))
ifneq (, $(findstring mingw, $(SYS)))
MODULE =	parabot64
else
ifneq (, $(findstring linux, $(SYS)))
MODULE =	parabot64
endif
ifneq (, $(findstring darwin, $(SYS)))
MODULE =	parabot64
endif
endif
CFLAGS +=       -fPIC
endif
endif

ifneq ($(DEBUG),1)
CFLAGS +=	-O2 -pipe -DNDEBUG
ifeq (, $(findstring darwin, $(SYS)))
LDFLAGS +=	-Wl,--strip-all
endif
ifeq ($(NO_EXCEPTIONS),1)
CPPFLAGS +=	-fno-exceptions -fno-unwind-tables -fno-asynchronous-unwind-tables
CFLAGS +=	-fomit-frame-pointer
endif
else
CFLAGS +=	-O0 -pipe -DDEBUG -D_DEBUG -g -gdwarf-2
endif

ifeq ($(AGGR_OPT),1)
CPPFLAGS +=	-fvisibility-inlines-hidden -fno-rtti
CFLAGS +=	-O3 -fvisibility=hidden -ffast-math -funroll-loops -ftree-vectorize -fno-common
endif

ifeq ($(SIZE_OPT),1)
CPPFLAGS +=     -fvisibility-inlines-hidden -fno-rtti
CFLAGS +=	-Os -fvisibility=hidden
endif

ifneq ($(LTO),1)
ifeq ($(SIZE_OPT),1)
ifneq (, $(findstring mingw, $(SYS)))
CFLAGS +=	-fno-data-sections
else
CFLAGS +=	-fdata-sections
endif
CFLAGS +=	-ffunction-sections
LDFLAGS +=	-Wl,--gc-sections
endif
else
CFLAGS +=	-flto
LDFLAGS +=	-flto
endif

ifeq ($(STATIC_GCCLIBS),1)
LDFLAGS +=	-static-libgcc -static-libstdc++
endif

ifneq (, $(findstring mingw, $(SYS)))
LDFLAGS +=	-Wl,--add-stdcall-alias -static-libgcc -static-libstdc++ -Wl,--no-undefined
OSLIB =		dll
else
ifneq (, $(findstring darwin, $(SYS)))
OSLIB =		dylib
else
OSLIB =		so
LDFLAGS +=	-Wl,--no-undefined
ifneq (, $(findstring linux, $(SYS)))
LDFLAGS +=	-ldl
endif
endif
endif

INCLUDES =	-I. \
		-I../bot \
		-I../utils \
		-I../common \
		-I../dlls \
		-I../engine \
		-I../pm_shared \
		-I../metamod

SRC =		../bot/bot.cpp \
		../bot/bot_client.cpp \
		dllwrap.cpp \
		../engine/engwrap.cpp \
		mathlib.cpp \
		physint.cpp \
		../bot/marker.cpp \
		../bot/parabot.cpp \
		../bot/action.cpp \
		../bot/cell.cpp \
		../bot/menu.cpp \
		../bot/chat.cpp \
		../bot/combat.cpp \
		../bot/combatgoals.cpp \
		../bot/configuration.cpp \
		../bot/focus.cpp \
		../bot/pb_global.cpp \
		../bot/goalfinder.cpp \
		../bot/goals.cpp \
		../bot/journey.cpp \
		../bot/kills.cpp \
		../bot/mapcells.cpp \
		../bot/mapgraph.cpp \
		../bot/mapimport.cpp \
		../bot/navpoint.cpp \
		../bot/needs.cpp \
		../bot/observer.cpp \
		../bot/path.cpp \
		../bot/perception.cpp \
		../bot/personalities.cpp \
		../bot/roaming.cpp \
		../bot/sectors.cpp \
		../bot/vistable.cpp \
		../bot/weapon.cpp \
		../bot/weaponhandling.cpp \
		../utils/priorityqueue.cpp \
		../bot/sounds.cpp \
		../bot/utilityfuncs.cpp \
		../metamod/meta_api.cpp


LDFLAGS +=	-shared
LIBNAME =	$(MODULE).$(OSLIB)

OBJ = $(SRC:%.cpp=%.o)

all: $(LIBNAME)

$(LIBNAME): $(OBJ)
	$(CXX) $(OBJ) $(LDFLAGS) -s -o $(LIBNAME)

.cpp.o:
	$(CXX) $(CFLAGS) $(CPPFLAGS) $(INCLUDES)  -c $< -o $@

.PHONY: all clean

clean:
	rm -rf $(OBJ)
	rm $(LIBNAME)
