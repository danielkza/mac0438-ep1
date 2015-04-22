#ifndef TRACK_H
#define TRACK_H

#include <stdbool.h>
#include "cycler.h"

#define MAX_CYCLERS_PER_POS 4
#define MIN_CYCLERS_FOR_CRASHES 4

typedef struct {
    int occupied; // número de ciclistas nessa posição
    int occupied_ready; // número de ciclistas nessa posição que já se moveram
    int cyclers[MAX_CYCLERS_PER_POS]; // IDs dos ciclistas, -1 nas entradas vazias
} track_pos_t;

typedef struct {
    int length;
    int lap;
    bool use_random_velocity;
    int orig_num_cyclers; // número original de ciclistas
    int num_cyclers; // número de ciclistas ainda correndo
    cycler_info *cycler_infos;
    track_pos_t *positions;
    // número de eliminações que terão que ocorrer, dado que 2N voltas já terminaram
    int waiting_for_elimination;
    // número de ciclistas que irão crashar na próxima volta
    int will_crash;
} track_t;

extern track_t *g_track;

track_t* track_new(int num_cyclers, int length, bool use_random_velocity);

void track_update_lap(track_t *track);
int track_update_eliminations(track_t *track);
void track_print_cyclers(track_t *track);
void track_print_three_last(track_t *track);
void track_print_final(track_t *track);
void track_free(track_t *old_track);

#endif // TRACK_H
