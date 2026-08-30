CC ?= cc
CXX ?= c++
CFLAGS = -std=c11 -Wall -Wextra -Werror
CXXFLAGS = -std=c++14 -Wall -Wextra -Werror
BUILD_DIR = ../build/usb
TARGETS = $(BUILD_DIR)/pcm_ring_test $(BUILD_DIR)/adaptive_playback_test $(BUILD_DIR)/audio_router_test $(BUILD_DIR)/button_gesture_test $(BUILD_DIR)/host_volume_test $(BUILD_DIR)/display_model_test $(BUILD_DIR)/oled_ssd1306_test $(BUILD_DIR)/preset_model_test $(BUILD_DIR)/preset_cbor_test $(BUILD_DIR)/preset_storage_test $(BUILD_DIR)/hid_rpc_test

.PHONY: test clean descriptor-test flash-size-test

test: $(TARGETS) descriptor-test flash-size-test
	$(BUILD_DIR)/pcm_ring_test
	$(BUILD_DIR)/adaptive_playback_test
	$(BUILD_DIR)/audio_router_test
	$(BUILD_DIR)/button_gesture_test
	$(BUILD_DIR)/host_volume_test
	$(BUILD_DIR)/display_model_test
	$(BUILD_DIR)/oled_ssd1306_test
	$(BUILD_DIR)/preset_model_test
	$(BUILD_DIR)/preset_cbor_test
	$(BUILD_DIR)/preset_storage_test
	$(BUILD_DIR)/hid_rpc_test

descriptor-test:
	./audio_descriptor_test.sh

flash-size-test:
	../check_flash_size.sh --bytes=126976 126976
	@if ../check_flash_size.sh --bytes=126977 126976 >/dev/null 2>&1; then exit 1; fi
	@$(MAKE) -C .. -f Makefile.usb-composite -n program-dfu | grep -q './check_flash_size.sh'
	@$(MAKE) -C .. -f Makefile.usb-composite -Bn build/usb_audio.o | grep -q -- ' -Os '

$(BUILD_DIR)/pcm_ring_test: pcm_ring.c pcm_ring.h pcm_ring_test.c
	mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) pcm_ring.c pcm_ring_test.c -o $@

$(BUILD_DIR)/adaptive_playback_test: adaptive_playback.c adaptive_playback.h pcm_ring.c pcm_ring.h adaptive_playback_test.c
	mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) adaptive_playback.c pcm_ring.c adaptive_playback_test.c -o $@

$(BUILD_DIR)/audio_router_test: audio_router.c audio_router.h audio_router_test.c preset_model.c preset_model.h
	mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) audio_router.c preset_model.c audio_router_test.c -lm -o $@

$(BUILD_DIR)/button_gesture_test: button_gesture.c button_gesture.h button_gesture_test.c
	mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) button_gesture.c button_gesture_test.c -o $@

$(BUILD_DIR)/host_volume_test: host_volume.c host_volume.h host_volume_test.c
	mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) host_volume.c host_volume_test.c -lm -o $@

$(BUILD_DIR)/display_model_test: display_model.c display_model.h display_model_test.c preset_model.c preset_model.h
	mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) display_model.c preset_model.c display_model_test.c -lm -o $@

$(BUILD_DIR)/oled_ssd1306_test: oled_ssd1306.c oled_ssd1306.h oled_ssd1306_test.c display_model.h
	mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) oled_ssd1306.c oled_ssd1306_test.c -o $@

$(BUILD_DIR)/preset_model_test: preset_model.c preset_model.h preset_model_test.c
	mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) preset_model.c preset_model_test.c -lm -o $@

$(BUILD_DIR)/preset_cbor_test: preset_model.c preset_model.h preset_cbor.cpp preset_cbor.h preset_cbor_test.cpp
	mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c preset_model.c -o $(BUILD_DIR)/preset_model_cpp_test.o
	$(CXX) $(CXXFLAGS) preset_cbor.cpp preset_cbor_test.cpp $(BUILD_DIR)/preset_model_cpp_test.o -o $@

$(BUILD_DIR)/preset_storage_test: preset_model.c preset_model.h preset_storage.c preset_storage.h preset_storage_test.c
	mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) preset_model.c preset_storage.c preset_storage_test.c -lm -o $@

$(BUILD_DIR)/hid_rpc_test: preset_model.c preset_model.h preset_cbor.cpp preset_cbor.h hid_rpc.cpp hid_rpc.h hid_rpc_test.cpp audio_device.h
	mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c preset_model.c -o $(BUILD_DIR)/preset_model_hid_test.o
	$(CXX) $(CXXFLAGS) preset_cbor.cpp hid_rpc.cpp hid_rpc_test.cpp $(BUILD_DIR)/preset_model_hid_test.o -o $@

clean:
	rm -f $(TARGETS)
