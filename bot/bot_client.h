//
// HPB_bot - botman's High Ping Bastard bot
//
// (http://planethalflife.com/botman/)
//
// bot_client.h
//

void Client_Valve_DeathMsg(void *p, int noMatter);


void BotClient_TFC_VGUI(void *p, int bot_index);
void BotClient_CS_VGUI(void *p, int bot_index);
void BotClient_CS_ShowMenu(void *p, int bot_index);

void BotClient_Valve_WeaponList(void *p, int bot_index);
void BotClient_Holywars_WeaponList(void *p, int bot_index);
void BotClient_DMC_WeaponList(void *p, int bot_index);
void BotClient_TFC_WeaponList(void *p, int bot_index);
void BotClient_CS_WeaponList(void *p, int bot_index);
void BotClient_Gearbox_WeaponList(void *p, int bot_index);

void BotClient_Valve_CurrentWeapon(void *p, int bot_index);
void BotClient_Holywars_CurrentWeapon(void *p, int bot_index);
void BotClient_DMC_CurrentWeapon(void *p, int bot_index);
void BotClient_TFC_CurrentWeapon(void *p, int bot_index);
void BotClient_CS_CurrentWeapon(void *p, int bot_index);
void BotClient_Gearbox_CurrentWeapon(void *p, int bot_index);
void HumanClient_CurrentWeapon( void *p, int clientIndex );

void BotClient_Valve_AmmoX(void *p, int bot_index);
void BotClient_Holywars_AmmoX(void *p, int bot_index);
void BotClient_DMC_AmmoX(void *p, int bot_index);
void BotClient_TFC_AmmoX(void *p, int bot_index);
void BotClient_CS_AmmoX(void *p, int bot_index);
void BotClient_Gearbox_AmmoX(void *p, int bot_index);

void BotClient_Valve_AmmoPickup(void *p, int bot_index);
void BotClient_Holywars_AmmoPickup(void *p, int bot_index);
void BotClient_DMC_AmmoPickup(void *p, int bot_index);
void BotClient_TFC_AmmoPickup(void *p, int bot_index);
void BotClient_CS_AmmoPickup(void *p, int bot_index);
void BotClient_Gearbox_AmmoPickup(void *p, int bot_index);

void BotClient_Valve_WeaponPickup(void *p, int bot_index);
void BotClient_Holywars_WeaponPickup(void *p, int bot_index);
void BotClient_DMC_WeaponPickup(void *p, int bot_index);
void BotClient_TFC_WeaponPickup(void *p, int bot_index);
void BotClient_CS_WeaponPickup(void *p, int bot_index);
void BotClient_Gearbox_WeaponPickup(void *p, int bot_index);

void BotClient_Valve_ItemPickup(void *p, int bot_index);
void BotClient_Holywars_ItemPickup(void *p, int bot_index);
void BotClient_DMC_ItemPickup(void *p, int bot_index);
void BotClient_TFC_ItemPickup(void *p, int bot_index);
void BotClient_CS_ItemPickup(void *p, int bot_index);
void BotClient_Gearbox_ItemPickup(void *p, int bot_index);

void BotClient_Valve_Health(void *p, int bot_index);
void BotClient_Holywars_Health(void *p, int bot_index);
void BotClient_DMC_Health(void *p, int bot_index);
void BotClient_TFC_Health(void *p, int bot_index);
void BotClient_CS_Health(void *p, int bot_index);
void BotClient_Gearbox_Health(void *p, int bot_index);

void BotClient_Valve_Battery(void *p, int bot_index);
void BotClient_Holywars_Battery(void *p, int bot_index);
void BotClient_DMC_Battery(void *p, int bot_index);
void BotClient_TFC_Battery(void *p, int bot_index);
void BotClient_CS_Battery(void *p, int bot_index);
void BotClient_Gearbox_Battery(void *p, int bot_index);

void BotClient_Valve_Damage(void *p, int bot_index);
void BotClient_Holywars_Damage(void *p, int bot_index);
void BotClient_DMC_Damage(void *p, int bot_index);
void BotClient_TFC_Damage(void *p, int bot_index);
void BotClient_CS_Damage(void *p, int bot_index);
void BotClient_Gearbox_Damage(void *p, int bot_index);

void BotClient_CS_Money(void *p, int bot_index);

