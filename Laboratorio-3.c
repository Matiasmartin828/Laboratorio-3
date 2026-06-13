//LABORATORIO 3
//AUTORES MATÍAS MARTIN E IVO DI MARCO
//LABORATORIO 3
//LINK DEL REPOSITORIO DE GITHUB: https://github.com/Matiasmartin828/Laboratorio-3

#include <stdio.h>          /* printf()                 */
#include <stdlib.h>         /* exit(), malloc(), free() */
#include <unistd.h>
#include <sys/types.h>      /* key_t, sem_t, pid_t      */
#include <sys/wait.h>       /* key_t, sem_t, pid_t      */
#include <sys/shm.h>        /* shmat(), IPC_RMID        */
#include <errno.h>          /* errno, ECHILD            */
#include <semaphore.h>      /* sem_open(), sem_destroy(), sem_wait().. */
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>          /* O_CREAT, O_EXEC          */
#include <time.h>

sem_t *sem;
double *saldo;

void credito(char *nombre_archivo, int pipe_fd)
{
    FILE *pArchivo = fopen(nombre_archivo, "r");
    if (pArchivo == NULL)
    {
        printf("No se pudo abrir el archivo de credito\n");
        exit(1);
    }
    double monto;
    while (fscanf(pArchivo, "%lf", &monto) == 1)
    {
        sem_wait(sem);
        *saldo += monto;
        sem_post(sem);
        write(pipe_fd, &monto, sizeof(monto));
    }
    fclose(pArchivo);
    close(pipe_fd);
    return;
}

void debito(char *nombre_archivo, int pipe_fd)
{
    FILE *pArchivo = fopen(nombre_archivo, "r");
    if (pArchivo == NULL)
    {
        printf("No se pudo abrir el archivo de debito\n");
        exit(1);
    }
    double monto;
    while (fscanf(pArchivo, "%lf", &monto) == 1)
    {
        sem_wait(sem);
        *saldo -= monto;
        sem_post(sem);
        write(pipe_fd, &monto, sizeof(monto));
    }

    fclose(pArchivo);
    close(pipe_fd);
    return;
}

int main(){
    const char *ruta_credito = "credito.txt";
    const char *ruta_debito = "debito.txt";
    sem = mmap(NULL, sizeof(*sem), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);     
    if (sem == MAP_FAILED) {
        fprintf(stderr, "Error en mmap: %s (Código: %d)\n", strerror(errno), errno);
        exit(EXIT_FAILURE);
    }
    saldo = mmap(NULL, sizeof(*saldo), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0); 
    if (saldo == MAP_FAILED) {
        fprintf(stderr, "Error en mmap: %s (Código: %d)\n", strerror(errno), errno);
        exit(EXIT_FAILURE);
    }
    sem_init(sem, 1, 1); 
    int pipe_credito[2];   
    int pipe_debito[2];
    pipe(pipe_credito);
    pipe(pipe_debito);

    pid_t pid_credito = fork();
    if (pid_credito == 0){
        close(pipe_credito[0]); 
        credito("credito.txt", pipe_credito[1]);
        close(pipe_credito[1]); // cierra escritura
        exit(0);
    }
    pid_t pid_debito = fork();
    if (pid_debito == 0){
        close(pipe_debito[0]);
        debito("debito.txt", pipe_debito[1]);
        close(pipe_debito[1]); 
        exit(0);
    }
    close(pipe_credito[1]);
    close(pipe_debito[1]);
    int n1 = 1, n2 = 1;
    double monto;
    while (n1 > 0 || n2 > 0) 
    {
        if (n1 > 0)
        {
            n1 = read(pipe_debito[0], &monto, sizeof(monto));
            if (n1 > 0)
            {
                printf("monto DEBITO: %lf\n", monto);
            }
        }
        if (n2 > 0)
        {
            n2 = read(pipe_credito[0], &monto, sizeof(monto));
            if (n2 > 0)
            {
                printf("monto CREDITO: %lf\n", monto);
            }
        }
    }
    printf("Saldo final: %lf\n", *saldo);
    wait(NULL);
    wait(NULL);
    return 0;
}