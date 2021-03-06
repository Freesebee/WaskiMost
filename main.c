#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
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

/*!
 * @details Pomocnicza funkcja wyświetlająca elementy należące do kolejki
 * @param QUEUE_ELEM* Pierwszy element kolejki
 */
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

/*!@details Liczba pojazdów aktualnie znajdujących się w mieście A*/
int cityCountA;

/*!@details Liczba pojazdów aktualnie znajdujących się w mieście B*/
int cityCountB;

/*!@details Kontroluje dostęp wątków do mostu - czy samochód może na niego
 * wjechać, czy nie*/
sem_t semBridge;

//endregion

//region Podstawowe funkcje
/*!
@details wypisuje aktualny stan ruchu przez most
*/
void PrintCurrentState()
{
    if(carOnBridge != -1)
    {
        char* direction;

        if (carOnBridgeDirection == 0 ) //jedzie z miasta A
        {
            direction = ">>";
        }
        else //jedzie z miasta B
        {
            direction = "<<";
        }

        printf("A-%d %d>>> [%s %d %s] <<<%d %d-B\n",
               cityCountA,
               GetQueueLenght(queueA),
               direction,
               carOnBridge,
               direction,
               GetQueueLenght(queueB),
               cityCountB);
    }
    else
        printf("A-%d %d>>> [        ] <<<%d %d-B\n",
           cityCountA,
           GetQueueLenght(queueA),
           GetQueueLenght(queueB),
           cityCountB);
}

/*!
@param _carNumber Wskażnik do numeru pojazdu przydzielonego przy tworzeniu
@details Funkcja wykonywana przez każdy utworzony wątek (wariant b) zadania)
*/
_Noreturn void* CarMovement_vB(void* _carNumber)
{
    int carNumber = *(int*)_carNumber;

    //konieczna dealokacja, gdyż CreateCars() alokuje pamięć specjalnie dla parametru
    free(_carNumber);

    for(;;)
    {
        //region Wyjazd z miasta A i dołączenie do kolejki
        pthread_mutex_lock(&bridgeMutex);
        cityCountA--;
        Enqueue(&queueA, carNumber);
        PrintCurrentState();
        pthread_mutex_unlock(&bridgeMutex);
        //endregion

        //regoin Oczekiwanie i wjazd na most
        pthread_mutex_lock(&bridgeMutex);
        while(Peek(queueA) != carNumber || (carOnBridgeDirection == 0 && GetQueueLenght(queueB) > 0))
        {
            pthread_cond_wait(&bridgeCondition, &bridgeMutex);
        }
        Dequeue(&queueA);
        carOnBridge = carNumber;
        carOnBridgeDirection = 0;
        PrintCurrentState();
        pthread_mutex_unlock(&bridgeMutex);
        //endregion

        //region Zjazd z mostu i wjazd do miasta B
        pthread_mutex_lock(&bridgeMutex);
        carOnBridge = -1;
        cityCountB++;
        PrintCurrentState();
        pthread_cond_broadcast(&bridgeCondition);
        pthread_mutex_unlock(&bridgeMutex);
        //endregion

        //region Wyjazd z miasta B i dołączenie do kolejki
        pthread_mutex_lock(&bridgeMutex);
        cityCountB--;
        Enqueue(&queueB, carNumber);
        PrintCurrentState();
        pthread_mutex_unlock(&bridgeMutex);
        //endregion

        //region Oczekiwanie i wjazd na most
        pthread_mutex_lock(&bridgeMutex);
        while(Peek(queueB) != carNumber || (carOnBridgeDirection == 1 && GetQueueLenght(queueA) > 0))
        {
            pthread_cond_wait(&bridgeCondition, &bridgeMutex);
        }
        Dequeue(&queueB);
        carOnBridge = carNumber;
        carOnBridgeDirection = 1;
        PrintCurrentState();
        pthread_mutex_unlock(&bridgeMutex);
        //endregion

        //region Zjazd z mostu i wjazd do miasta A
        pthread_mutex_lock(&bridgeMutex);
        carOnBridge = -1;
        cityCountA++;
        PrintCurrentState();
        pthread_cond_broadcast(&bridgeCondition);
        pthread_mutex_unlock(&bridgeMutex);
        //endregion
    }
}

/*!
@param _carNumber Wskażnik do numeru pojazdu przydzielonego przy tworzeniu
@details Funkcja wykonywana przez utworzony wątek (wariant a) zadania)
*/
_Noreturn void* CarMovement_vA_A(void* _carNumber)
{
    int carNumber = *(int*)_carNumber;
    free(_carNumber);

    while(1)
    {
        //region Wyjazd z A i dołączenie do kolejki
        pthread_mutex_lock(&bridgeMutex);
        cityCountA--; //opuszczenie miasta A
        Enqueue(&queueA, carNumber); //stanięcie w kolejce z A
        PrintCurrentState();
        pthread_mutex_unlock(&bridgeMutex);
        //endregion

        //region Oczekiwanie i wjazd na most
        do
        {
            sem_wait(&semBridge); //chec wjazdu na most
            //jesli watek jest na szczycie kolejki
            if(Peek(queueA) == carNumber && GetQueueLenght(queueA) >= GetQueueLenght(queueB))
            {
                pthread_mutex_lock(&bridgeMutex);
                Dequeue(&queueA); //opuszcza kolejkę z A
                break;
            }
            else
            {
                sem_post(&semBridge);
            }
        }
        while(1);

        carOnBridge = carNumber; //wjezdza na most
        carOnBridgeDirection = 0; //jedzie z A do B
        PrintCurrentState();
        pthread_mutex_unlock(&bridgeMutex);
        //endregion

        //region Zjazd z mostu i wjazd do miasta B
        pthread_mutex_lock(&bridgeMutex);
        carOnBridge = -1; //zjezdza z mostu
        sem_post(&semBridge); //odblokowanie mostu dla innych aut
        cityCountB++; //wjazd do miasta B
        PrintCurrentState();
        pthread_mutex_unlock(&bridgeMutex);
        //endregion

        //region Wyjazd z miasta B i dołączenie do kolejki
        pthread_mutex_lock(&bridgeMutex);
        cityCountB--; //opuszczenie miasta B
        Enqueue(&queueB, carNumber); //stanięcie w kolejce B
        PrintCurrentState();
        pthread_mutex_unlock(&bridgeMutex);
        //endregion

        //region Oczekiwanie i wjazd na most
        do
        {
            sem_wait(&semBridge); //chec wjazdu na most
            //jesli jest na szczycie kolejki
            if(Peek(queueB) == carNumber && GetQueueLenght(queueA) < GetQueueLenght(queueB))
            {
                pthread_mutex_lock(&bridgeMutex);
                Dequeue(&queueB); //opuszcza kolejkę z B
                break;
            }
            else
            {
                sem_post(&semBridge);
            }
        }
        while(1);

        carOnBridge = carNumber; //wjezdza na most
        carOnBridgeDirection = 1; //jedzie z miasta B do A
        PrintCurrentState();
        pthread_mutex_unlock(&bridgeMutex);
        //endregion

        //region Zjazd z mostu i wjazd do miasta A
        pthread_mutex_lock(&bridgeMutex);
        carOnBridge = -1; //zjezdza z mostu
        cityCountA++; //wjazd do miasta A
        PrintCurrentState();
        sem_post(&semBridge); //odblokowanie mostu dla innych aut
        pthread_mutex_unlock(&bridgeMutex);
        //endregion
    }
}

/*!
@param count Dodatnia liczba pojazdów do utworzenia jako wątki
@param threadFunction Funkcja jaka ma być wykonywana przez utworzone wątki
@returns Tablica utworzonych wątków
@details Tworzy tablicę adresów wątków oraz przydziela każdemu numer pojazdu <0; maxInt>
*/
pthread_t* CreateCars(int count, void* threadFunction)
{
    pthread_t* carsArray = (pthread_t*)malloc(sizeof(pthread_t) * count);

    cityCountA = count;
    cityCountB = 0;

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
@param carsArray Tablica wątków
@param count Rozmiar tablicy wątków
@details Przyłącza utworzone wątki znajduące się w podanej tablicy
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
@param argc Liczba argumentów programu
@param argv Tablica argumentow programu
@returns Liczba pojazdów do utworzenia
@details Sprawdza poprawność podanych i odczytuje wprowadzone argumenty
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
    sem_init(&semBridge, 0, 1);
//    pthread_mutex_init(&cityA,NULL);
//    pthread_mutex_init(&cityB,NULL);
    pthread_mutex_init(&bridgeMutex,NULL);

    int carCountA = carCount;//2;
    int carCountB = carCount - carCountA;

    pthread_t* carsArrayA = CreateCars(carCountA, CarMovement_vA_A);
//    pthread_t* carsArrayB = CreateCars(carCountB, CarMovement_vA_B);

    DestroyCars(carsArrayA,carCountA);
//    DestroyCars(carsArrayB,carCountB);

    pthread_mutex_destroy(&bridgeMutex);
//     pthread_mutex_destroy(&cityA);
//     pthread_mutex_destroy(&cityB);

    sem_destroy(&semBridge);
}

/*!
 * @details Funkcja wykonująca wariant b) programu
 * @param Liczba pojazdów/wątków, które mają przejeżdżać przez most
 */
void CrossBridgeVersionB(int carCount)
{
    pthread_mutex_init(&bridgeMutex, NULL);
    pthread_cond_init(&bridgeCondition, NULL);

    pthread_t* carsArray = CreateCars(carCount, CarMovement_vB);

    pthread_mutex_destroy(&bridgeMutex);
    DestroyCars(carsArray, carCount);
    free(carsArray);
}
//endregion

/*!
 * @details Główna funkcja programu
 * @param argc Liczba argumentów programu
 * @param argv Tablica argumentów programu
 * */
int main(int argc, char** argv)
{
    CrossBridgeVersionA(GetCarCount(argc, argv)); //<<wariant a) zadania
    //CrossBridgeVersionB(GetCarCount(argc, argv)); //<<wariant b) zadania
    return 0;
}
