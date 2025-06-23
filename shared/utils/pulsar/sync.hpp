#pragma once 

#include <Arduino.h>

void time_sync_init(); 


void time_sync_loop();

String current_iso_time();

time_t current_epoch_time();

