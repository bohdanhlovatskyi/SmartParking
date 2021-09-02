#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <ctype.h>
#include <stdint.h>
#include <math.h>
#include <stdbool.h>

enum direction{forward=0, left, right};

typedef struct {
    uint8_t parking_lot_id, x, y;
    float distance_to_entry;
    int taken;
    enum direction dir;
} ParkingLot;

typedef struct {
    int x, y;
} Entry;

const uint8_t x_coords[256] = {0};
const uint8_t y_coords[256] = {0};
const uint8_t directions[256] = {0};

ParkingLot *lots[256];
Entry entry = {-1, -1};

void init_db(void);
double compute_distance(uint8_t x, uint8_t y);

int main(void) {
    // that is 16 bytes (4 bytes for each unsigned long)
    ParkingLot pl = {0};

    pl.parking_lot_id = 2;
    pl.x = 1;
    pl.y = 1;
    pl.dir = left;

    printf("%lu\n", sizeof(pl));
    printf("%lu\n", sizeof(&pl));
    printf("%lu\n", sizeof(left));
    printf("Dir: %d\n", pl.dir);

    // 1. Via UART put into SFLASH (three rows) the x, y and directions from the entry
    // 2. Via UART put into SFLASH (the same as id) the coordinated of the entry
    // 3. init_db
    // 4. sort the parking lots basing on their distance to the entry 
    // 5. somehow delete and add them in the proper place, basing on the fact whether they are taken or not
    // 6. keep track of empty places
    // 7. dynamically change three nearest places (maybe one, though it is definetely bad practice.
    // However, if we combine this with the fact that a car has entered a parking space, we can show different
    // parking spaces to everyone)

    /*

    |   num   |   | < | -  | 
    |  taken  | / |  all   |    

    */
}

void init_db(void) {
    for (int i = 0; i < 256; i++) {
        lots[i]->parking_lot_id = i;
        lots[i]->x = x_coords[i];
        lots[i]->y = y_coords[i];
        lots[i]->dir = directions[i];
        lots[i]->distance_to_entry = compute_distance(x_coords[i], y_coords[i]);
        lots[i]->taken = 0;
    }
}

double compute_distance(uint8_t x, uint8_t y) {
    if (entry.x == -1) {
        fprintf(stderr, "Entry is not initialised\n");
        exit(EXIT_FAILURE);
    }

    double square_sum = pow(x - entry.x, 2) + pow(y - entry.y, 2);

    return sqrt(square_sum);
}
