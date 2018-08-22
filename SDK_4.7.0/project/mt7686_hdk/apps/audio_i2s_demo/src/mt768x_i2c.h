#ifndef __MT768X_AUDIO_I2C_INC__
#define __MT768X_AUDIO_I2C_INC__


typedef enum{
    NONE_IN = 0,
    MIC_IN = 1
}MT768X_AUDIO_INPUT_MODE;

typedef enum{
    NONE_OUT = 0,
    SPK_OUT = 1,
    LINE_OUT = 2
}MT768X_AUDIO_OUTPUT_MODE;


int mt768x_audio_i2c_init(void);

int mt768x_audio_i2c_deinit(void);

int mt768x_audio_i2c_set_parameters(unsigned char isinput, unsigned short samplerate, unsigned char channelnum);

void mt768x_audio_i2c_set_mute(char enable);

void mt768x_audio_i2c_set_volume(unsigned char volume); // 0 ~ 100

unsigned char mt768x_audio_i2c_get_volume();

void mt768x_audio_i2c_set_input(MT768X_AUDIO_INPUT_MODE mode);

void mt768x_audio_i2c_set_output(MT768X_AUDIO_OUTPUT_MODE mode);


#endif
