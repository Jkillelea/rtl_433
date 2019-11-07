#include "decoder.h"
#include <math.h>

// Thermistor consts
static const double R1  = 18e3;
static const double T25 = 273.15 + 25.0;
static const double R25 = 10e3;
static const double B25 = 3435.0;


static int thermistor_decode_callback(r_device *decoder, bitbuffer_t *bitbuffer) {
    data_t  *data;
    uint8_t *b;
    int      id;
    int      len;
    int      payload;

    // bitbuffer_print(bitbuffer);

    if (bitbuffer->bits_per_row[0] < 65)
        return 0;
    if (bitbuffer->bits_per_row[0] > 70)
        return 0;

    b = bitbuffer->bb[0];

    id  = (b[0] << 8*3) | (b[1] << 8*2) | (b[2] << 8*1) | (b[3]);
    len = b[4];

    // Sent LSBfirst
    payload = (b[5] << 8*0) | (b[6] << 8*1) | (b[7] << 8*2) | (b[8] << 8*3);

    // fraction of VDD
    double vfrac = ((double) payload) / 1023.0;

    // out of bounds voltage reading from ADC -> something garbeled
    if (vfrac < 0 || 1 < vfrac)
	    return 0;

    // Thermistor resistance
    double Rth = -(vfrac*R1)/(vfrac-1);
    // Thermistor characteristic equation
    double temperature = 1/(1/T25 + log(Rth/R25)/B25) - 273.15;

    data = data_make(
            "model", "",   DATA_STRING, _X("Custom Thermistor", "Custom Thermistor"),
            "id",   "ID (16bit)",       DATA_FORMAT, "0x%x", DATA_INT,    id,
            "len",  "Len (8bit)",       DATA_FORMAT, "0x%x", DATA_INT,    len,
            "data", "Raw Data (32bit)", DATA_FORMAT, "%d",   DATA_INT,    payload,
            "data", "temperature_C",    DATA_FORMAT, "%f",   DATA_DOUBLE, temperature,
            // "data",  "vfrac (32bit)", DATA_FORMAT, "%f",   DATA_DOUBLE, vfrac,
            NULL);

    decoder_output_data(decoder, data);
    return 1;
}

static char *output_fields[] = {
    "reading",
    NULL
};

r_device custom_thermistor = {
    .name        = "Custom Thermistor",
    // On Off Keying, Pulse Code Modulated, Return To Zero
    .modulation  = OOK_PULSE_PCM_RZ,
    .short_width = 1000, // 1000 us = 1 ms = 1kBaud
    .long_width  = 1000,
    .reset_limit = 10000,
    .sync_width  = 0,
    .tolerance   = 100, // us
    .decode_fn   = &thermistor_decode_callback,
    .disabled    = 0,
    .fields      = output_fields,
};
