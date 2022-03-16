#include "pico7219.h"

struct Pico7219 * pico7219_create (
    uint8_t spi_num,
    int32_t baud,
    uint8_t mosi,
    uint8_t sck,
    uint8_t cs)
{
    struct Pico7219 *this = malloc(sizeof (struct Pico7219));
    if (this) {
        this->baud = baud;
        this->mosi = mosi;
        this->sck = sck;
        this->cs = cs;
        this->vrow_data = NULL;

        pico7219_set_vrow(this);
        memset(this->data, 0, sizeof (this->data));
        // Set all data clean
        memset (this->dirty_row, 0, sizeof (this->dirty_row));

        switch (spi_num) {
            case 1:
                this->spi = spi1;
            case 0:
            default:
            this->spi = spi0;
        }

        spi_init (this->spi, baud); 

        gpio_set_function(mosi, GPIO_FUNC_SPI);
        gpio_set_function(sck, GPIO_FUNC_SPI);

        gpio_init (this->cs);
        gpio_set_dir (this->cs, GPIO_OUT);
        gpio_put (this->cs, 1);

        pico7219_init(this);
    }

    return this;
}
static void pico7219_write_word(const struct Pico7219 *this, uint8_t msb, uint8_t lsb)
{
    pico7219_set_sc(this,0);
    uint8_t buf[] = {msb, lsb};
    spi_write_blocking (this->spi, buf, 2);
    pico7219_set_sc(this,1);
}

static void pico7219_init(const struct Pico7219 *this)
{
    //rows
    for (int r = 0; r<=8 ; r++) {
        pico7219_write_word(this, 0x00|r, 0x00);
    }
    // Control register
    pico7219_write_word(this, 0x09, 0x00);
    pico7219_write_word(this, PICO7219_INTENSITY_REG, 0x01);
    pico7219_write_word(this, 0x0b, 0x07);
    pico7219_write_word(this, PICO7219_SHUTDOWN_REG, 0x01);
    pico7219_write_word(this, 0x0f, 0x00);

} 

void pico7219_destory( struct Pico7219 * this)
{
    if(this) {
        spi_deinit (this->spi);
        if(this->vrow_data)
            free(this->vrow_data);
        this->vrow_data = NULL;
        free(this);
        this = NULL;
    }
}

static void pico7219_set_sc(const struct Pico7219 *this, uint8_t val) {
    asm volatile("nop \n nop \n nop");
    gpio_put (this->cs, val);  
    asm volatile("nop \n nop \n nop");
}

void pico7219_set_intensity (struct Pico7219 *this, uint8_t intensity)
{
  pico7219_write_word(this, PICO7219_INTENSITY_REG, intensity); 
}

void pico7219_set_vrow(struct Pico7219 *this)
{
    if(this->vrow_data) {
        memset(this->vrow_data, 0x0, PICO7219_ROWS);
        return;
    }
    this->vrow_data = calloc(PICO7219_ROWS, sizeof(uint8_t));
}

void pico7219_flush( struct Pico7219 *this)
{
    memcpy(this->data, this->vrow_data, sizeof(this->data));
    for (int i = 0; i < PICO7219_ROWS; i++) {
        if(this->dirty_row[i])
            pico7219_set_row_bits (this, i, this->data[i]);
        this->dirty_row[i] = 0;
    }
    pico7219_set_vrow(this);
}

void pico7219_set_row_bits (const struct Pico7219 *this, uint8_t row, const uint8_t bits) 
{
  pico7219_write_word(this, row + 1, bits);
}

void pico7219_switch_off_all (struct Pico7219 *this, bool flush)
{
    for (int i = 0; i < PICO7219_ROWS; i++)
        memset(this->vrow_data, 0x0, PICO7219_COLS);
    memset(this->dirty_row, 1, PICO7219_ROWS);
    if (flush) pico7219_flush (this);
}

void pico7219_set_on_row (struct Pico7219 *this, uint8_t row, uint8_t byte, bool flush)
{
    if(row <= PICO7219_ROWS) {
        this->vrow_data[(int)row] |= byte;
        this->dirty_row[(int)row] = 1;
        if (flush) pico7219_flush (this);
    }
}

void pico7219_switch_off_row (struct Pico7219 *this, uint8_t row, bool flush)
{
    if(row <= PICO7219_ROWS) {
        this->vrow_data[(int)row] = 0x00;
        this->dirty_row[(int)row] = 1;
        if (flush) pico7219_flush (this);
    }
}