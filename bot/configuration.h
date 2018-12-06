#pragma once
#if !defined(PB_CONFIGURATION_H)
#define PB_CONFIGURATION_H

#include "bot.h"
#include "pb_global.h"
#include <stdio.h>

bool		 configuration_init(const char *configPath);
bool		 configuration_create(const char *configFile);
int		 configuration_minskill();
int		 configuration_maxskill();
int		 configuration_numbots();
int		 configuration_minbots();
int		 configuration_maxbots();
float		 configuration_staytime();
const char	*configuration_chatfile();
bool		 configuration_usingchat();
bool		 configuration_onalwaysrespond();
bool		 configuration_onservermode();
bool		 configuration_onpeacemode();
bool		 configuration_onrestrictedweaponmode();
bool		 configuration_ontouringmode();
bool		 configuration_onchatlog();
void		 configuration_setfloatconvar(const char *name, float value = 0, int max = 1, int min = 0);
#endif
