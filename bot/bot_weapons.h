//
// HPB_bot - botman's High Ping Bastard bot
//
// (http://planethalflife.com/botman/)
//
// bot_weapons.h
//

#ifndef BOT_WEAPONS_H
#define BOT_WEAPONS_H



#define VALVE_MAX_NORMAL_BATTERY   100
#define VALVE_HORNET_MAX_CARRY      8




typedef struct
{
   char szClassname[64];
   int  iAmmo1;     // ammo index for primary ammo
   int  iAmmo1Max;  // max primary ammo
   int  iAmmo2;     // ammo index for secondary ammo
   int  iAmmo2Max;  // max secondary ammo
   int  iSlot;      // HUD slot (0 based)
   int  iPosition;  // slot position
   int  iId;        // weapon ID
   int  iFlags;     // flags???
} bot_weapon_t;


#endif // BOT_WEAPONS_H

