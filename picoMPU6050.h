static void mpu6050_reset();

static void mpu6050_read_raw(int16_t accel[3], int16_t gyro[3], int16_t *temp);

void int_mpu6050();

void start_mpu6050();