#include "decoder.h"

static int thermistor_decode_callback(r_device *decoder, bitbuffer_t *bitbuffer) {
    data_t *data;
    uint8_t *b;
    int id;
    int len;
    int payload;

    // puts("");
    // bitbuffer_print(bitbuffer);
    if (!((bitbuffer->bits_per_row[0] == 65) || (bitbuffer->bits_per_row[0] == 66)))
        return 0;

    b = bitbuffer->bb[0];

    id = (b[0] << 8*3) | (b[1] << 8*2) | (b[2] << 8*1) | (b[3]);
    len = b[4];

    // Sent LSBfirst
    payload = (b[5] << 8*0) | (b[6] << 8*1) | (b[7] << 8*2) | (b[8] << 8*3);

    data = data_make(
            "model",  "",             DATA_STRING, _X("Custom Thermistor", "Custom Thermistor"),
            "id",     "ID (16bit)",   DATA_FORMAT, "0x%x", DATA_INT, id,
            "len",    "Len (8bit)",   DATA_FORMAT, "0x%x", DATA_INT, len,
            "data",   "Data (32bit)", DATA_FORMAT, "%d",   DATA_INT, payload,
            NULL);

    decoder_output_data(decoder, data);
    return 1;
}

static char *output_fields[] = {
    "reading",
    NULL
};

r_device custom_thermistor = {
    .name          = "Custom Thermistor",
    .modulation    = OOK_PULSE_PCM_RZ,
    .short_width   = 1000, // 1000 us = 1 ms = 1kBaud
    .long_width    = 1000,
    .reset_limit   = 10000,
    .sync_width    = 0,
    .tolerance     = 100, // us
    .decode_fn     = &thermistor_decode_callback,
    .disabled      = 0,
    .fields        = output_fields,
};
