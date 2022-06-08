# use of function

source = $(wildcard audio_test/*.c)   $(wildcard libaudio/components/audio_pipeline/*.c)  \
$(wildcard libaudio/components/audio_sal/*.c)  $(wildcard libaudio/components/audio_stream/*.c)  $(wildcard libaudio/components/eswin-adf-libs/eswin_codec/lib/codec/*.c)   \
$(wildcard libaudio/components/eswin-adf-libs/eswin_codec/lib/processing/*.c)  $(wildcard libaudio/idf_components/log/*.c)  \
$(wildcard libaudio/components/eswin-adf-libs/eswin_codec/lib/codec/soft_decoder/mp3/src/*.c) \
$(wildcard libaudio/idf_components/esp_http_client/*.c)

object = $(if $(source),$(patsubst %.c,%.o,$(source)),$(error "no source file"))

CC = gcc
BIN = pipeline_test

CFLGAS = -I libaudio/components/audio_sal/include \
         -I libaudio/components/audio_pipeline/include \
		 -I libaudio/components/audio_stream/include \
         -I libaudio/components/eswin-adf-libs/eswin_codec/include/codec \
		 -I libaudio/components/eswin-adf-libs/eswin_codec/include/processing \
         -I libaudio/idf_components/esp_common/include \
		 -I libaudio/idf_components/log/include \
         -I libaudio/components/eswin-adf-libs/eswin_codec/lib/codec/soft_decoder/mp3/include \
		 -I libaudio/idf_components/esp_http_client/include


LD = gcc
all: $(BIN)
.PHONY: clean

LIBS = -lc -lpthread -lrt -lXext -lz -ltinyalsa -ldl

$(BIN): $(object) 
	@ $(LD) $^ $(LIBPATH) -o $@  $(LIBS)

%.o: %.c
	@ $(CC) -g -c $< -o $@ $(CFLGAS)

clean:
	rm $(object) $(BIN)

