#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "hardware/gpio.h"
#include "pico7219.h"
#include "picoMPU6050.h"

#define MOSI 3
#define SCK 2
#define CS 5
#define SPI 0
#define BAUD (1500*1000)
#define FRAMES_PER_SECOND 25
#define SKIP_TICKS (1000/FRAMES_PER_SECOND)
#define MAX_FRAMESKIP 5

uint32_t getTickCount(void);

struct _2d {
    float x;
    float y;
};


struct dot {
 struct _2d pos;
 struct _2d vel;
 struct _2d acc;
};

struct dir {
    uint8_t axis;
    int8_t or;
};

float map(int16_t x, int16_t in_min, int16_t in_max, float out_min, float out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}


void update_grid(struct Pico7219 *device, int8_t r, int8_t c, int8_t val) {
    pico7219_set_on_row(device, r, (0x0100 >> (c + 1)) & val, false);
}
void render_grid(struct Pico7219 *device, float interpolation){
    pico7219_flush(device);
}

void get_gravity_dir(int16_t *acc_ptr, struct dir *dir){
    
    int16_t x = abs(acc_ptr[0]),y = abs(acc_ptr[1]),z = abs(acc_ptr[2]);
    if (x >= y) {
        if (x >= z)
            dir->axis = 0;
        else
            dir->axis = 2;
    }else {
        if (y >=z)
            dir->axis = 1;
        else
            dir->axis = 2;
    }
    dir->or = (acc_ptr[dir->axis] >0)?1:-1;
}

int main() {
    

    struct Pico7219 *device = pico7219_create(SPI, BAUD, MOSI, SCK, CS);
    printf("init mpu\n");
    int_mpu6050();

    stdio_init_all();
    pico7219_switch_off_all (device, true);
    
    struct dot dot;
    dot.pos.x=0;
    dot.pos.y=0;
    dot.vel.x=0;
    dot.vel.y=0;
    dot.acc.x=0;
    dot.acc.y=0;

    float acc = 0;

    int16_t acceleration[3] = {0,0,0};
    int16_t *acc_ptr = acceleration;
    struct dir *dir = malloc(sizeof(struct dir));
    dir->axis = 2;
    dir->or = 1;

    uint32_t next_game_tick =  getTickCount();
    int loops;
    float interpolation;

    int8_t prev = 0;
    while (true) {
        loops = 0;
        while ( getTickCount() > next_game_tick && loops < MAX_FRAMESKIP ) {
            start_mpu6050(acc_ptr);
            get_gravity_dir(acc_ptr, dir);
            

            dot.acc.x = map(acc_ptr[0],-20000,20000,-0.9,0.9);
            dot.acc.y = map(acc_ptr[1],-20000,20000,-0.9,0.9);

            dot.vel.x += dot.acc.x*0.001;
            dot.vel.y += dot.acc.y*0.001;


            printf("Acc. X = %d, Y = %d, Z = %d, max:%d,,accx:%f,accy:%f\n", acc_ptr[0], acc_ptr[1], acc_ptr[2], (dir->axis+1)*(dir->or), dot.acc.x,dot.acc.y);
            // if(dir->axis == 0 && dir->or > 0)
            //     dot.vel.x += 0.9*acc*0.001;
            // else if(dir->axis == 0)
            //     dot.vel.x -= 0.9*0.001;
            // else if(dir->axis == 1 && dir->or > 0)
            //     dot.vel.y += 0.9*0.001;
            // else if(dir->axis == 1)
            //     dot.vel.y -= 0.9*0.001; 
            dot.vel.x *= 0.9;
            dot.vel.y *= 0.9;
            
            // switch(dir->axis) {
            //     case 0:
            //         if(dir->or > 0) {
            //             dot.pos.x*=1;
            //             dot.pos.y*=1;
            //         } else {
            //             dot.pos.x*=-1;
            //             dot.pos.y;
            //         }
                           
            //     break;
            //     case 1:
                
            //     {
            //         float temp = dot.pos.x;
            //         dot.pos.x = dot.pos.y;
            //         dot.pos.y = temp;
            //     }
            //     if(dir->or > 0) {
            //             dot.pos.x*=1;
            //             dot.pos.y*=1;
            //         } else {
            //             dot.pos.x*=-1;
            //             dot.pos.y;
            //         }
            // }

            // update_grid(device,r,c, 0xFF);
            next_game_tick += SKIP_TICKS;
            loops++;
        }

        
        //sleep_ms(200);
        // pico7219_set_on_row(device, row, 0x00, true);
        interpolation = (float)( getTickCount() + SKIP_TICKS - next_game_tick ) / (float)( SKIP_TICKS );
        pico7219_switch_off_all(device, true);
        
        dot.pos.x = dot.pos.x + dot.vel.x*interpolation;
        dot.pos.y = dot.pos.y + dot.vel.y*interpolation;
        if(dot.pos.x > 7 ) { dot.pos.x = 7; dot.vel.x = 0;}
        if(dot.pos.y > 7 ) { dot.pos.y = 7; dot.vel.y = 0;}
        if(dot.pos.x < 0 ) { dot.pos.x = 0; dot.vel.x = 0;}
        if(dot.pos.y < 0 ) { dot.pos.y = 0; dot.vel.y = 0;}
        
        update_grid(device,dot.pos.x,dot.pos.y, 0xFF);
        render_grid(device, interpolation);
    }

    free(dir);
    dir = NULL;
}

uint32_t getTickCount() {
    return time_us_64()/1000;
}

