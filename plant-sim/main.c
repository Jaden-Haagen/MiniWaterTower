#include <stdio.h>
#include <stdbool.h>
#include <signal.h>
#include <unistd.h>
#include <modbus.h>
#include <errno.h>

//value for checking if program needs to end
volatile sig_atomic_t run = 1;

#define TANK_LEVEL_REGISTER 0
#define VALVE_COIL 0

//constant values for the tank
const float FILL_RATE = 0.5f;
const float DRAIN_RATE = 0.2f;
const float TANK_MIN = 0.0f;
const float TANK_MAX = 100.0f;

void end_program(int sig) {
    (void)sig; //Unused parameter
    run = 0;
}
//Calculates the level in tank based on wether the valve is open or closed.
void tankLevelCalc(float *TANK_LEVEL, bool VALVE_OPEN) {
    if (VALVE_OPEN){
        *TANK_LEVEL += FILL_RATE;
    } else {
        *TANK_LEVEL -= DRAIN_RATE;
    }
    //Ensure tank level doesn't break max and min values
    if (*TANK_LEVEL > TANK_MAX) *TANK_LEVEL = TANK_MAX;
    if (*TANK_LEVEL < TANK_MIN) *TANK_LEVEL = TANK_MIN;
}


int main() {
    signal(SIGINT, end_program); //catch CTRL + C for ending the script

    modbus_t *ctx = modbus_new_tcp("0.0.0.0", 5020); //Create a new modbus context for TCP communication
    //check if the context was created successfully
    if (ctx == NULL) {
        fprintf(stderr, "Failed to create Modbus context: %s\n", modbus_strerror(errno));
        return -1;
    }   
    modbus_mapping_t *mb_mapping = modbus_mapping_new(10,0,10,0); //Create a new modbus mapping with 10 coils and 10 holding registers
    if (mb_mapping == NULL) {
        fprintf(stderr, "Failed to allocate the mapping: %s\n", modbus_strerror(errno));
        modbus_free(ctx);
        return -1;
    }
    
    int server_socket = modbus_tcp_listen(ctx, 1); //Listen for incoming connections on the specified port
    if (server_socket == -1) {
        fprintf(stderr, "Failed to listen: %s\n", modbus_strerror(errno));
        modbus_mapping_free(mb_mapping);
        modbus_free(ctx);
        return -1;
    }

    //modbus_tcp_accept(ctx, &server_socket); //Accept the incoming connection
    if (modbus_tcp_accept(ctx, &server_socket) == -1) {
        fprintf(stderr, "Failed to accept: %s\n", modbus_strerror(errno));
        modbus_mapping_free(mb_mapping);
        modbus_free(ctx);
        return -1;
    }
    //variables for the tank level and valve state
    float TANK_LEVEL = 0;
    bool VALVE_OPEN = true;

    int millisecs = 500; //Default sleep time in milliseconds
    //loop for updating the tank level
    while (run) {
        tankLevelCalc(&TANK_LEVEL, VALVE_OPEN); //Update the tank level based on the valve state
        
        mb_mapping->tab_registers[TANK_LEVEL_REGISTER] = (uint16_t)(TANK_LEVEL * 100); //Store the tank level in the holding register (scaled by 100 to preserve 2 decimal places)
        VALVE_OPEN = mb_mapping->tab_bits[VALVE_COIL]; //Update the valve state based on coil value
        
        uint8_t query[MODBUS_TCP_MAX_ADU_LENGTH];
        int rc = modbus_receive(ctx, query);
        if (rc > 0) {
            modbus_reply(ctx, query, rc, mb_mapping);
        } else if (rc == -1) {
            printf("Client disconnected.\n");
            modbus_close(ctx);  // close this client's connection cleanly
            break; // drop back to outer loop, wait for next connection
        }

        printf("Tank level: %.2f\n", TANK_LEVEL); //Print the current level
        usleep(millisecs * 1000); //sleeps for specified length of milliseconds
    }

    printf("\nShutting down.\n");

    //Free up all resources and close the modbus connection
    modbus_mapping_free(mb_mapping);
    modbus_close(ctx);
    modbus_free(ctx);
    return 0;
}