#pragma once
#if !defined(PERSONALITIES)
#define PERSONALITIES
typedef struct personality {
	char name[BOT_NAME_LEN];
	char model[BOT_SKIN_LEN];
	int aimskill;
	int aggression;
	int sensitivity;
	int communication;
	char topcolor[4];
	char bottomcolor[4];
	bool inuse;
} PERSONALITY;

bool		 personalities_init(const char *personalityPath);
PERSONALITY	 personalities_get(int index);
const char	*personalities_gettopcolor(int index);
const char	*personalities_getbottomcolor(int index);
void		 personalities_joins(int index);
void		 personalities_leave(int index);
int		 personalities_count();
#endif // PERSONALITIES
