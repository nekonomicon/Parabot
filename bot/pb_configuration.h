#pragma once
#if !defined(PB_CONFIGURATION_H)
#define PB_CONFIGURATION_H

#include "bot.h"
#include "pb_global.h"
#include <stdio.h>

extern CVAR bot_num;
extern CVAR bot_minnum;
extern CVAR bot_maxnum;
extern CVAR bot_realgame;
extern CVAR bot_staytime;
extern CVAR bot_chatenabled;
extern CVAR bot_chatlang;
extern CVAR bot_chatlog;
extern CVAR bot_chatrespond;
extern CVAR bot_peacemode;
extern CVAR bot_restrictedweapons;
extern CVAR bot_touringmode;
extern CVAR bot_maxaimskill;
extern CVAR bot_minaimskill;
extern CVAR bot_menukey;

typedef struct personality {
	char name[BOT_NAME_LEN];
	char model[BOT_SKIN_LEN];
	int aimSkill;
	int aggression;
	int sensitivity;
	int communication;
	char topColor[4];
	char bottomColor[4];
	bool inUse;
} PERSONALITY;

class PB_Configuration
{
public:
	PB_Configuration();

	bool initConfiguration( const char *configPath );

	bool initPersonalities( const char *personalityPath );

	bool createConfiguration( const char *configFile );

	bool createPersonalities( const char *personalityFile );

	bool savePersonalities( const char *personalityFile );

	void registerVars();

	PERSONALITY personality( int index );

	const char *getColor( int persNr, int modulo );

	const char *getTopColor( int index ) { return character[index].topColor; }

	const char *getBottomColor( int index ) { return character[index].bottomColor; }

	void personalityJoins( int index ) { character[index].inUse = true; }

	void personalityLeaves( int index ) { character[index].inUse = false; }

	int numberOfPersonalities()		{ return character.size(); }

	int minSkill()					{ return bot_minaimskill.value;	}

	int maxSkill()					{ return bot_maxaimskill.value;	}

	int numBots()					{ return bot_num.value;	}

	int minBots()					{ return bot_minnum.value;	}

	int maxBots()					{ return bot_maxnum.value;	}

	float stayTime()				{ return bot_staytime.value * 60; }

	const char *chatFile()				{ return bot_chatlang.string;	}

	bool usingChat()				{ return bot_chatenabled.value;	}

	bool onAlwaysRespond()			{ return bot_chatrespond.value;	}

	bool onServerMode()				{ return bot_realgame.value;	}

	bool onPeaceMode()				{ return bot_peacemode.value;	}

	bool onRestrictedWeaponMode()	{ return bot_restrictedweapons.value;	}

	bool onTouringMode()			{ return bot_touringmode.value;	}

	bool onChatLog()			{ return bot_chatlog.value;	}

	void setFloatVar( const char *name, float value = 0, int max = 1, int min = 0 )
	{
		cvar_setfloat( name, clamp( value, max, min ) );
	}

	bool loadModelList( const char *gamedir );

	void clearModelList();
private:
	std::vector<PERSONALITY> character;
	std::vector<char*> playerModelList;
};

#endif
