#if !defined( PB_CONFIGURATION_H )
#define PB_CONFIGURATION_H


#include <stdio.h>



#define MAX_PERS 128	// different personalities


typedef struct {
	char name[32];
	char model[32];
	int  aimSkill;
	int  aggression;
	int  sensitivity;
	int  communication;
	bool inUse;
} PB_Personality;




class PB_Configuration
{


public:

	PB_Configuration();

	bool initConfiguration( const char *configPath );

	bool initPersonalities( const char *personalityPath );

	bool createConfiguration( const char *configFile );

	bool createPersonalities( const char *personalityFile );

	bool setBoolVar( const char *name, const char *value );

	bool setIntVar( const char *name, int value, int min, int max );

	PB_Personality personality( int index );

	char* getColor( int persNr, int modulo );

	void personalityJoins( int index, float time ) { character[index].inUse = true; }

	void personalityLeaves( int index, float time ) { character[index].inUse = false; }

	int	numberOfPersonalities()		{ return maxPers;				}

	int minSkill()					{ return minAimSkill;			}

	int maxSkill()					{ return maxAimSkill;			}

	int numBots()					{ return myNumBots;				}

	int minBots()					{ return myMinBots;				}

	int maxBots()					{ return myMaxBots;				}

	float stayTime()				{ return myStayTime;			}

	char* chatFile()				{ return chatFileName;			}

	char* menuActivation()			{ return menuKey;				}

	bool usingChat()				{ return botChat;				}

	bool onAlwaysRespond()			{ return chatAlwaysRespond;		}

	bool onServerMode()				{ return serverMode;			}

	bool onPeaceMode()				{ return peaceMode;				}

	bool onRestrictedWeaponMode()	{ return restrictedWeaponMode;	}

	bool onTouringMode()			{ return touringMode;			}

	bool onChatLog()				{ return chatLog;				}



protected:

	int clampInt( const char *str, int min, int max );

	bool varSet( const char *srcName, const char *srcValue, const char *varName, bool &var );

	bool varSet( const char *srcName, int srcValue, const char *varName, int &var );

	bool varSet( const char *srcName, FILE *file, const char *varName, bool &var );



private:

	int myNumBots, myMinBots, myMaxBots;
	float myStayTime;
	int minAimSkill, maxAimSkill;
	bool botChat, chatAlwaysRespond;
	char chatFileName[64];
	char menuKey[16];
	bool peaceMode, restrictedWeaponMode, serverMode, touringMode, chatLog;

	int maxPers;							// max. personlities
	PB_Personality character[MAX_PERS];	// stores different bot personalities
	
};

#endif
