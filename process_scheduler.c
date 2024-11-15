// Trabalho 1: Escalonador de Processos
// Matéria: Arquitetura de Computadores e Sistemas Operacionais
// Professpra: Valéria Bastos
// Alunos: Joâo Vitor Lopes Alvarenga e Yuri Elias da Silva Laranja

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define MAX_PROCESSES 50
#define TIME_QUANTUM 5
#define DISK_IO_TIME 5
#define TAPE_IO_TIME 3
#define PRINTER_IO_TIME 4

const char* DEVICE_NAMES[] = {"Disco", "Fita Magnética", "Impressora"};

typedef struct {
    int pid;
    int burst_time;
    int remaining_time;
    int arrival_time;
    int priority; 
    int io_time;
    int io_device; 
} Process;

typedef struct {
    Process* processes[MAX_PROCESSES];
    int count;
} Queue;

void enqueue(Queue* queue, Process* process) {
    queue->processes[queue->count++] = process;
}

Process* dequeue(Queue* queue) {
    if (queue->count == 0) return NULL;
    Process* process = queue->processes[0];
    for (int i = 1; i < queue->count; i++) {
        queue->processes[i - 1] = queue->processes[i];
    }
    queue->count--;
    return process;
}

int is_empty(Queue* queue) {
    return queue->count == 0;
}

void manage_queues(Queue* ready_high, Queue* ready_low, Queue* io_queues[3], Process* process) {
    if (process->remaining_time == 0) {
        printf("Processo %d finalizado.\n", process->pid);
        return;
    }

    if (process->io_time > 0) {
        enqueue(io_queues[process->io_device], process);
        printf("Processo %d enviado para I/O (%s).\n", process->pid, DEVICE_NAMES[process->io_device]);
    } else {
        if (process->priority == 0) {
            enqueue(ready_high, process);
        } else {
            enqueue(ready_low, process);
        }
    }
}

void simulate_scheduler(Process processes[], int n) {
    Queue ready_high = { .count = 0 };
    Queue ready_low = { .count = 0 };
    Queue io_queues[3] = { { .count = 0 }, { .count = 0 }, { .count = 0 } };

    int time = 0, completed = 0;

    while (completed < n) {
        // Adicionar novos processos
        for (int i = 0; i < n; i++) {
            if (processes[i].arrival_time == time) {
                enqueue(&ready_high, &processes[i]);
                printf("Tempo %d: Processo %d chegou.\n", time, processes[i].pid);
            }
        }

        // Executar processos
        Process* current = dequeue(&ready_high);
        if (current == NULL) current = dequeue(&ready_low);

        if (current != NULL) {
            int execute_time = (current->remaining_time < TIME_QUANTUM) ? current->remaining_time : TIME_QUANTUM;
            current->remaining_time -= execute_time;
            time += execute_time;

            if (current->remaining_time == 0) {
                printf("Tempo %d: Processo %d finalizado.\n", time, current->pid);
                completed++;
            } else {
                manage_queues(&ready_high, &ready_low, io_queues, current);
            }
        } else {
            printf("Tempo %d: CPU ociosa.\n", time);
            time++;
        }

        // Gerenciar I/O
        for (int i = 0; i < 3; i++) {
            if (!is_empty(&io_queues[i])) {
                Process* io_process = dequeue(&io_queues[i]);
                io_process->io_time--;

                if (io_process->io_time == 0) {
                    io_process->priority = (i == 0) ? 1 : 0; 
                    enqueue((i == 0) ? &ready_low : &ready_high, io_process);
                    printf("Tempo %d: Processo %d finalizou I/O no %s.\n", time, io_process->pid, DEVICE_NAMES[i]);
                } else {
                    enqueue(&io_queues[i], io_process);
                }
            }
        }
    }
}

int main() {
    int n;

    printf("Informe o número de processos (máximo %d): ", MAX_PROCESSES);
    scanf("%d", &n);

    if (n > MAX_PROCESSES) {
        printf("Número de processos excede o limite de %d.\n", MAX_PROCESSES);
        return 1;
    }

    Process processes[MAX_PROCESSES];
    srand(time(NULL));

    for (int i = 0; i < n; i++) {
        processes[i].pid = i + 1;
        processes[i].burst_time = rand() % 16 + 5; 
        processes[i].remaining_time = processes[i].burst_time;
        processes[i].arrival_time = rand() % 10; 
        processes[i].priority = 0; 
        processes[i].io_time = rand() % 6 + 2; 
        processes[i].io_device = rand() % 3;

        printf("Processo %d: Burst=%d, Chegada=%d, I/O=%d (%s)\n",
               processes[i].pid, processes[i].burst_time,
               processes[i].arrival_time, processes[i].io_time,
               DEVICE_NAMES[processes[i].io_device]);
    }

    simulate_scheduler(processes, n);

    return 0;
}
