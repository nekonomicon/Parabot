//
// HPB_bot - botman's High Ping Bastard bot
//
// (http://planethalflife.com/botman/)
//
// bot_func.h
//

#ifndef BOT_FUNC_H
#define BOT_FUNC_H


//prototypes of bot functions...

void BotSpawnInit( bot_t *pBot );
void BotCreate( int fixedPersNr = -1 );
void BotStartGame( bot_t *pBot );
void BotFindItem( bot_t *pBot );
void BotThink( bot_t *pBot );


void BotFixIdealPitch( EDICT *pEdict );
float BotChangePitch( bot_t *pBot, float speed );
void BotFixIdealYaw( EDICT *pEdict );
float BotChangeYaw( bot_t *pBot, float speed );

void BotShootAtEnemy( bot_t *pBot );


#endif // BOT_FUNC_H

