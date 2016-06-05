#ifndef PERKS_H
#define PERKS_H

#define PERK_INTERVAL_IN_SECONDS 40
#define PERK_INTERVAL_IN_FRAMES  PERK_INTERVAL_IN_SECONDS * FPS

#define PERK_DURATION_ON_SCREEN_IN_SECONDS 20
#define PERK_DURATION_ON_SCREEN_IN_FRAMES  PERK_DURATION_ON_SCREEN_IN_SECONDS * FPS
#define PERK_DURATION_ON_PLAYER_IN_SECONDS 20
#define PERK_DURATION_ON_PLAYER_IN_FRAMES  PERK_DURATION_ON_PLAYER_IN_SECONDS * FPS

typedef enum Perk {
    PERK_POWER_INVINCIBILITY,
    PERK_POWER_LEVITATION,
    PERK_POWER_LOW_GRAVITY,
    PERK_POWER_SUPER_JUMP,
    PERK_POWER_TIME_STOP,
    PERK_BONUS_EXTRA_POINTS,
    PERK_BONUS_EXTRA_LIFE,
    PERK_COUNT,
    PERK_NONE
} Perk;

char *get_perk_symbol(void);

Perk get_random_perk(void);

int is_bonus_perk(Perk perk);

char *get_perk_name(Perk perk);

# endif
