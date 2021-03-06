#ifndef PLANETIO_ENGINECONSTANTS_H
#define PLANETIO_ENGINECONSTANTS_H

#define NB_EVENTS 64

#define NB_PLAYERS 8
#define NB_NON_PLAYERS  120
#define NB_WRECKS 128
#define NB_BODIES (NB_NON_PLAYERS + NB_PLAYERS + NB_WRECKS)
#define NB_NON_WRECKS (NB_NON_PLAYERS + NB_PLAYERS)

#define PLAYER_INDEX NB_NON_PLAYERS
#define WRECKS_INDEX (NB_NON_PLAYERS + NB_PLAYERS)

#define CRITICAL_RATIO 0.1

#define G_CONSTANT 	1.0634E4
#define MIN_NONNULL 1.00E-20

#define UNIVERSE_SIZE 2000.0

#define SERVER_IP "127.0.0.1"


#endif //PLANETIO_ENGINECONSTANTS_H
