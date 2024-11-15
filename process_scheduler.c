// Matéria: Arquitetura de Computadores e Sistemas Operacionais
// Turma: 2024.2
// Professora: Valéria Bastos
// Alunos: Joâo Vitor Lopes Alvarenga e Yuri Elias da Silva Laranja
// Trabalho 1: Escalonador de Processos

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define MAX_PROCESSES 100
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

void print_separator() {
    printf("\n========================================================\n");
}

void print_header() {
    printf("\n******************** SIMULADOR DE PROCESSOS ********************\n");
}

void print_process_info(Process* p) {
    printf("| PID: %2d | Burst: %2d | Chegada: %2d | I/O: %2d (%s) |\n",
           p->pid, p->burst_time, p->arrival_time, p->io_time,
           DEVICE_NAMES[p->io_device]);
}

void print_queue_status(int time, Queue* ready_high, Queue* ready_low, Queue io_queues[3]) {
    printf("\n[Tempo %d] Status das Filas:\n", time);
    printf("+--------------------------+\n");
    printf("| Fila             | Total |\n");
    printf("+--------------------------+\n");
    printf("| Alta Prioridade  |  %2d   |\n", ready_high->count);
    printf("| Baixa Prioridade |  %2d   |\n", ready_low->count);
    printf("| Disco            |  %2d   |\n", io_queues[0].count);
    printf("| Fita Magnética   |  %2d   |\n", io_queues[1].count);
    printf("| Impressora       |  %2d   |\n", io_queues[2].count);
    printf("+--------------------------+\n");
}

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

void manage_queues(Queue* ready_high, Queue* ready_low, Queue io_queues[3], Process* process) {
    if (process->remaining_time == 0) {
        printf("\n[FINALIZADO] Processo %d completou execução\n", process->pid);
        return;
    }

    if (process->io_time > 0) {
        enqueue(&io_queues[process->io_device], process);
        printf("\n[I/O] Processo %d iniciando operação de I/O (%s)\n", 
               process->pid, DEVICE_NAMES[process->io_device]);
    } else {
        if (process->priority == 0) {
            enqueue(ready_high, process);
            printf("\n[FILA] Processo %d movido para fila de alta prioridade\n", process->pid);
        } else {
            enqueue(ready_low, process);
            printf("\n[FILA] Processo %d movido para fila de baixa prioridade\n", process->pid);
        }
    }
}

void simulate_scheduler(Process processes[], int n) {
    print_header();
    Queue ready_high = { .count = 0 };
    Queue ready_low = { .count = 0 };
    Queue io_queues[3] = { { .count = 0 }, { .count = 0 }, { .count = 0 } };

    int time = 0, completed = 0;

    printf("\nIniciando simulação com %d processos...\n", n);
    print_separator();

    while (completed < n || !is_empty(&ready_high) || !is_empty(&ready_low) ||
           (!is_empty(&io_queues[0]) || !is_empty(&io_queues[1]) || !is_empty(&io_queues[2]))) {
        
        print_queue_status(time, &ready_high, &ready_low, io_queues);

        // Adicionar novos processos
        for (int i = 0; i < n; i++) {
            if (processes[i].arrival_time == time) {
                enqueue(&ready_high, &processes[i]);
                printf("\n[NOVO] Processo %d chegou ao sistema\n", processes[i].pid);
            }
        }

        // Executar processos
        Process* current = dequeue(&ready_high);
        if (current == NULL) current = dequeue(&ready_low);

        if (current != NULL) {
            int execute_time = (current->remaining_time < TIME_QUANTUM) ? 
                              current->remaining_time : TIME_QUANTUM;
            current->remaining_time -= execute_time;
            time += execute_time;

            printf("\n[EXECUÇÃO] Processo %d executando por %d unidades de tempo\n", 
                   current->pid, execute_time);

            if (current->remaining_time == 0) {
                printf("\n[FINALIZADO] Processo %d completou execução no tempo %d\n", 
                       current->pid, time);
                completed++;
            } else {
                manage_queues(&ready_high, &ready_low, io_queues, current);
            }
        } else {
            if (is_empty(&ready_high) && is_empty(&ready_low) &&
                is_empty(&io_queues[0]) && is_empty(&io_queues[1]) && is_empty(&io_queues[2])) {
                print_separator();
                printf("\nSimulação encerrada no tempo %d\n", time);
                printf("Total de processos completados: %d\n", completed);
                print_separator();
                return;
            } else {
                printf("\n[SISTEMA] CPU ociosa no tempo %d\n", time);
                time++;
            }
        }

        // Gerenciar I/O
        for (int i = 0; i < 3; i++) {
            if (!is_empty(&io_queues[i])) {
                Process* io_process = dequeue(&io_queues[i]);
                io_process->io_time--;

                if (io_process->io_time == 0) {
                    io_process->priority = (i == 0) ? 1 : 0;
                    enqueue((i == 0) ? &ready_low : &ready_high, io_process);
                    printf("\n[I/O] Processo %d finalizou operação de I/O em %s\n", 
                           io_process->pid, DEVICE_NAMES[i]);
                } else {
                    enqueue(&io_queues[i], io_process);
                }
            }
        }
        print_separator();
    }
}

int main() {
    print_header();
    int n;

    printf("\nInforme o número de processos (máximo %d): ", MAX_PROCESSES);
    scanf("%d", &n);

    if (n > MAX_PROCESSES) {
        printf("\nERRO: Número de processos excede o limite de %d.\n", MAX_PROCESSES);
        return 1;
    }

    Process processes[MAX_PROCESSES];
    srand(time(NULL));

    printf("\nProcessos criados:\n");
    print_separator();
    
    for (int i = 0; i < n; i++) {
        processes[i].pid = i + 1;
        processes[i].burst_time = rand() % 16 + 5;
        processes[i].remaining_time = processes[i].burst_time;
        processes[i].arrival_time = rand() % 10;
        processes[i].priority = 0;
        processes[i].io_time = rand() % 6 + 2;
        processes[i].io_device = rand() % 3;
        
        print_process_info(&processes[i]);
    }
    
    print_separator();
    simulate_scheduler(processes, n);

    return 0;
}