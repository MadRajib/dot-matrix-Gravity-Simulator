#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "hardware/gpio.h"
#include "pico7219.h"

#define MOSI 3
#define SCK 2
#define CS 5
#define SPI 0
#define BAUD (1500*1000)
#define FRAMES_PER_SECOND 25
#define SKIP_TICKS (1000/FRAMES_PER_SECOND)
#define MAX_FRAMESKIP 5

uint32_t getTickCount(void);

void update_grid(struct Pico7219 *device, int8_t r, int8_t c, int8_t val) {
    pico7219_set_on_row(device, r, (0x0100 >> (c + 1)) & val, false);
}
void render_grid(struct Pico7219 *device, float interpolation){
    pico7219_flush(device);
}

int main() {
    
    struct Pico7219 *device = pico7219_create(SPI, BAUD, MOSI, SCK, CS);
    stdio_init_all();
    pico7219_switch_off_all (device, true);
    
    float r = 0;
    float c = 0;

    uint32_t next_game_tick =  getTickCount();
    int loops;
    float interpolation;

    while (true) {
        // pico7219_set_on_row(device, row, 0x80 >> col, false);
        // pico7219_set_on_row(device, row, 0x80 >> col+1, false);
        // pico7219_set_on_row(device, row, 0x80 >> col+2, false);
        // pico7219_set_on_row(device, row, 0x80 >> col+3, true);
        // update_grid(device,r,c, 0xFF); //FF on, 00 off for a particular column
        // update_grid(device,r+1,c+1, 0xFF);
        loops = 0;
        while ( getTickCount() > next_game_tick && loops < MAX_FRAMESKIP ) {
            printf("Working\n");
            r += 1*0.5;
            c += 1*0.5;
            if(r > 8) r = 0;
            if(c > 8) c = 0;
            // update_grid(device,r,c, 0xFF);
            next_game_tick += SKIP_TICKS;
            loops++;
        }

        
        //sleep_ms(200);
        // pico7219_set_on_row(device, row, 0x00, true);
        interpolation = (float)( getTickCount() + SKIP_TICKS - next_game_tick ) / (float)( SKIP_TICKS );
        pico7219_switch_off_all(device, true);
        
        r= r + 0.0005*interpolation;
        c= c + 0.0005*interpolation;
        
        update_grid(device,r,c, 0xFF);
        render_grid(device, interpolation);
    }
}

uint32_t getTickCount() {
    return time_us_64()/1000;
}

