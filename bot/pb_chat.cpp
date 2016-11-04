#include "pb_chat.h"
#include "pb_configuration.h"
#include "bot.h"
#include "pb_weapon.h"



extern PB_Configuration pbConfig;	// from configfiles.cpp
extern bot_t bots[32];   // from bot.cpp
extern int clientWeapon[32];	// from combat.cpp
extern int gmsgSayText;


/*
// speech start
#include <mmsystem.h>
#include <objbase.h>
#include <speech.h>

extern PITTSCENTRALW gpITTSCentral;
// speech end
*/


void pfnEmitSound( edict_t *entity, int channel, const char *sample, /*int*/float volume, float attenuation, int fFlags, int pitch );
// from engine.h

void pfnEmitAmbientSound(edict_t *entity, float *pos, const char *samp, float vol, float attenuation, int fFlags, int pitch);
extern edict_t *playerEnt;

// this should go into the player class:
void botChatMessage( edict_t *speaker, char *msg, bool speechSynthesis )
// prints the message to everybody
{
	if ( speaker == 0 || msg == 0 ) return;

/*
	// speech start
	WCHAR wszSpeak[1024];
	SDATA data;
	
	// Speak
	data.dwSize = (DWORD)MultiByteToWideChar( CP_ACP, 0, msg, -1, wszSpeak,
											  sizeof(wszSpeak) / sizeof(WCHAR) ) * sizeof(WCHAR);
	data.pData = wszSpeak;
	gpITTSCentral->TextData ( CHARSET_TEXT, 0, data, NULL, IID_ITTSBufNotifySinkW );
	// speech end
*/
	if (!speechSynthesis) {
		if (gmsgSayText == 0)
			gmsgSayText = REG_USER_MSG( "SayText", -1 );
		
		char sayText[256];
		strcpy( sayText, STRING(speaker->v.netname) );
		strcat( sayText, ": " );
		strcat( sayText, msg );

		MESSAGE_BEGIN( MSG_ALL, gmsgSayText );
		WRITE_BYTE( 1 );
		WRITE_STRING( sayText );
		MESSAGE_END();

		if (IS_DEDICATED_SERVER()) printf( "%s\n", sayText );
	}
	else {
		debugMsg( "Speaking ", msg, "\n" );
		pfnEmitSound( speaker, CHAN_VOICE, msg, 1.0, ATTN_IDLE, 0, RANDOM_LONG( 90, 120 ) );
	}
}




PB_Chat::PB_Chat()
{
	nextChatTime = 0;
	nextSpeaker = 0;
	chatFileLoaded = false;
	speechSynthesis = false;
}


PB_Chat::~PB_Chat()
{
	free();
}


bool PB_Chat::load( char *chatFile )
{
	char str[256];
	ChatList *currentCodeBlock;

	FILE *file = fopen( chatFile, "rt" );
	if (!file) {
		errorMsg( "Missing ", chatFile );
		return false;
	}

	if (chatFileLoaded) free();

	infoMsg( "Reading ", chatFile, "... " );
	currentCodeBlock = 0;
		
	while (!feof(file)) {
		fscanf( file, "%1s", str );			// read first char
		if (feof(file)) break;

		while (str[0]=='#') {				// Comments:
			fscanf( file, "%[^\n]", str );	//   read entire line
			fscanf( file, "%1s", str );		//   read first char
		}
		if ( !feof(file) ) {
			
			if (str[0]=='@') {						// new codeblock
				fscanf( file, "%[a-zA-Z_]", str);
					 if ( _stricmp( str, "USE_SPEECH_SYNTHESIS" ) == 0 ) speechSynthesis = true;
				else if ( _stricmp( str, "GOT_KILLED"    ) == 0 ) currentCodeBlock = &chatGotKilled;
				else if ( _stricmp( str, "KILLED_PLAYER" ) == 0 ) currentCodeBlock = &chatKilledPlayer;
				else if ( _stricmp( str, "GOT_WEAPON"    ) == 0 ) currentCodeBlock = &chatGotWeapon;
				else if ( _stricmp( str, "JOINED_SERVER" ) == 0 ) currentCodeBlock = &chatJoin;
				else if ( _stricmp( str, "REPLY_UNKNOWN" ) == 0 ) currentCodeBlock = &chatReplyUnknown;
				else if ( _stricmp( str, "REPLY"         ) == 0 ) {
					fscanf( file, " \"%[^\"]\" ", str );
					str[MAX_CODE_LEN-1] = 0;			// make sure that codeword isn't too long
					_strupr( str );						// convert to upper case
					ReplyList *newReplyList = new ReplyList;
					strcpy( newReplyList->code, str );
					newReplyList->reply = new ChatList;
					chatReplies.push_back( newReplyList );
					currentCodeBlock = newReplyList->reply;
					bool moreKeywords = true;
					do {
						fscanf( file, "%1s", str );
						if (str[0]==',') {				// next keyword following?
							fscanf( file, " \"%[^\"]\" ", str );
							ReplyList *addReplyList = new ReplyList;
							strcpy( addReplyList->code, str );
							addReplyList->reply = currentCodeBlock;
							chatReplies.push_back( addReplyList );
						}
						else moreKeywords = false;
					} while (moreKeywords);
					fseek( file, -1, SEEK_CUR );	// reset filepointer
				}
			}
			else {
				fseek( file, -1, SEEK_CUR );	// reset filepointer
				fscanf( file, "%[^\n]", str);	// read entire line
				if (speechSynthesis) strcat( str, ".wav" );
				PB_ChatMessage cm;
				cm.text = new char[strlen(str)+1];  
				strcpy( cm.text, str );  
				cm.time = -1000; 
				currentCodeBlock->push_back( cm );	// add to current block
			}
			
		}
	}
	fclose(	file );
	chatFileLoaded = true;
	infoMsg( "OK!\n" );
	return true;
}


bool PB_Chat::free()
// free all memory allocated for botchat
{
	if (!chatFileLoaded) return false;

	int i;

	for (i=0; i<chatGotKilled.size();	 i++ ) delete chatGotKilled[i].text;		
	chatGotKilled.clear();
	for (i=0; i<chatKilledPlayer.size(); i++ ) delete chatKilledPlayer[i].text;		
	chatKilledPlayer.clear();
	for (i=0; i<chatGotWeapon.size();	 i++ ) delete chatGotWeapon[i].text;		
	chatGotWeapon.clear();
	for (i=0; i<chatJoin.size();		 i++ ) delete chatJoin[i].text;				
	chatJoin.clear();
	for (i=0; i<chatReplyUnknown.size(); i++ ) delete chatReplyUnknown[i].text;		
	chatReplyUnknown.clear();
	
	for (int r=0; r<chatReplies.size(); r++ ) {
		for (i=0; i<chatReplies[r]->reply->size(); i++ ) delete ((*chatReplies[r]->reply)[i]).text;
		chatReplies[r]->reply->clear();
		delete chatReplies[r];
	}
	chatReplies.clear();

	chatFileLoaded = false;
	return true;
}


PB_ChatMessage* PB_Chat::getMessageFromList( ChatList &clist, bool forceReply )
{
	if (clist.size() > 0 ) {
		int c = 0;
		bool msgFound = false;
		int msgNr;
		do {
			c++;
			msgNr = RANDOM_LONG( 0, (clist.size()-1) );
			if ( worldTime() > (clist[msgNr].time+600.0) ||
				 worldTime() < clist[msgNr].time		 ||
				 forceReply									) msgFound = true;
		} while (c<3 && !msgFound);
		if (msgFound) return &(clist[msgNr]);
	}
	return 0;
}


edict_t* PB_Chat::findNameInMessage( char *msg, bool forceReply )
// looks if one of the playernames appears in msg and returns the correponding edict
// or 0 if msg doesn't contain any playernames
{
	char name1[36], name2[36];

	for (int i=0; i<32; i++) if (bots[i].is_used) {

		strcpy( name1, STRING( bots[i].pEdict->v.netname ) );
		_strupr( name1 );
		char *firstName = &name1[0];

		char *clanTagStart = strchr( firstName, '[' );
		if (clanTagStart) {
			char *clanTagEnd = strchr( firstName, ']' );
			if ( (clanTagEnd-clanTagStart) < 5	&&
			 strlen(firstName) > 5			) firstName = clanTagEnd+1;
		}
		char *space = strchr( firstName, ' ' );
		if (space) {
			*space = 0;						// shorten first part
			strcpy( name2, (space+1) );		// store second part
		}
		// check if one of the two parts appears in msg and return botedict
		if ( strstr( msg, firstName ) || (space && strstr( msg, name2 )) ) {
			int chatRate = pbConfig.personality( bots[i].personality ).communication;
			int rand = RANDOM_LONG( 1, PERSONAL_REPLY_CHAT );
			if ( (rand < chatRate) || forceReply ) return bots[i].pEdict;
		}
	}
	return 0;
}


char* PB_Chat::checkMessageForWeapon( const char *msg, const char *wpnName, edict_t *wpnOwner )
// checks if msg contains %w and replaces it by the weapon wpnOwner is carrying
{
	
	char usedWpn[32];

	char *wpnPos = strstr( msg, "%w" );
	if (!wpnPos) {
		strcpy( weaponMsg, msg );
		return &weaponMsg[0];
	}
	
	if (strncmp( wpnName, "weapon_", 7 ) == 0) strcpy( usedWpn, wpnName+7 );
	else {
		// TODO: check for tripmines!
		int owner = ENTINDEX( wpnOwner ) - 1;
		strcpy( usedWpn, getWeaponName( clientWeapon[owner] ) );
	}

	char *namePos = strstr( msg, "%s" );

	if (!namePos) {
		wpnPos[1] = 's';
		sprintf( weaponMsg, msg, usedWpn );
		wpnPos[1] = 'w';
	}
	else {
		namePos[0] = '!';
		wpnPos[1] = 's';
		sprintf( weaponMsg, msg, usedWpn );
		wpnPos[1] = 'w';
		namePos[0] = '%';
		namePos = strstr( weaponMsg, "!s" );
		namePos[0] = '%';
	}

	return &weaponMsg[0];
}


const char* PB_Chat::getName( edict_t *player )
{
	strcpy( nameBuffer, STRING( player->v.netname ) );
	char *fullName = &nameBuffer[0];

	char *clanTagStart = strchr( fullName, '[' );
	if (clanTagStart) {
		char *clanTagEnd = strchr( fullName, ']' );
		int remove = RANDOM_LONG( 0, 2 );		// remove clantag in 2/3 of all cases
		if ( (clanTagEnd-clanTagStart) < 5	&&
			 strlen(fullName) > 5			&&  
			 remove	> 0							) fullName = clanTagEnd+1;
	}
	char *space = strchr( fullName, ' ' );
	if (space) {
		int rand = RANDOM_LONG( 0, 2 );
		if (rand==1) {  *space = 0;  return fullName;  }	// return first part
		else if (rand==2) { return (space+1); }				// return second part
	}
	return fullName;
}


edict_t* PB_Chat::getRandomResponder( edict_t *excluding, bool forceReply )
// choses a random bot to respond, but not the excluding one
{
	int numCandidates = 0;
	int replyCandidate[32];

	for (int i=0; i<32; i++) if (bots[i].is_used && bots[i].pEdict!=excluding) {
		int chatRate = pbConfig.personality( bots[i].personality ).communication;
		int rand = RANDOM_LONG( 1, REPLY_CHAT );
		if ( (rand < chatRate) || forceReply ) {
			replyCandidate[numCandidates] = i;
			numCandidates++;
		}
	}
	if (numCandidates==0) return 0;
	
	int chosen = RANDOM_LONG( 0, numCandidates-1 );
	return bots[replyCandidate[chosen]].pEdict;
}


void PB_Chat::suggestMessage( edict_t *speaker, PB_ChatMessage *msg, edict_t *objective, char *realText )
// realText (if supplied) is the modified msg
{
	char *realMsg;
	if (realText) realMsg = realText;
	else		  realMsg = msg->text;
	char suggestedMsg[256];
	if (objective) sprintf( suggestedMsg, realMsg, getName( objective ) );
	else strcpy( suggestedMsg, realMsg );
	float suggestedTime = worldTime() + 1.0 + strlen( suggestedMsg )*0.2;
	
	if (suggestedTime<nextChatTime || nextChatTime==0) {
		strcpy( nextChatMessage, suggestedMsg );
		nextChatMsgPtr = msg;
		nextChatTime = suggestedTime;
		nextSpeaker = speaker;
	}
}


void PB_Chat::parseMessage( edict_t *speaker, char *msg )
// analyze the message the speaker says
{
	if ( speaker == 0 || msg == 0 ) return;

	if (pbConfig.onChatLog()) {
		FILE *fp=fopen( "parabot/chatlog.txt", "a" ); 
		if (!FBitSet( speaker->v.flags, FL_FAKECLIENT)) fprintf( fp, "[HUMAN]" );
		fprintf( fp, STRING( speaker->v.netname ) ); 
		fprintf( fp, ": " );
		fprintf( fp, msg ); 
		fprintf( fp, "\n" );
		fclose( fp );
	}
	if (!pbConfig.usingChat()) return;
	// delete old messages from queue:
	nextChatTime = 0;

	// build parseBuffer
	char parseBuffer[256];
	strcpy( parseBuffer, ": " );
	strcat( parseBuffer, msg );
	strcat( parseBuffer, " " );
	_strupr( parseBuffer );	// convert to upper case

	// replace special characters
	int len = strlen( parseBuffer );
	int pos;
	while ( (pos=strcspn( parseBuffer, ",;-'()/.!?" )) < len ) parseBuffer[pos]=' ';

	// find keyword
	char *wordFound = 0;
	for (int i=0; i<chatReplies.size(); i++) {
		wordFound = strstr( parseBuffer, chatReplies[i]->code );
		if (wordFound) break;
	}
	// check if this has to be answered in any case:
	bool forceAnswer = false;
	if (pbConfig.onAlwaysRespond() && !FBitSet( speaker->v.flags, FL_FAKECLIENT )) forceAnswer = true;
	// find answer:
	PB_ChatMessage *answer;
	if (wordFound) answer = getMessageFromList( *(chatReplies[i]->reply), forceAnswer );
	else		   answer = getMessageFromList( chatReplyUnknown, forceAnswer );
	// find someone to answer:
	if (answer) {
		edict_t *responder = findNameInMessage( parseBuffer, forceAnswer );		
		if (!responder) responder = getRandomResponder( speaker, forceAnswer );
		// suggest message
		if (responder) suggestMessage( responder, answer, speaker );
	}
}


void PB_Chat::check()
// checks if the next chat message should get displayed
{
	if (!pbConfig.usingChat()) return;

	if (nextChatTime > 0) {
		if (worldTime() > nextChatTime) {
			if (playerExists( nextSpeaker )) {	// make sure player hasn't left the game
				botChatMessage( nextSpeaker, nextChatMessage, speechSynthesis );
				nextChatMsgPtr->time = worldTime();				// store time
				parseMessage( nextSpeaker, nextChatMessage );	// send message to others
			}
		}
		else if (nextChatTime > (worldTime() + 30.0)) nextChatTime=0;	// mapchange...
	}
}


void PB_Chat::registerGotKilled( edict_t *victim, edict_t *killer, const char *wpnName )
{
	if (!pbConfig.usingChat()) return;

	if ( chatGotKilled.size() > 0 ) {
		bot_t *bot = UTIL_GetBotPointer( victim );
		if ( bot == 0 ) return;
		int chatRate = pbConfig.personality( bot->personality ).communication;
		int rand = RANDOM_LONG( 1, GOT_KILLED_CHAT );
		if ( rand < chatRate ) {
			PB_ChatMessage *msg = getMessageFromList( chatGotKilled );
			if (msg) {
				char *text = checkMessageForWeapon( msg->text, wpnName, killer );
				suggestMessage( victim, msg, killer, text );
			}
		}
	}
}


void PB_Chat::registerKilledPlayer( edict_t *victim, edict_t *killer, const char *wpnName )
{
	if (!pbConfig.usingChat()) return;
	
	if ( chatKilledPlayer.size() > 0 ) {
		bot_t *bot = UTIL_GetBotPointer( killer );
		if ( bot == 0 ) return;
		int chatRate = pbConfig.personality( bot->personality ).communication;
		int rand = RANDOM_LONG( 1, KILLED_CHAT );
		if ( rand < chatRate ) {
			PB_ChatMessage *msg = getMessageFromList( chatKilledPlayer );
			if (msg) {
				char *text = checkMessageForWeapon( msg->text, wpnName, killer );
				suggestMessage( killer, msg, victim, text );
			}
		}
	}
}


void PB_Chat::registerGotWeapon( edict_t *finder, const char *wpnName )
{
	if ( !pbConfig.usingChat() || !FBitSet( finder->v.flags, FL_FAKECLIENT) ) return;

	bot_t *bot = UTIL_GetBotPointer( finder );
	if ( bot == 0 ) return;
	int chatRate = pbConfig.personality( bot->personality ).communication;
	int rand = RANDOM_LONG( 1, WEAPON_CHAT );
	if ( rand < chatRate ) {
		PB_ChatMessage *msg = getMessageFromList( chatGotWeapon );
		if (msg) {
			char *text = checkMessageForWeapon( msg->text, wpnName, finder );
			suggestMessage( finder, msg, 0, text );
		}
	}
}


void PB_Chat::registerJoin( edict_t *joiner )
{
	if ( !pbConfig.usingChat() || !FBitSet( joiner->v.flags, FL_FAKECLIENT) ) return;
	
	if (worldTime() < 30.0) return;	// don't greet at mapstart (just respawn, no new connect)

	bot_t *bot = UTIL_GetBotPointer( joiner );
	if ( bot == 0 ) return;
	int chatRate = pbConfig.personality( bot->personality ).communication;
	int rand = RANDOM_LONG( 1, JOIN_CHAT );
	if ( rand < chatRate ) {
		PB_ChatMessage *msg = getMessageFromList( chatJoin );
		if (msg) suggestMessage( joiner, msg, 0, 0 );
	}
}