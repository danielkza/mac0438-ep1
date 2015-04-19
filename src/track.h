#ifndef TRACK_H
#define TRACK_H

typedef struct {
    int *positions;
    int length;
    int laps;
} track;

typedef struct {
    int occupied;
    int occupied_ready;
    int *cyclists;
} track_pos;

#endif // TRACK_H
