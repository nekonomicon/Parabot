#ifndef PB_CHAT_H
#define PB_CHAT_H


#include <vector>
#include "pb_global.h"


class PB_ChatMessage 
{

public:

	char		*text;
	float		time;

	bool operator==(const PB_ChatMessage& O) const  {  return text == O.text; }
	bool operator<(const PB_ChatMessage& O) const   {  return text <  O.text; }
};




class PB_Chat
{

// chat frequencies (lower value -> higher frequency)
#define KILLED_CHAT			40
#define GOT_KILLED_CHAT		40
#define WEAPON_CHAT			40
#define JOIN_CHAT			20
#define REPLY_CHAT			20
#define PERSONAL_REPLY_CHAT	10

// max. chars of codeword for replies
#define MAX_CODE_LEN 32


typedef std::vector<PB_ChatMessage> ChatList;

typedef struct {
	char		code[MAX_CODE_LEN];
	ChatList	*reply;
} ReplyList;


public:

	PB_Chat();
	~PB_Chat();

	bool load( char *chatFile );
	bool free();

	void registerGotKilled( edict_t *victim, edict_t *killer, const char *wpnName );

	void registerKilledPlayer( edict_t *victim, edict_t *killer, const char *wpnName );

	void registerGotWeapon( edict_t *finder, const char *wpnName );

	void registerJoin( edict_t *joiner );

	void parseMessage( edict_t *pEntity, char *msg );

	void check();
	// checks if the next chat message should get displayed


protected:

	PB_ChatMessage* getMessageFromList( ChatList &clist, bool forceReply = false );
	edict_t* findNameInMessage( char *msg, bool forceReply );
	char* checkMessageForWeapon( const char *msg, const char *wpnName, edict_t *wpnOwner );
	const char* getName( edict_t *player );
	edict_t* getRandomResponder( edict_t *excluding, bool forceReply );
	void suggestMessage( edict_t *speaker, PB_ChatMessage *msg, edict_t *objective=0, char *realText=0 );


	std::vector<ReplyList*> chatReplies;
	ChatList chatGotKilled, chatKilledPlayer, chatGotWeapon, chatJoin, chatReplyUnknown;

	float nextChatTime;
	char  nextChatMessage[256];
	PB_ChatMessage *nextChatMsgPtr;
	edict_t *nextSpeaker;

	bool chatFileLoaded;
	bool speechSynthesis;

	char weaponMsg[256];
	char nameBuffer[256];

};



#endif