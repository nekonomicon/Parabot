#include "parabot.h"
#include "chat.h"
#include "pb_configuration.h"
#include "bot.h"
#include "weapon.h"
#include "utf8_strfunc.h"
#include <ctype.h>
#include <stdio.h>
extern PB_Configuration pbConfig;	// from configfiles.cpp
extern bot_t bots[32];   // from bot.cpp
extern int clientWeapon[32];	// from combat.cpp
static CHAT chat;
extern int message_SayText;

/*
// speech start
#include <mmsystem.h>
#include <objbase.h>
#include <speech.h>

extern PITTSCRALW gpITTSCentral;
// speech end
*/


// this should go into the player class:
void
chat_botmessage()
// prints the message to everybody
{
	if(!chat.nextspeaker)
		return;
/*
	// speech start
	WCHAR wszSpeak[256];
	SDATA data;
	
	// Speak
	data.dwSize = (DWORD)MultiByteToWideChar( CP_ACP, 0, msg, -1, wszSpeak,
											  sizeof(wszSpeak) / sizeof(WCHAR) ) * sizeof(WCHAR);
	data.pData = wszSpeak;
	gpITTSCentral->TextData ( CHARSET_TEXT, 0, data, NULL, IID_ITTSBufNotifySinkW );
	// speech end
*/
	if (!chat.speechsynthesis) {
		char sayText[256];
		sayText[0] = 2; // turn on color set 2  (color on,  no sound)
                sayText[1] = '\0';
		strcat( sayText, STRING(chat.nextspeaker->v.netname) );
		strcat( sayText, ": " );
		strncat( sayText, chat.nextmessage, sizeof( sayText ) - strlen( sayText ) - 2 ); // -2 for /n and null terminator
		strcat( sayText, "\n" );

		MSG_Begin(MSG_ALL, message_SayText, NULL, NULL);
			MSG_WriteByte(indexofedict( chat.nextspeaker ) );
			MSG_WriteString(sayText);
		MSG_End();

		if (is_dedicatedserver()) printf( "%s", sayText );
	} else {
		DEBUG_MSG( "Speaking %s\n", chat.nextmessage );
		sound( chat.nextspeaker, CHAN_VOICE, chat.nextmessage, 1.0, ATTN_IDLE, 0, randomint(90, 120));
	}
}

bool
chat_load()
{
	char str[256], filename[64];
	int fileSize, filePos = 0;
	const char *pBuffer;
	byte *pMemFile;
	CHATLIST *currentCodeBlock;
	REPLYLIST *nextReplyList;

	strcpy( filename, "addons/parabot/config/lang/chat/" );
	strcat( filename, pbConfig.chatFile() );
	strcat( filename, ".txt" );

	INFO_MSG("Reading %s... ", filename);

	pMemFile = loadfileforme(filename, &fileSize);
	if(!pMemFile) {
		INFO_MSG("failed\n");

		return false;
	}

	if( chat.fileloaded )
		chat_free();

	currentCodeBlock = NULL;

	while((pBuffer = memfgets( pMemFile, fileSize, &filePos))) {
		// skip whitespace
		while( *pBuffer && isspace( *pBuffer ) )
			pBuffer++;

		if( !*pBuffer )
			continue;

		// skip comment lines
		if( *pBuffer == '#' )
			continue;

		if (*pBuffer=='@') {		// new codeblock
			sscanf( pBuffer, "@%s", str );
			if ( Q_STRIEQ( str, "USE_SPEECH_SYNTHESIS" ) ) chat.speechsynthesis = true;
			else if ( Q_STRIEQ( str, "GOT_KILLED"    ) ) currentCodeBlock = &chat.gotkilled;
			else if ( Q_STRIEQ( str, "KILLED_PLAYER" ) ) currentCodeBlock = &chat.killedplayer;
			else if ( Q_STRIEQ( str, "GOT_WEAPON"    ) ) currentCodeBlock = &chat.gotweapon;
			else if ( Q_STRIEQ( str, "JOINED_SERVER" ) ) currentCodeBlock = &chat.join;
			else if ( Q_STRIEQ( str, "REPLY_UNKNOWN" ) ) currentCodeBlock = &chat.replyunknown;
			else if ( Q_STRIEQ( str, "REPLY"         ) ) {
				pBuffer += 6; // skip @REPLY

				// skip whitespace
				while( *pBuffer && isspace( *pBuffer ) )
					pBuffer++;

				if(!*pBuffer)
					continue;

				// ReplyList *newReplyList = new ReplyList;
				REPLYLIST *newReplyList = (REPLYLIST *)malloc(sizeof(REPLYLIST));
				sscanf(pBuffer, " \"%31[^\"]\" ", newReplyList->code);
				UTF8_strupr(newReplyList->code);			// convert to upper case
				newReplyList->reply = (CHATLIST *)calloc(1, sizeof(CHATLIST));
				//newReplyList->reply = new ChatList;
				if (!chat.replies) {
					chat.replies = newReplyList;
					nextReplyList = chat.replies;
				} else {
					nextReplyList->next = newReplyList;
					nextReplyList = nextReplyList->next;
				}
				// chatReplies.push_back( newReplyList );
				currentCodeBlock = newReplyList->reply;
				do {
					while( *pBuffer && *pBuffer != ',' )
						pBuffer++;

					if( !*pBuffer )
                                                break;

					REPLYLIST *addReplyList = (REPLYLIST *)malloc(sizeof(REPLYLIST));
					sscanf(pBuffer, ", \"%31[^\"]\" ", addReplyList->code);
					//ReplyList *addReplyList = new ReplyList;
					UTF8_strupr( addReplyList->code );			// convert to upper case
					addReplyList->next = NULL;
					addReplyList->reply = currentCodeBlock;
					nextReplyList->next = addReplyList;
					nextReplyList = nextReplyList->next;
					// chatReplies.push_back( addReplyList );
					pBuffer++;
				} while (true);
			}
		} else {
			sscanf(pBuffer, "%255[^\n]", str);	// read entire line
			//if( !currentCodeBlock )
			//	continue;
			if (chat.speechsynthesis)
				strcat(str, ".wav");
			// currentCodeBlock = malloc(sizeof(CHATMESSAGE));
			//CHATMESSAGE *cm = malloc(sizeof(CHATMESSAGE));
			//cm->next = NULL;
			if(currentCodeBlock->message.text) {
				currentCodeBlock->next = (CHATLIST *)calloc(1, sizeof(CHATLIST));
				currentCodeBlock = currentCodeBlock->next;
			}
			currentCodeBlock->message.text = strdup(str);
			currentCodeBlock->message.time = -1000;
			// currentCodeBlock->push_back( cm );	// add to current block
		}
	}
	freefile( pMemFile );
	chat.fileloaded = true;
	INFO_MSG( "OK!\n" );

	return true;
}

static size_t
chat_countofmsg(CHATLIST *msglist)
{
	size_t i = 0;
	while (msglist) {
		i++;
		msglist = msglist->next;
	}

	return i;
}

static CHATLIST *
chat_findmsgbynumber(CHATLIST *msglist, size_t number)
{
	while(0 < number) {
		--number;
		msglist = msglist->next;
	}

	return msglist;
}

static void
chat_clearmsglist(CHATLIST *msglist)
{
	ptrdiff_t i = 0;
	CHATLIST *current, *next;

	current = msglist;
	while (current) {
		next = current->next;
		free((void *)current->message.text);
		if (0 < i) {
			free((void *)current);
		}
		current = next;
		i = 1;
	}
	msglist->next = NULL;
	msglist->message.text = NULL;
}

static void
chat_clearreplylist(REPLYLIST *replylist)
{
	ptrdiff_t i = 0;
	REPLYLIST *current, *next;

	current = replylist;
	while (current) {
		next = current->next;
		chat_clearmsglist(current->reply);
		free((void *)current->reply);
		if (0 < i) {
			free((void *)current);
		}
		current = next;
		i = 1;
	}
	replylist->next = NULL;
	replylist->reply = NULL;
}

bool
chat_free()
// free all memory allocated for botchat
{
	if (!chat.fileloaded)
		return false;

	chat_clearmsglist(&chat.gotkilled);
	chat_clearmsglist(&chat.killedplayer);
	chat_clearmsglist(&chat.gotweapon);
	chat_clearmsglist(&chat.join);
	chat_clearmsglist(&chat.replyunknown);
	chat_clearreplylist(chat.replies);

	chat.fileloaded = false;
	return true;
}

CHATMESSAGE *
chat_getmsgfromlist(CHATLIST *msglist, bool forceReply)
{
	size_t i = 0, c = 0;
	bool msgFound = false;
	CHATLIST *current;

	if (msglist->message.text) {
		i = chat_countofmsg(msglist);

		do {
			c++;
			current = chat_findmsgbynumber(msglist, randomint(0, i - 1));

			if (worldtime() > (current->message.time + 600.0f)
			    || worldtime() < current->message.time
			    || forceReply)
				msgFound = true;
		} while (3 > c && !msgFound);
		if (msgFound)
			return &current->message;
	}
	return 0;
}

EDICT *
chat_findnameinmsg( const char *msg, bool forceReply )
// looks if one of the playernames appears in msg and returns the correponding edict
// or 0 if msg doesn't contain any playernames
{
	char name1[32], name2[32];

	for (int i = 0; i < 32; i++) {
		if (bots[i].is_used) {
			strcpy(name1, STRING(bots[i].e->v.netname));
			UTF8_strupr(name1);
			const char *firstName = &name1[0];

			const char *clanTagStart = strchr(firstName, '[');
			if (clanTagStart) {
				const char *clanTagEnd = strchr(firstName, ']');
				if ((clanTagEnd - clanTagStart) < 5
				    && strlen(firstName) > 5)
					firstName = clanTagEnd + 1;
			}
			char *space = (char *)strchr(firstName, ' ');
			if (space) {
				*space = 0;						// shorten first part
				strcpy(name2, space + 1);		// store second part
			}
			UTF8_strupr(name1);
			// check if one of the two parts appears in msg and return botedict
			if (strstr(msg, firstName)
			    || (space && strstr(msg, name2))) {
				int chatRate = pbConfig.personality(bots[i].personality).communication;
				int rand = randomint(1, PERSONAL_REPLY_CHAT);
				if ((rand < chatRate) || forceReply)
					return bots[i].e;
			}
		}
	}

	return 0;
}

static void
chat_checkmsgforweapon(const char *msg, const char *wpnName, EDICT *wpnOwner, char *weaponmsg)
// checks if msg contains %w and replaces it by the weapon wpnOwner is carrying
{
	char usedWpn[32], *wpnPos, *namePos;

	wpnPos = (char *)strstr(msg, "%w");
	if (!wpnPos) {
		strcpy(weaponmsg, msg);
		return;
	}

	if (strncmp(wpnName, "weapon_", 7) == 0)
		strcpy(usedWpn, wpnName + 7);
	else {
		// TODO: check for tripmines!
		int owner = indexofedict(wpnOwner) - 1;
		strcpy(usedWpn, weapon_getweaponname(clientWeapon[owner]));
	}

	namePos = (char *)strstr(msg, "%s");

	if (!namePos) {
		wpnPos[1] = 's';
		sprintf(weaponmsg, msg, usedWpn);
		wpnPos[1] = 'w';
	} else {
		namePos[0] = '!';
		wpnPos[1] = 's';
		sprintf(weaponmsg, msg, usedWpn);
		wpnPos[1] = 'w';
		namePos[0] = '%';
		namePos = strstr(weaponmsg, "!s");
		namePos[0] = '%';
	}
}

static void
chat_getname(EDICT *player, char *namebuffer)
{
	const char *fullName, *clanTagStart, *clanTagEnd;
	char *space;

	fullName = STRING(player->v.netname);

	clanTagStart = strchr(fullName, '[');
	if (clanTagStart) {
		clanTagEnd = strchr(fullName, ']');
		int remove = randomint(0, 2);		// remove clantag in 2/3 of all cases
		if ((clanTagEnd - clanTagStart) < 5
		    && strlen(fullName) > 5
		    && remove > 0)
			fullName = clanTagEnd + 1;
	}

	strcpy(namebuffer, fullName);

	space = (char *)strchr(namebuffer, ' ');
	if (space) {
		int rand = randomint(0, 2);
		if (rand == 1) { // return first part
			*space = '\0';
		} else if (rand == 2) { // return second part
			strcpy(namebuffer, (space + 1));
		}
	}
}

EDICT *
chat_getrandomresponder(EDICT *excluding, bool forcereply)
// choses a random bot to respond, but not the excluding one
{
	int i, numcandidates = 0, chatrate, rand, chosen;
	int replycandidate[com.globals->maxclients];

	for (i = 0; i < com.globals->maxclients; i++) {
		if (bots[i].is_used && bots[i].e != excluding) {
			chatrate = pbConfig.personality(bots[i].personality).communication;
			rand = randomint(1, REPLY_CHAT);
			if ((rand < chatrate) || forcereply) {
				replycandidate[numcandidates] = i;
				numcandidates++;
			}
		}
	}

	if (numcandidates == 0)
		return 0;

	chosen = randomint(0, numcandidates - 1);

	return bots[replycandidate[chosen]].e;
}

void
chat_suggestmsg(EDICT *speaker, CHATMESSAGE *msg, EDICT *objective, const char *realText)
// realText (if supplied) is the modified msg
{
	const char *realMsg;
	char suggestedMsg[256], namebuffer[32];
	float suggestedTime;

	if (realText)
		realMsg = realText;
	else
		realMsg = msg->text;

	if (objective) {
		chat_getname(objective, namebuffer);
		sprintf(suggestedMsg, realMsg, namebuffer);
	} else
		strcpy(suggestedMsg, realMsg);

	suggestedTime = worldtime() + 1.0f + (float)strlen(suggestedMsg) * 0.2f;

	if (suggestedTime < chat.nexttime || chat.nexttime == 0) {
		strcpy(chat.nextmessage, suggestedMsg);
		chat.nextmsgptr = msg;
		chat.nexttime = suggestedTime;
		chat.nextspeaker = speaker;
	}
}

void
chat_parsemsg(EDICT *speaker, const char *msg)
// analyze the message the speaker says
{
	REPLYLIST *nextreply;
	if ( speaker == 0 || msg == 0 ) return;

	if (pbConfig.onChatLog()) {
		char logfile[64];
		sprintf(logfile, "%s/addons/parabot/log/chat.txt", com.modname);
		FILE *fp = fopen(logfile, "a");
		if (!(speaker->v.flags & FL_FAKECLIENT))
			fprintf(fp, "[HUMAN]");
		fprintf(fp, "%s: %s\n", STRING(speaker->v.netname), msg);
		fclose(fp);
	}

	if (!pbConfig.usingChat())
		return;

	// delete old messages from queue:
	chat.nexttime = 0;

	// build parseBuffer
	char parseBuffer[256];
	strcpy( parseBuffer, ": " );
	strcat( parseBuffer, msg );
	strcat( parseBuffer, " " );
	UTF8_strupr( parseBuffer );	// convert to upper case

	// replace special characters
	int len = strlen( parseBuffer );
	int pos, i;
	while ((pos = strcspn(parseBuffer, ",;-'()/.!?")) < len)
		parseBuffer[pos]=' ';

	// find keyword
	const char *wordFound = 0;
	for (nextreply = chat.replies; nextreply; nextreply = nextreply->next) {
		wordFound = strstr(parseBuffer, nextreply->reply->message.text);
		if (wordFound)
			break;
	}

	// check if this has to be answered in any case:
	bool forceAnswer = false;
	if (pbConfig.onAlwaysRespond()
	    && !(speaker->v.flags & FL_FAKECLIENT))
		forceAnswer = true;
	// find answer:
	CHATMESSAGE *answer;

	answer = chat_getmsgfromlist(wordFound ? nextreply->reply : &chat.replyunknown, forceAnswer);

	// find someone to answer:
	if (answer) {
		EDICT *responder = chat_findnameinmsg(parseBuffer, forceAnswer);
		if (!responder)
			responder = chat_getrandomresponder(speaker, forceAnswer);

		// suggest message
		if (responder)
			chat_suggestmsg(responder, answer, speaker, NULL);
	}
}

void
chat_check()
// checks if the next chat message should get displayed
{
	if (!pbConfig.usingChat())
		return;

	if (chat.nexttime > 0) {
		if (worldtime() > chat.nexttime) {
			if (playerexists(chat.nextspeaker)) {	// make sure player hasn't left the game
				chat_botmessage();
				chat.nextmsgptr->time = worldtime();				// store time
				chat_parsemsg(chat.nextspeaker, chat.nextmessage);	// send message to others
			}
		} else if (chat.nexttime > (worldtime() + 30.0))	// mapchange...
			chat.nexttime = 0;
	}
}

void
chat_registergotkilled( EDICT *victim, EDICT *killer, const char *wpnName )
{
	if (!pbConfig.usingChat())
		return;

	if (chat.gotkilled.message.text) {
		bot_t *bot = getbotpointer(victim);
		if (bot == 0)
			return;
		int chatrate = pbConfig.personality(bot->personality).communication;
		int rand = randomint(1, GOT_KILLED_CHAT);
		if (rand < chatrate) {
			CHATMESSAGE *msg = chat_getmsgfromlist(&chat.gotkilled, false);
			if (msg) {
				char weaponmsg[256];
				chat_checkmsgforweapon(msg->text, wpnName, killer, weaponmsg);
				chat_suggestmsg(victim, msg, killer, weaponmsg);
			}
		}
	}
}

void
chat_registerkilledplayer( EDICT *victim, EDICT *killer, const char *wpnName )
{
	if (!pbConfig.usingChat())
		return;

	if (chat.killedplayer.message.text) {
		bot_t *bot = getbotpointer(killer);
		if (!bot)
			return;
		int chatRate = pbConfig.personality(bot->personality).communication;
		int rand = randomint(1, KILLED_CHAT);
		if (rand < chatRate) {
			CHATMESSAGE *msg = chat_getmsgfromlist(&chat.killedplayer, false);
			if (msg) {
				char weaponmsg[256];
				chat_checkmsgforweapon(msg->text, wpnName, killer, weaponmsg);
				chat_suggestmsg(killer, msg, victim, weaponmsg);
			}
		}
	}
}

void
chat_registergotweapon(EDICT *finder, const char *wpnName)
{
	if (!pbConfig.usingChat()
	    || !(finder->v.flags & FL_FAKECLIENT))
		return;

	bot_t *bot = getbotpointer(finder);
	if (!bot)
		return;
	int chatRate = pbConfig.personality(bot->personality).communication;
	int rand = randomint(1, WEAPON_CHAT);
	if (rand < chatRate) {
		CHATMESSAGE *msg = chat_getmsgfromlist(&chat.gotweapon, false);
		if (msg) {
			char weaponmsg[256];
			chat_checkmsgforweapon(msg->text, wpnName, finder, weaponmsg);
			chat_suggestmsg(finder, msg, NULL, weaponmsg);
		}
	}
}

void
chat_registerjoin(EDICT *joiner)
{
	if (!pbConfig.usingChat()
	    || !(joiner->v.flags & FL_FAKECLIENT)
	    || worldtime() < 30.0f)	// don't greet at mapstart (just respawn, no new connect)
		return;

	bot_t *bot = getbotpointer(joiner);
	if (!bot)
		return;
	int chatRate = pbConfig.personality(bot->personality).communication;
	int rand = randomint(1, JOIN_CHAT);
	if (rand < chatRate) {
		CHATMESSAGE *msg = chat_getmsgfromlist(&chat.join, false);
		if (msg)
			chat_suggestmsg(joiner, msg, NULL, NULL);
	}
}
