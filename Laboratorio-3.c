//LABORATORIO 3
//LABORATORIO 3
//AUTORES MATÍAS MARTIN E IVO DI MARCO
//AUTORES MATÍAS MARTIN E IVO DI MARCO
//LINK DEL REPOSITORIO DE GITHUB: https://github.com/Matiasmartin828/Laboratorio-3
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

sem_t *sem;                                                     //Puntero a semáforo para sincronización
double *saldo;                                                  //Puntero a la variable compartida para el saldo  

void credito(char *nombre_archivo, int pipe_fd)                 //Función para procesar el archivo de crédito
{
    FILE *pArchivo = fopen(nombre_archivo, "r");
    if (pArchivo == NULL)
    {
        printf("No se pudo abrir el archivo de credito\n");
        exit(1);
    }
    double monto;
    while (fscanf(pArchivo, "%lf", &monto) == 1)                //Lee cada monto del archivo de crédito y actualiza el saldo de forma segura utilizando el semáforo
    {
        sem_wait(sem);
        *saldo += monto;
        sem_post(sem);
        write(pipe_fd, &monto, sizeof(monto));
    }
    fclose(pArchivo);
    close(pipe_fd);                                             //Cierra el descriptor de archivo
    return;
}

void debito(char *nombre_archivo, int pipe_fd)                  //Función para procesar el archivo de débito
{
    FILE *pArchivo = fopen(nombre_archivo, "r");
    if (pArchivo == NULL)
    {
        printf("No se pudo abrir el archivo de debito\n");
        exit(1);
    }
    double monto;
    while (fscanf(pArchivo, "%lf", &monto) == 1)                //Lee cada monto del archivo de débito y actualiza el saldo de forma segura utilizando el semáforo
    {
        sem_wait(sem);
        *saldo -= monto;
        sem_post(sem);
        write(pipe_fd, &monto, sizeof(monto));
    }
    fclose(pArchivo);
    close(pipe_fd);                                             //Cierra el descriptor de archivo 
    return;
}

int main(){
    const char *ruta_credito = "credito.txt";                  //Rutas de los archivos de crédito y débito
    const char *ruta_debito = "debito.txt";
    sem = mmap(NULL, sizeof(*sem), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);      //Crea un espacio de memoria compartida para el semáforo   
    if (sem == MAP_FAILED) {
        fprintf(stderr, "Error en mmap: %s (Código: %d)\n", strerror(errno), errno);
        exit(EXIT_FAILURE);
    }
    saldo = mmap(NULL, sizeof(*saldo), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);  //Crea un espacio de memoria compartida para el saldo
    if (saldo == MAP_FAILED) {
        fprintf(stderr, "Error en mmap: %s (Código: %d)\n", strerror(errno), errno);
        exit(EXIT_FAILURE);
    }
    sem_init(sem, 1, 1);                                        //Inicializa el semáforo con un valor de 1 para permitir el acceso exclusivo a la variable compartida     
    int pipe_credito[2];                                        //Crea dos pipes para la comunicación entre procesos, uno para crédito y otro para débito
    int pipe_debito[2];
    pipe(pipe_credito);                                         //Inicializa los pipes
    pipe(pipe_debito);

    pid_t pid_credito = fork();                                 //Crea un proceso hijo para procesar el archivo de crédito
    if (pid_credito == 0){                                      //Código del proceso hijo para crédito
        close(pipe_credito[0]);                                 //Cierra el extremo de lectura del pipe de crédito, ya que solo se usará para escribir
        credito(ruta_credito, pipe_credito[1]);                 //Llama a la función de crédito para procesar el archivo y escribir los montos en el pipe
        close(pipe_credito[1]);                                 //Cierra el extremo de escritura del pipe de crédito después de escribir todos los montos
        exit(0);                                                //Termina el proceso hijo de crédito
    }
    pid_t pid_debito = fork();                                  //Crea otro proceso hijo para procesar el archivo de débito                    
    if (pid_debito == 0){                                       //Código del proceso hijo para débito
        close(pipe_debito[0]);                                  //Cierra el extremo de lectura del pipe de débito, ya que solo se usará para escribir
        debito(ruta_debito, pipe_debito[1]);                    //Llama a la función de débito para procesar el archivo y escribir los montos en el pipe
        close(pipe_debito[1]);                                  //Cierra el extremo de escritura del pipe de débito después de escribir todos los montos
        exit(0);                                                //Termina el proceso hijo de débito                         
    }
    close(pipe_credito[1]);                                     //Cierra el extremo de escritura del pipe de crédito en el proceso padre, ya que solo se usará para leer    
    close(pipe_debito[1]);                                      //Cierra el extremo de escritura del pipe de débito en el proceso padre, ya que solo se usará para leer
    int n1 = 1, n2 = 1;                                         //Variables para controlar la lectura de los pipes, inicializadas en 1 para entrar al bucle de lectura
    double monto;
    while (n1 > 0 || n2 > 0)                                    //Bucle para leer los montos de ambos pipes mientras haya datos disponibles en alguno de ellos
    {
        if (n1 > 0)                                             //Si hay datos disponibles en el pipe de débito, lee un monto y lo imprime
        {
            n1 = read(pipe_debito[0], &monto, sizeof(monto));
            if (n1 > 0)                                         //Si se leyó un monto del pipe de débito, lo imprime
            {
                printf("Monto débito: %lf\n", monto);
            }
        }
        if (n2 > 0)                                             //Si hay datos disponibles en el pipe de crédito, lee un monto y lo imprime
        {
            n2 = read(pipe_credito[0], &monto, sizeof(monto));
            if (n2 > 0)                                         //Si se leyó un monto del pipe de crédito, lo imprime
            {
                printf("Monto crédito: %lf\n", monto);
            }
        }
    }
    printf("Saldo final: %lf\n", *saldo);                       //Imprime el saldo final después de procesar todos los montos de crédito y débito
    wait(NULL);                                                 //Espera a que ambos procesos hijos terminen antes de finalizar el programa
    wait(NULL);
    return 0;
}