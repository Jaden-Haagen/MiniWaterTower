#include <modbus.h>
#include <stdio.h>

float TANK_LEVEL = 0;

int DRAIN_VALVE = 1; // 0 = closed valve, 1 = open valve

float FillTank(float fillRate, int DRAIN_VALVE) {
    if (DRAIN_VALVE == 0){
        TANK_LEVEL += fillRate;
    }
    return TANK_LEVEL;
}


int main() {
    printf("libmodbus linked OK\n");
    return 0;
}