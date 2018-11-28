#pragma once
#if !defined(PB_CHAT_H)
#define PB_CHAT_H

#include <vector>
#include "pb_global.h"


typedef struct chatmessage {
	const char	*text;
	float		 time;
} CHATMESSAGE;




// chat frequencies (lower value -> higher frequency)
#define KILLED_CHAT			40
#define GOT_KILLED_CHAT		40
#define WEAPON_CHAT			40
#define JOIN_CHAT			20
#define REPLY_CHAT			20
#define PERSONAL_REPLY_CHAT	10

// max. chars of codeword for replies
#define MAX_CODE_LEN 32


typedef struct chatlist {
	struct chatlist		*next;
	CHATMESSAGE		 message;
} CHATLIST;

typedef struct replylist {
	struct replylist	*next;
	char			 code[MAX_CODE_LEN];
	CHATLIST		*reply;
} REPLYLIST;


bool chat_load();
bool chat_free();

void chat_registergotkilled( EDICT *victim, EDICT *killer, const char *wpnName );

void chat_registerkilledplayer( EDICT *victim, EDICT *killer, const char *wpnName );

void chat_registergotweapon( EDICT *finder, const char *wpnName );

void chat_registerjoin( EDICT *joiner );

void chat_parsemsg( EDICT *pEntity, const char *msg );

void chat_check();

// checks if the next chat message should get displayed
CHATMESSAGE* chat_getMessageFromList(CHATLIST *clist, bool forceReply = false);
EDICT *chat_findNameInMessage( const char *msg, bool forceReply );
const char* chat_checkMessageForWeapon( const char *msg, const char *wpnName, EDICT *wpnOwner );
const char* chat_getName( EDICT *player );
EDICT* chat_getRandomResponder( EDICT *excluding, bool forceReply );
void chat_suggestMessage( EDICT *speaker, CHATMESSAGE *msg, EDICT *objective=0, const char *realText=0 );

typedef struct chat {
	REPLYLIST *replies;
	CHATLIST gotkilled, killedplayer, gotweapon, join, replyunknown;
	CHATMESSAGE *nextmsgptr;
	EDICT *nextspeaker;
	char nextmessage[256];
	float nexttime;
	bool fileloaded, speechsynthesis;
} CHAT;

#endif
