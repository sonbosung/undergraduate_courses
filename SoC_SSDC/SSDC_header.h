#pragma once
#include <stdio.h>
#include <stdlib.h>
#include "Queue.h"

FILE* fp;
struct host_request* sp;
int count = 0;
int global_time;

Queue pcieq;
Queue sramq;
Queue dramq;
Queue fmc;
Queue nand;

int first_task_arrival_time;

const int SRAM_write_delay = 4096;
const int SRAM_read_delay = 4096;
const int DRAM_write_delay = 4416;
const int DRAM_read_delay = 4800;
const int NAND_write_delay = 1402750;

int sram_end_time;
int dram_end_time;
int fmc_end_time;
int nand_end_time;

bool sram_wport_busy = false;
bool dram_busy = false;
bool fmc_wport_busy = false;
bool nand_busy = false;

struct host_request {
    int file_name;
    int write_read;
    int size_in_bytes;
    int arrival_time;
    int sram_arrival_time;
    int dram_arrival_time;
    int fmc_arrival_time;
    int nand_arrival_time;
};

int next_global_time() {
    int comparator;
    if (sram_wport_busy)
        comparator = sram_end_time;
    if (dram_busy) {
        if (!sram_wport_busy)
            comparator = dram_end_time;
        else {
            if (dram_end_time <= comparator)
                comparator = dram_end_time;
        }
    }
    if (fmc_wport_busy) {
        if (!sram_wport_busy & !dram_busy)
            comparator = fmc_end_time;
        else {
            if (fmc_end_time <= comparator)
                comparator = fmc_end_time;
        }
    }
    if (nand_busy) {
        if (!sram_wport_busy & !dram_busy & !fmc_wport_busy)
            comparator = nand_end_time;
        else {
            if (nand_end_time <= comparator)
                comparator = nand_end_time;
        }
    }
    if (!sram_wport_busy & !dram_busy & !fmc_wport_busy & !nand_busy)
        exit(0);
    return comparator;
}

void store_request() {
    int fend, fn_buffer, wr_buffer, sib_buffer, at_buffer;
    sp = 0;
    fp = fopen("host_request.txt", "r");
    InitQueue(&pcieq);
    InitQueue(&sramq);
    InitQueue(&dramq);
    InitQueue(&fmc);
    while (1) {
        fend = fscanf(fp, "%d, %d, %d, %d\n", &fn_buffer, &wr_buffer, &sib_buffer, &at_buffer);
        if (fend == EOF)
            break;
        sp = (struct host_request*)realloc(sp, sizeof(struct host_request));
        sp[count].file_name = fn_buffer;
        sp[count].write_read = wr_buffer;
        sp[count].size_in_bytes = sib_buffer;
        sp[count].arrival_time = at_buffer;
        Enqueue(&pcieq, count);

        printf("%d, %d, %d, %d\n", sp[count].file_name, sp[count].write_read, sp[count].size_in_bytes, sp[count].arrival_time);
        count++;

    }
}

void process_pcieq() {
    first_task_arrival_time = sp[pcieq.buf[pcieq.front]].arrival_time;
    if ((!IsEmpty(&pcieq)) && !sram_wport_busy) {
        if (first_task_arrival_time <= global_time) {
            Enqueue(&sramq, pcieq.buf[pcieq.front]);
            sram_wport_busy = true;
            sram_end_time = global_time + SRAM_write_delay;
            sp[pcieq.buf[pcieq.front]].sram_arrival_time = sram_end_time;
            Dequeue(&pcieq);
        }
    }
}

void process_sramq() {
    first_task_arrival_time = sp[sramq.buf[sramq.front]].sram_arrival_time;
    if ((!IsEmpty(&sramq)) && !dram_busy) {
        if (first_task_arrival_time <= global_time) {
            Enqueue(&dramq, sramq.buf[sramq.front]);
            dram_busy = true;
            dram_end_time = global_time + DRAM_write_delay;
            sp[sramq.buf[sramq.front]].dram_arrival_time = dram_end_time;
            Dequeue(&sramq);
        }
    }
}

void process_dramq() {
    first_task_arrival_time = sp[dramq.buf[dramq.front]].dram_arrival_time;
    if ((!IsEmpty(&dramq)) && !fmc_wport_busy && !dram_busy && !nand_busy) {
        if (first_task_arrival_time <= global_time) {
            Enqueue(&fmc, dramq.buf[dramq.front]);
            fmc_wport_busy = true;
            fmc_end_time = global_time + DRAM_read_delay;
            sp[dramq.buf[dramq.front]].fmc_arrival_time = fmc_end_time;
            Dequeue(&dramq);
        }
    }
}

void process_fmc() {
    first_task_arrival_time = sp[fmc.buf[fmc.front]].fmc_arrival_time;
    if ((!IsEmpty(&fmc)) && !nand_busy) {
        if (first_task_arrival_time <= global_time) {
            Enqueue(&nand, fmc.buf[fmc.front]);
            nand_busy = true;
            nand_end_time = global_time + NAND_write_delay;
            sp[fmc.buf[fmc.front]].nand_arrival_time = nand_end_time;
            Dequeue(&fmc);
        }
    }
}
void debug_print() {
    printf("-------------------------------------\n");
    printf("status: %d %d %d %d\n", sram_wport_busy, dram_busy, fmc_wport_busy, nand_busy);
    printf("pcieq_first_task = %d\n", pcieq.buf[pcieq.front]);
    printf("pcieq_bytes = %d\n", Qsize(&pcieq));
    printf("sramq_first_task = %d\n", sramq.buf[sramq.front]);
    printf("sramq_bytes = %d\n", Qsize(&sramq));
    printf("dramq_first_task = %d\n", dramq.buf[dramq.front]);
    printf("dramq_bytes = %d\n", Qsize(&dramq));
    printf("fmc_first_task = %d\n", fmc.buf[fmc.front]);
    printf("fmc_bytes = %d\n", Qsize(&fmc));
    printf("-------------------------------------\n");
}

int debug_count = 0;
