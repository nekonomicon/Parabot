#include "parabot.h"
#include "configuration.h"
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
	if(configuration_onrestrictedweaponmode()) 
		pszString = "1. Disable RestrictedWeapons\n";
	else
		pszString = "1. Enable RestrictedWeapons\n";

	strcat( dynMenu, pszString );
	if(configuration_onpeacemode())
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
	if (configuration_usingchat()) {
		strcat( dynMenu, "1. Disable Botchat\n" );
		if (configuration_onalwaysrespond() ) 
			pszString = "2. Disable AlwaysRespond\n";
		else
			pszString = "2. Enable AlwaysRespond\n";

		strcat( dynMenu, pszString );
		if(configuration_onchatlog() ) 
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
	if (configuration_onservermode()) {
		sprintf( buffer, "MinBots is %i\n", configuration_minbots() );
		strcat( dynMenu, buffer );
		strcat( dynMenu, "  1. -    2. +\n");
		sprintf( buffer, "MaxBots is %i\n", configuration_maxbots() );
		strcat( dynMenu, buffer );
		strcat( dynMenu, "  3. -    4. +\n");
		strcat( dynMenu, "5. Disable");
	}
	else {
		sprintf( buffer, "NumBots is %i\n", configuration_numbots() );
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
	sprintf( buffer, "MinSkill is %i\n", configuration_minskill() );
	strcat( dynMenu, buffer );
	strcat( dynMenu, "  1. -    2. +\n");
	sprintf( buffer, "MaxSkill is %i\n", configuration_maxskill() );
	strcat( dynMenu, buffer );
	strcat( dynMenu, "  3. -    4. +\n");
	strcat( dynMenu, "5. Exit\n" );
	
	currentMenu = SKILL_MENU;
	menu_showtext( player, menu_slot(5), -1, false, dynMenu );
}

void
menu_selectitem( EDICT *player, int menuChoice )
{
	const char *cvarname = NULL;
	float value = 0.0f;
	int maxvalue = 1, minvalue = 0;

	switch (currentMenu) {
	case MAIN_MENU:
		switch(menuChoice) {
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
		if (configuration_onservermode()) {
			switch(menuChoice) {
			case 1:
				cvarname = "bot_minnum";
				value = configuration_minbots() - 1;
				maxvalue = com.globals->maxclients;
				menuChoice = NUMBER_MENU;
				break;
			case 2:
				cvarname = "bot_minnum";
				value = configuration_minbots() + 1;
				maxvalue = configuration_maxbots();
				menuChoice = NUMBER_MENU;
				break;
			case 3:
				cvarname = "bot_maxnum";
				value = configuration_maxbots() - 1;
				maxvalue = com.globals->maxclients;
				minvalue = configuration_minbots();
				menuChoice = NUMBER_MENU;
				break;
			case 4:
				cvarname = "bot_maxnum";
				value = configuration_maxbots() + 1;
				maxvalue = com.globals->maxclients;
				menuChoice = NUMBER_MENU;
				break;
			case 5:
				cvarname = "bot_realgame";
				bot_check_time = com.globals->time + 5.0;
				menuChoice = NUMBER_MENU;
				break;
			default:
				menuChoice = MAIN_MENU;
				break;
			}
		} else {
			switch(menuChoice) {
			case 1:
				cvarname = "bot_num";
				value = configuration_numbots() - 1;
				maxvalue = com.globals->maxclients;
				menuChoice = NUMBER_MENU;
				break;
			case 2:
				cvarname = "bot_num";
				value = configuration_numbots() + 1;
				maxvalue = com.globals->maxclients;
				menuChoice = NUMBER_MENU;
				break;
			case 3: 
			case 4:
				menuChoice = NUMBER_MENU;
				break;
			case 5:
				cvarname = "bot_realgame";
				value = 1.0f;
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
		switch(menuChoice) {
		case 1:
			cvarname = "bot_minaimskill";
			value = configuration_minskill() - 1;
			maxvalue = 10;
			minvalue = 1;
			menuChoice = SKILL_MENU;
			break;
		case 2:
			cvarname = "bot_minaimskill";
			value = configuration_minskill() + 1;
			maxvalue = configuration_maxskill();
			minvalue = 1;
			menuChoice = SKILL_MENU;
			break;
		case 3:
			cvarname = "bot_maxaimskill";
			value = configuration_maxskill() - 1;
			maxvalue = 10;
			minvalue = configuration_minskill();
			menuChoice = SKILL_MENU;
			break;
		case 4:
			cvarname = "bot_maxaimskill";
			value = configuration_maxskill() + 1;
			maxvalue = 10;
			minvalue = 1;
			menuChoice = SKILL_MENU;
			break;
		default:
			menuChoice = MAIN_MENU;
			break;
		}
		break;
	case MODE_MENU:
		switch (menuChoice) {
		case 1: 
			cvarname = "bot_restrictedweapons";
			value = configuration_onrestrictedweaponmode() ? 0 : 1;
			menuChoice = MODE_MENU;
			break;
		case 2:
			cvarname = "bot_peacemode";
			value = configuration_onpeacemode() ? 0 : 1;
			menuChoice = MODE_MENU;
			break;
		default:
			menuChoice = MAIN_MENU;
			break;
		}
		break;
	case CHAT_MENU:
		switch (menuChoice) {
		case 1:
			cvarname = "bot_chat_enabled";
			value = configuration_usingchat() ? 0 : 1;
			menuChoice = CHAT_MENU;
			break;
		case 2:
			if (configuration_usingchat()) {
				cvarname = "bot_chatrespond";
				value = configuration_onalwaysrespond() ? 0 : 1;
			}
			menuChoice = CHAT_MENU;
			break;
		case 3:
			if (configuration_usingchat()) {
				cvarname = "bot_chatlog";
                                value = configuration_onchatlog() ? 0 : 1;
			}
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

	if (cvarname) {
		configuration_setfloatconvar(cvarname, value, maxvalue, minvalue);
		adjustAimSkills();
	}

	switch (menuChoice) {
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
