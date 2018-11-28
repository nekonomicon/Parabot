#include "parabot.h"
#include "pb_configuration.h"
enum {
	NO_MENU,
	MAIN_MENU,
	MODE_MENU,
	CHAT_MENU,
	NUMBER_MENU,
	SKILL_MENU
};

char dynMenu[512];

int currentMenu;
int menuState = NO_MENU;
int menuChoice;
extern PB_Configuration pbConfig;
extern float bot_check_time;
extern int message_ShowMenu;
extern int oldBotStop;
extern bool pb_pause;
extern void adjustAimSkills();

static void menu_showtext( EDICT *player, int slots, int displaytime, bool needmore, const char *pText )
{
	MSG_Begin(MSG_ONE, message_ShowMenu, NULL, player);
	MSG_WriteShort(slots);
	MSG_WriteChar(displaytime);
	MSG_WriteByte(needmore);
	MSG_WriteString(pText);
	MSG_End();
}

static int
menu_slot(int number)
{
	int res = 0;
	for (int i = 0; i < number; i++)
		res |= BIT(i);
	return res;
}

void
menu_main(EDICT *player)
{
	const char *main_menu = {"\
	Parabot Configuration\n\n\
	1. Change Number of Bots\n\
	2. Change Botskill\n\
	3. Change Gamemodes\n\
	4. Configure Botchat\n\
	5. Exit\
	"};

	currentMenu = MAIN_MENU;
	menu_showtext( player, menu_slot(5), -1, false, main_menu );
}

static void
menu_gamemode( EDICT *player )
{
	const char *pszString;

	strcpy( dynMenu, "Change Gamemodes\n\n" );
	if( pbConfig.onRestrictedWeaponMode() ) 
		pszString = "1. Disable RestrictedWeapons\n";
	else
		pszString = "1. Enable RestrictedWeapons\n";

	strcat( dynMenu, pszString );
	if( pbConfig.onPeaceMode() )
		pszString = "2. Disable PeaceMode\n";
	else
		pszString = "2. Enable PeaceMode\n";

	strcat( dynMenu, pszString );
	strcat( dynMenu, "3. Exit\n" );

	currentMenu = MODE_MENU;
	menu_showtext( player, menu_slot(3), -1, false, dynMenu );
}

static void
menu_chat( EDICT *player )
{
	const char *pszString;

	strcpy( dynMenu, "Configure Botchat\n\n" );
	if ( pbConfig.usingChat() ) {
		strcat( dynMenu, "1. Disable Botchat\n" );
		if( pbConfig.onAlwaysRespond() ) 
			pszString = "2. Disable AlwaysRespond\n";
		else
			pszString = "2. Enable AlwaysRespond\n";

		strcat( dynMenu, pszString );
		if( pbConfig.onChatLog() ) 
			pszString = "3. Disable ChatLog\n";
		else
			pszString = "3. Enable ChatLog\n";

		strcat( dynMenu, pszString );
	}
	else {
		strcat( dynMenu, "1. Enable Botchat\n\n\n" );
	}
	strcat( dynMenu, "4. Exit\n" );

	currentMenu = CHAT_MENU;
	menu_showtext( player, menu_slot(4), -1, false, dynMenu );
}

static void
menu_botcount( EDICT *player )
{
	char buffer[64];

	strcpy( dynMenu, "Configure Number of Bots\n\n" );
	if (pbConfig.onServerMode()) {
		sprintf( buffer, "MinBots is %i\n", pbConfig.minBots() );
		strcat( dynMenu, buffer );
		strcat( dynMenu, "  1. -    2. +\n");
		sprintf( buffer, "MaxBots is %i\n", pbConfig.maxBots() );
		strcat( dynMenu, buffer );
		strcat( dynMenu, "  3. -    4. +\n");
		strcat( dynMenu, "5. Disable");
	}
	else {
		sprintf( buffer, "NumBots is %i\n", pbConfig.numBots() );
		strcat( dynMenu, buffer );
		strcat( dynMenu, "  1. -    2. +\n" );
		strcat( dynMenu, "\n\n5. Enable");
	}
	strcat( dynMenu, " ServerMode\n6. Exit\n" );

	currentMenu = NUMBER_MENU;
	menu_showtext( player, menu_slot(6), -1, false, dynMenu );
}

static void
menu_skill( EDICT *player )
{
	char buffer[64];

	strcpy( dynMenu, "Configure Bot Aimskill\n\n" );
	sprintf( buffer, "MinSkill is %i\n", pbConfig.minSkill() );
	strcat( dynMenu, buffer );
	strcat( dynMenu, "  1. -    2. +\n");
	sprintf( buffer, "MaxSkill is %i\n", pbConfig.maxSkill() );
	strcat( dynMenu, buffer );
	strcat( dynMenu, "  3. -    4. +\n");
	strcat( dynMenu, "5. Exit\n" );
	
	currentMenu = SKILL_MENU;
	menu_showtext( player, menu_slot(5), -1, false, dynMenu );
}

void
menu_selectitem( EDICT *player, int menuChoice )
{
	switch( currentMenu )
	{
	case MAIN_MENU:
		switch( menuChoice )
		{
		case 1:
			menuChoice = NUMBER_MENU;
			break;
		case 2:
			menuChoice = SKILL_MENU;
			break;					
		case 3:
			menuChoice = MODE_MENU;
			break;
		case 4:
			menuChoice = CHAT_MENU;
			break;
		default:
			currentMenu = menuChoice = NO_MENU; // exit		
			pb_pause = oldBotStop; 
			break;
		}
		break;
	case NUMBER_MENU:
		if( pbConfig.onServerMode() )
		{
			switch( menuChoice )
			{
			case 1:
				pbConfig.setFloatVar( "bot_minnum", pbConfig.minBots()-1, com.globals->maxclients );
				menuChoice = NUMBER_MENU;
				break;
			case 2:
				pbConfig.setFloatVar( "bot_minnum", pbConfig.minBots()+1, pbConfig.maxBots() );
				menuChoice = NUMBER_MENU;
				break;
			case 3:
				pbConfig.setFloatVar( "bot_maxnum", pbConfig.maxBots()-1, com.globals->maxclients , pbConfig.minBots() );
				menuChoice = NUMBER_MENU;
				break;
			case 4:
				pbConfig.setFloatVar( "bot_maxnum", pbConfig.maxBots()+1, com.globals->maxclients );
				menuChoice = NUMBER_MENU;
				break;
			case 5:
				pbConfig.setFloatVar( "bot_realgame" );
				bot_check_time = com.globals->time + 5.0;
				menuChoice = NUMBER_MENU;
				break;
			default:
				menuChoice = MAIN_MENU;
				break;
			}
		}
		else
		{
			switch( menuChoice )
			{
			case 1:
				pbConfig.setFloatVar( "bot_num", pbConfig.numBots()-1, com.globals->maxclients );
				menuChoice = NUMBER_MENU;
				break;
			case 2:
				pbConfig.setFloatVar( "bot_num", pbConfig.numBots()+1, com.globals->maxclients );
				menuChoice = NUMBER_MENU;
				break;
			case 3: 
			case 4:
				menuChoice = NUMBER_MENU;
				break;
			case 5:
				pbConfig.setFloatVar( "bot_realgame", 1 );
				bot_check_time = com.globals->time + 5.0;
				menuChoice = NUMBER_MENU;
				break;
			default:
				menuChoice = MAIN_MENU;
				break;
			}
			break;
		}
	case SKILL_MENU:
		switch( menuChoice )
		{
		case 1:
			pbConfig.setFloatVar( "bot_minaimskill", pbConfig.minSkill()-1, 10, 1 );
			menuChoice = SKILL_MENU;
			break;
		case 2:
			pbConfig.setFloatVar( "bot_minaimskill", pbConfig.minSkill()+1, pbConfig.maxSkill(), 1 );
			menuChoice = SKILL_MENU;
			break;
		case 3:
			pbConfig.setFloatVar( "bot_maxaimskill", pbConfig.maxSkill()-1, 10, pbConfig.minSkill() );
			menuChoice = SKILL_MENU;
			break;
		case 4:
			pbConfig.setFloatVar( "bot_maxaimskill", pbConfig.maxSkill()+1, 10, 1 );
			menuChoice = SKILL_MENU;
			break;
		default:
			menuChoice = MAIN_MENU;
			break;
		}
		adjustAimSkills();
		break;
	case MODE_MENU:
		switch( menuChoice )
		{
		case 1: 
			pbConfig.setFloatVar( "bot_restrictedweapons", pbConfig.onRestrictedWeaponMode() ? 0 : 1 );
			menuChoice = MODE_MENU;
			break;
		case 2:
			pbConfig.setFloatVar( "bot_peacemode", pbConfig.onPeaceMode() ? 0 : 1 );
			menuChoice = MODE_MENU;
			break;
		default:
			menuChoice = MAIN_MENU;
			break;
		}
		break;
	case CHAT_MENU:
		switch( menuChoice )
		{
		case 1:
			pbConfig.setFloatVar( "bot_chat_enabled", pbConfig.usingChat() ? 0 : 1 );
			menuChoice = CHAT_MENU;
			break;
		case 2:
			if( pbConfig.usingChat() )
				pbConfig.setFloatVar( "bot_chatrespond", pbConfig.onAlwaysRespond() ? 0 : 1 );
			menuChoice = CHAT_MENU;
			break;
		case 3:
			if( pbConfig.usingChat() )
				pbConfig.setFloatVar( "bot_chatlog", pbConfig.onChatLog() ? 0 : 1 );
			menuChoice = CHAT_MENU;
			break;
		default:
			menuChoice = MAIN_MENU;
			break;
		}
		break;
	default:
		break;
	}

	switch( menuChoice )
	{
	case MAIN_MENU:
		menu_main(player);
		break;
	case MODE_MENU:
		menu_gamemode(player);
		break;
	case CHAT_MENU:
		menu_chat(player);
		break;
	case NUMBER_MENU:
		menu_botcount(player);
		break;
	case SKILL_MENU:
		menu_skill(player);
		break;
	case NO_MENU:
	default:
		break;
	}
}
