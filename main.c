#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <unistd.h>
#include <semaphore.h>
#include <pthread.h>

//region Implementacja kolejki

/*!@details Element kolejki będący elementem listy jednokierunkowej*/
typedef struct queueElement{
    int carNuber;
    struct queueElement* next;
} QUEUE_ELEM;

/*!
 * @details Funkcja usuwająca numer pojazdu z kolejki
 * @param QUEUE_ELEM* adres pierwszego elementu kolejki
 * @return (int) pierwszy element kolejki lub -1 jeżeli kolejka jest pusta
 */
int Peek(QUEUE_ELEM* head)
{
    if(head == NULL) return -1;
    return head->carNuber;
}

/*!
 * @details Funkcja dodająca numer pojazdu do kolejki
 * @param QUEUE_ELEM** adres wskaźnika do pierwszego elementu kolejki
 * @return 0 w przypadku sukcesu, -1 w przypadku błędu (errno = ENOMEM)
 */
int Enqueue(QUEUE_ELEM **head, int carNumber)
{
    QUEUE_ELEM* newElem = (QUEUE_ELEM *)malloc(sizeof(QUEUE_ELEM));

    if(newElem == NULL)
    {
        errno = ENOMEM;
        return -1;
    }

    newElem->next = NULL;
    newElem->carNuber = carNumber;

    if(*head == NULL)
    {
        *head = newElem;
        return 0;
    }

    QUEUE_ELEM* iterator = *head;

    while(iterator->next != NULL) iterator = iterator->next;

    iterator->next = newElem;

    return 0;
}

/*!
 * @details Funkcja usuwająca numer pojazdu z kolejki
 * @param QUEUE_ELEM** adres wskaźnika do pierwszego elementu kolejki
 * @return (int) pierwszy element kolejki lub -1 jeżeli kolejka jest pusta
 */
int Dequeue(QUEUE_ELEM **head)
{
    if(*head == NULL) return -1;
    int carNumber = (*head)->carNuber;
    QUEUE_ELEM* first = *head;
    *head = (*head)->next;
    free(first);
    return carNumber;
}

/*!
 * @details Funkcja zwracająca rozmiar kolejki
 * @param QUEUE_ELEM* adres pierwszego elementu kolejki
 * @return (int) rozmiar kolejki
 */
int GetQueueLenght(QUEUE_ELEM* head)
{
    int length = 0;
    QUEUE_ELEM * iterator = head;
    while(iterator != NULL)
    {
        length++;
        iterator = iterator->next;
    }
    return length;
}

void ShowQueue(QUEUE_ELEM* iterator)
{
    if(GetQueueLenght(iterator) > 0)
    {
        while(iterator != NULL)
        {
            printf("%d\n",iterator->carNuber);
            iterator = iterator->next;
        }
    }
}

//endregion

//region Zmienne globalne
/*!@details Opisuje prawa dostępu do pliku kolejki FIFO*/
pthread_mutex_t bridgeMutex;

/*!@details Zmienna warunkowa sygnalizująca wątkom dostęp do zasobu*/
pthread_cond_t bridgeCondition;

/*!@details Pierwszy element kolejki do mostu od strony miasta A*/
QUEUE_ELEM *queueA;

/*!@details Pierwszy element kolejki do mostu od strony miasta B*/
QUEUE_ELEM *queueB;

/*!@details Numer pojazdu aktualnie przejeżdżającego przez most*/
int carOnBridge = -1;

/*!@details Kierunek ruchu pojazdu aktualnie przejeżdżającego przez most
 * 0 jeżeli jedzie z miasta A, 1 jeżeli jedzie z miasta B */
int carOnBridgeDirection;

/*!@details liczba pojazdów aktualnie znajdujących się w mieście A*/
int cityCountA;

/*!@details liczba pojazdów aktualnie znajdujących się w mieście B*/
int cityCountB;

/*!@details semafor miasta A*/ //TODO: do opisania przez Arka
sem_t semA;

/*!@details semafor miasta B*/ //TODO: do opisania przez Arka
sem_t semB;

/*!@details semafor mostu*/ //TODO: do opisania przez Arka
sem_t variable;

//endregion

//region Podstawowe funkcje
/*!
@details wypisuje aktualny stan ruchu przez most
*/
void PrintCurrentState()
{
//    if(carOnBridge != -1)
//    {
//        char* direction;
//
//        if (carOnBridgeDirection == 0 ) //jedzie z miasta A
//        {
//            direction = ">>";
//        }
//        else //jedzie z miasta B
//        {
//            direction = "<<";
//        }
//
//        printf("A-%d %d>>> [%s %d %s] <<<%d %d-B\n",
//               cityCountA,
//               GetQueueLenght(queueA),
//               direction,
//               carOnBridge,
//               direction,
//               GetQueueLenght(queueB),
//               cityCountB);
//    }
//    else

    printf("A-%d %d>>> [    %d    ] <<<%d %d-B\n",
           cityCountA,
           GetQueueLenght(queueA),
           carOnBridge,
           GetQueueLenght(queueB),
           cityCountB);

}

/*!
@param _carNumber wskażnik do numeru pojazdu przydzielonego przy tworzeniu
@details Funkcja wykonywana przez każdy utworzony wątek (wariant b) zadania)
*/
void* CarMovement_vB(void* _carNumber)
{
    int carNumber = *(int*)_carNumber;

    //konieczna dealokacja, gdyż CreateCars() alokuje pamięć specjalnie dla parametru
    free(_carNumber);

    for(;;)
    {
        //region Wyjazd z miasta A
        pthread_mutex_lock(&bridgeMutex);
        cityCountA--;
        Enqueue(&queueA, carNumber);
        PrintCurrentState();
        pthread_mutex_unlock(&bridgeMutex);

        pthread_mutex_lock(&bridgeMutex);

        while(Peek(queueA) != carNumber || (carOnBridgeDirection == 0 && GetQueueLenght(queueB) > 0))
        {
            pthread_cond_wait(&bridgeCondition, &bridgeMutex);
        }

        Dequeue(&queueA);
        //endregion

        //region Ruch na moście i dojazd do B
        carOnBridge = carNumber;
        carOnBridgeDirection = 0;
        PrintCurrentState();
        carOnBridge = -1;
        cityCountB++;
        PrintCurrentState();
        pthread_mutex_unlock(&bridgeMutex);
        pthread_cond_broadcast(&bridgeCondition);
        //endregion

        //region Wyjazd z miasta B
        pthread_mutex_lock(&bridgeMutex);
        cityCountB--;
        Enqueue(&queueB, carNumber);
        PrintCurrentState();
        pthread_mutex_unlock(&bridgeMutex);

        pthread_mutex_lock(&bridgeMutex);
        while(Peek(queueB) != carNumber || (carOnBridgeDirection == 1 && GetQueueLenght(queueA) > 0))
        {
            pthread_cond_wait(&bridgeCondition, &bridgeMutex);
        }

        Dequeue(&queueB);
        //endregion

        //region Ruch na moście i dojazd do A
        carOnBridge = carNumber;
        carOnBridgeDirection = 1;
        PrintCurrentState();
        carOnBridge = -1;
        cityCountA++;
        PrintCurrentState();
        pthread_mutex_unlock(&bridgeMutex);
        pthread_cond_broadcast(&bridgeCondition);
        //endregion
    }

    return NULL;
}

//V - post
//P - wait

/*!
@param _carNumber wskażnik do numeru pojazdu przydzielonego przy tworzeniu
@details Funkcja wykonywana przez utworzony wątek (wariant a) zadania) //TODO: Do opisania przez Arka
*/
_Noreturn void* CarMovement_vA_A(void* _carNumber)
{
    int carNumber = *(int*)_carNumber;
    free(_carNumber);

    while(1)
    {
        sem_post(&variable);
        cityCountA++;

        PrintCurrentState();
        if(cityCountB == 0)
        {
            while(GetQueueLenght(queueA) < cityCountA)
            {
                Enqueue(&queueA, carNumber);
                PrintCurrentState();
                sem_post(&semA);
            }
        }
        sem_post(&variable);
        sem_wait(&semA);
        //wjezdza na most
        carOnBridge = carNumber;
        PrintCurrentState();
        carOnBridge = -1;
        //zjezdza z mostu
        sem_wait(&variable);
        Dequeue(&queueA);
        PrintCurrentState();
        cityCountA--;
        PrintCurrentState();

        if(GetQueueLenght(queueA) == 0)
        {

            while(GetQueueLenght(queueB)<cityCountB)
            {
                Enqueue(&queueB, carNumber);
                PrintCurrentState();

                sem_post(&semB);
            }
        }
        sem_post(&variable);
    }
}

/*!
@param _carNumber wskażnik do numeru pojazdu przydzielonego przy tworzeniu
@details Funkcja wykonywana przez utworzony wątek (wariant a) zadania) //TODO: Do opisania przez Arka
*/
_Noreturn void* CarMovement_vA_B(void* _carNumber)
{
    int carNumber = *(int*)_carNumber;
    free(_carNumber);
    while(1)
    {
        sem_wait(&variable);
        cityCountB++;
        PrintCurrentState();
        if(cityCountA == 0)
        {
            while(GetQueueLenght(queueB)<cityCountB) {
                Enqueue(&queueB, carNumber);
                PrintCurrentState();

                sem_post(&semB);
            }
        }
        sem_post(&variable);
        sem_wait(&semB);
        //wjezdza na most
        carOnBridge = carNumber;
        PrintCurrentState();
        carOnBridge = -1;
        //zjezdza z mostu
        sem_wait(&variable);
        Dequeue(&queueB);
        PrintCurrentState();

        cityCountB--;
        PrintCurrentState();

        if(GetQueueLenght(queueB) == 0)
        {
            while(GetQueueLenght(queueA) < cityCountA)
            {
                Enqueue(&queueA, carNumber);
                PrintCurrentState();

                sem_post(&semA);
            }
        }
        sem_post(&variable);
    }
}

/*!
@param count dodatnia liczba pojazdów do utworzenia jako wątki
@param threadFunction Funkcja jaka ma być wykonywana przez utworzone wątki
@returns tablica utworzonych wątków
@details tworzy tablicę adresów wątków oraz przydziela każdemu numer pojazdu <0; maxInt>
*/
pthread_t* CreateCars(int count, void* threadFunction)
{
    pthread_t* carsArray = (pthread_t*)malloc(sizeof(pthread_t) * count);

    cityCountA = count;
    cityCountB = 0;

    PrintCurrentState();

    for (int i = 0; i < count; ++i)
    {
//        wątki nie startują zaraz po utworzeniu, więc każdy musi odwoływać się
//        do adresu, który nie ulegnie zmianie w czasie przed wystartowaniem
        int* carNumber = (int*)malloc(sizeof (int));
        *carNumber = i;

        if(pthread_create(&carsArray[i], NULL, threadFunction, carNumber) != 0)
        {
            perror("Nie udało się utworzyć wątku");
        }
    }

    return carsArray;
}

/*!
@param carsArray tablica wątków
@param count rozmiar tablicy wątków
@details przyłącza utworzone wątki znajduące się w podanej tablicy
*/
void DestroyCars(pthread_t* carsArray, int count)
{
    for (int i = 0; i < count; ++i)
    {
        if(pthread_join(carsArray[i], NULL) != 0)
        {
            perror("Nie udało się połączyć wątku");
        }
    }
}

/*!
@param argc liczba argumentów programu
@param argv tablica argumentow programu
@returns liczba pojazdów do utworzenia
@details sprawdza poprawność podanych i odczytuje wprowadzone argumenty
*/
int GetCarCount(int argc, char **argv)
{
    if(argc != 2)
    {
        perror("Niepoprawna liczba argumentow\nSyntax: tightbridge liczba_pojazdów\n");
        exit(EXIT_FAILURE);
    }

    int N = strtol(argv[1], NULL, 0);

    if(N <= 0)
    {
        perror("Liczba pojazdów musi być dodatnia\n");
        exit(EXIT_FAILURE);
    }

    return N;
}

//endregion

//region Warianty zadania
/*!
 * @details Funkcja wykonująca wariant a) programu
 * @param Liczba pojazdów/wątków, które mają przejeżdżać przez most
 */
void CrossBridgeVersionA(int carCount)
{
    sem_init(&semA, 0, 0);
    sem_init(&semB, 0, 0);
    sem_init(&variable, 0, 1);

    int carCountA = carCount/2;
    int carCountB = carCount - carCountA;

    pthread_t* carsArrayA = CreateCars(carCountA, CarMovement_vA_A);
    pthread_t* carsArrayB = CreateCars(carCountB, CarMovement_vA_B);

    DestroyCars(carsArrayA,carCountA);
    DestroyCars(carsArrayB,carCountB);

    sem_destroy(&semA);
    sem_destroy(&semB);
    sem_destroy(&variable);
}

/*!
 * @details Funkcja wykonująca wariant b) programu
 * @param Liczba pojazdów/wątków, które mają przejeżdżać przez most
 */
void CrossBridgeVersionB(int carCount)
{
    pthread_cond_init(&bridgeCondition, NULL);

    pthread_t* carsArray = CreateCars(carCount, CarMovement_vB);

    DestroyCars(carsArray, carCount);

    free(carsArray);
}
//endregion

int main(int argc, char** argv)
{
    CrossBridgeVersionB(GetCarCount(argc, argv));
    return 0;
}
