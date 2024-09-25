# test/Makefile
CC=gcc
TOPDIR=$(shell pwd)/..
MQTT_SRC=$(TOPDIR)/src
CFLAGS=-Wall -g -I$(MQTT_SRC)/message -I$(MQTT_SRC)/network -I$(MQTT_SRC)
LDFLAGS=-L$(MQTT_SRC)/lib -lsimple_mqtt

BUILD_DIR=build

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

msg_connect_test: msg_connect_test.c 
	$(CC) $(CFLAGS) msg_connect_test.c -o $(BUILD_DIR)/$@ $(LDFLAGS)

message_test: message_test.c
	$(CC) $(CFLAGS) message_test.c -o $(BUILD_DIR)/$@ $(LDFLAGS)

.PHONY: clean
clean:
	rm -rf $(BUILD_DIR)