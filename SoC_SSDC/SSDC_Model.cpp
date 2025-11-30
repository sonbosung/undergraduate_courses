#define _CRT_SECURE_NO_WARNINGS
#include "SSDC_header.h"

int main() {
    store_request(); // store all requests from host to pcieq with arrival_time
    global_time = sp[0].arrival_time; // global time = first task start time

    while(1){
        // debug_print();
        
        process_pcieq();
        process_sramq();
        process_dramq();
        process_fmc();

        global_time = next_global_time();
        printf("global_time = %d\n", global_time);
        if (sram_end_time <= global_time)    sram_wport_busy = false;
        if (dram_end_time <= global_time)    dram_busy = false;
        if (fmc_end_time <= global_time)     fmc_wport_busy = false;
        if (nand_end_time <= global_time)    nand_busy = false;

        // print each tasks' end times
        for (int i = 0; i < count; i++) {
            printf("#: %d | SRAM: %d | DRAM: %d | FMC: %d | NAND: %d\n", i, sp[i].sram_arrival_time
                , sp[i].dram_arrival_time, sp[i].fmc_arrival_time, sp[i].nand_arrival_time);
        }
    }
    free(sp);
    return 0;
}