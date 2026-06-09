#include <stdio.h>
#include <stdlib.h>

typedef struct Nodo {
    int n;              // el dato entero
    struct Nodo *p;     // puntero al siguiente nodo (mismo tipo que la estructura)
} Nodo;

    Nodo *x = NULL; // puntero a nodo, inicialmente apunta a NULL (lista vacía)
    Nodo *aux = NULL; 

void imprimir(Nodo *inicio) {
    Nodo *actual = inicio;
    while (actual != NULL) {
        printf("%d", actual->n);
        if (actual->p != NULL) printf(" -> ");
        actual = actual->p;
    }
    printf("\n");
}

Nodo *crearLista(){
    //Reservo memoria para cada nodo
    //n1 ->10, n2 ->40, n3 -> 30, n4 -> 50
    Nodo *n1 = (Nodo *)malloc(sizeof(Nodo));
    Nodo *n2 = (Nodo *)malloc(sizeof(Nodo));
    Nodo *n3 = (Nodo *)malloc(sizeof(Nodo));
    Nodo *n4 = (Nodo *)malloc(sizeof(Nodo)); 

     // Asignamos los valores
    n1->n = 10;
    n2->n = 40;
    n3->n = 30;
    n4->n = 50;

    // Conectamos los punteros: 10->40->30->50->NULL
    n1->p = n2;
    n2->p = n3;
    n3->p = n4;
    n4->p = NULL;

    return n1;
    }


int main()
{
  printf("=== Ejercicio 3a: Reordenar a 10->30->40->50 ===\n");
    x = crearLista();
    printf("Antes: "); 
    imprimir(x);

    aux = x->p;           // aux -> [40]
    x->p = x->p->p;       // [10] -> [30]  (saltamos el 40)
    x->p->p = aux;        // [30] -> [40]  (y el 40 ya apuntaba a [50])

    printf("Despues: "); 
    imprimir(x);

    /* Resultado esperado: 10 -> 30 -> 40 -> 50 */

    return 0;
}   
