#pragma once
#include <stdlib.h>
#include <stdint.h>
#include "hardware/spi.h"
#include "hardware/gpio.h"
#include <string.h>


#define PICO7219_INTENSITY_REG 0x0A
#define PICO7219_SHUTDOWN_REG 0x0C
#define PICO7219_ROWS 8
#define PICO7219_COLS 8

struct Pico7219 {
    spi_inst_t* spi;
    int32_t baud;
    uint8_t mosi;
    uint8_t sck;
    uint8_t cs;
    uint8_t *vrow_data;
    uint8_t data[PICO7219_ROWS];
    uint8_t dirty_row [PICO7219_ROWS]; 
};

struct Pico7219 * pico7219_create (
    uint8_t spi_num,
    int32_t baud,
    uint8_t mosi,
    uint8_t sck,
    uint8_t cs);

static void pico7219_write_word(const struct Pico7219 *this, uint8_t msb, uint8_t lsb);
static void pico7219_init(const struct Pico7219 *this);
void pico7219_destory( struct Pico7219 * this);
static void pico7219_set_sc(const struct Pico7219 *this, uint8_t val);
void pico7219_set_intensity (struct Pico7219 *this, uint8_t intensity);
void pico7219_set_vrow(struct Pico7219 *this);
void pico7219_flush( struct Pico7219 *this);
void pico7219_set_row_bits (const struct Pico7219 *self, uint8_t row, const uint8_t bits) ;
void pico7219_switch_off_all (struct Pico7219 *this, bool flush);
void pico7219_set_on_row (struct Pico7219 *this, uint8_t row, uint8_t byte, bool flush);
void pico7219_switch_off_row (struct Pico7219 *this, uint8_t row, bool flush);