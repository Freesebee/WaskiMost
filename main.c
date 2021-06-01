#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>

//TODO: Realizacja b) - funkcje:
/*

pthread_cond_signal(&brdigeCondition, &bridgeMutex);

pthread_cond_wait(&brdigeCondition);
Wykonuje:
1. pthread_mutex_unlock(&bridgeMutex);
2. Czeka na sygnał od pthread_cond_signal(&brdigeCondition, &bridgeMutex);
3. pthread_mutex_lock(&bridgeMutex);

pthread_cond_broadcast(&brdigeCondition, &bridgeMutex);
Wykonuje to samo co pthread_cond_wait(,) lecz dla wszystkich wątków oczekujących na sygnał
*/

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

//endregion

//region Zmienne globalne
/*!@details Opisuje prawa dostępu do pliku kolejki FIFO*/
pthread_mutex_t bridgeMutex;

/*!@details Zmienna warunkowa sygnalizująca wątkom dostęp do zasobu*/
pthread_cond_t brdigeCondition;

/*!@details Pierwszy element kolejki*/
QUEUE_ELEM *queueA, *queueB;

/*!@details Numer pojazdu aktualnie przejeżdżającego przez most*/
int selectedCar = 0;

/*!@details liczba pojazdów aktualnie znajdujących się w mieście A*/
int cityCountA = 0;

/*!@details liczba pojazdów aktualnie znajdujących się w mieście B*/
int cityCountB = 0;

//endregion

//region Podstawowe funkcje
/*!
@param cityA liczba pojazdów znajdujących się w mieście A
@param cityB liczba pojazdów znajdujących się w mieście B
@param queueA liczba pojazdów jadących z miasta A, czekających na przejazd
@param queueB liczba pojazdów jadących z miasta B, czekających na przejazd
@param fromCityA kierunek ruchu pojazdu przejeżdżającego przez most
@param carOnBridge numer pojazdu przejeżdżającego przez most
@details wypisuje aktualny stan ruchu przez most
*/
void PrintCurrentState(int cityA, int cityB, char fromCityA, int carInfo,int carOnBridge )
{
    char* direction;

    if (fromCityA == 0) //jedzie z miasta B
    {
       direction = "<<";
    }
    else //jedzie z miasta A
    {
        direction = ">>";
    }

    printf("[#%d] A-%d %d>>> [%s %d %s] <<<%d %d-B\n",
           carInfo,
           cityA,
           GetQueueLenght(queueA),
           direction,
           carOnBridge,
           direction,
           GetQueueLenght(queueB),
           cityB);
}

/*!
@param _carNumber wskażnik do numeru pojazdu przydzielonego przy tworzeniu
@details Funkcja wykonywana przez każdy utworzony wątek
*/
void* CarMovement(void* _carNumber)
{
    int carNumber = *(int*)_carNumber;

    //konieczna dealokacja, gdyż CreateCars() alokuje pamięć specjalnie dla parametru
    free(_carNumber);

    for/*(;;)*/(int i = 0; i < 3; i++) //TODO: Zastąp nieskończoną pętlą
    {
        printf("#%d w A\n", carNumber);

        pthread_mutex_lock(&bridgeMutex);

        while(selectedCar != -1 /*&& selectedCar != carNumber*/)
        {
            printf("#%d czeka, bo #%d na moście\n",carNumber,selectedCar);
            pthread_cond_wait(&brdigeCondition, &bridgeMutex);
        }

        selectedCar = carNumber;
        PrintCurrentState(0,0,1,carNumber, selectedCar);

        selectedCar = -1;
        pthread_mutex_unlock(&bridgeMutex);
        pthread_cond_broadcast(&brdigeCondition);

        printf("#%d w B\n", carNumber);

        pthread_mutex_lock(&bridgeMutex);

        while(selectedCar != -1 /*&& selectedCar != carNumber*/)
        {
            printf("#%d czeka, bo #%d na moście\n",carNumber,selectedCar);
            pthread_cond_wait(&brdigeCondition, &bridgeMutex);
        }

        selectedCar = carNumber;
        PrintCurrentState(0,0,0,carNumber,selectedCar);

        selectedCar = -1;
        pthread_mutex_unlock(&bridgeMutex);
        pthread_cond_broadcast(&brdigeCondition);
    }

    printf("#%d w A\n", carNumber);

    return NULL;
}

/*!
@param count dodatnia liczba pojazdów do utworzenia jako wątki
@returns tablica utworzonych wątków
@details tworzy tablicę adresów wątków oraz przydziela każdemu numer pojazdu <0; maxInt>
*/
pthread_t* CreateCars(int count)
{
    pthread_t* carsArray = (pthread_t*)malloc(sizeof(pthread_t) * count);

    for (int i = 0; i < count; ++i)
    {
//        wątki nie startują zaraz po utworzeniu, więc każdy musi odwoływać się
//        do adresu, który nie ulegnie zmianie w czasie przed wystartowaniem
        int* carNumber = (int*)malloc(sizeof (int));
        *carNumber = i;

        if(pthread_create(&carsArray[i], NULL, CarMovement, carNumber) != 0)
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

void CrossBridgeVersionB(int carCount)
{
    pthread_cond_init(&brdigeCondition, NULL);

    pthread_t* carsArray = CreateCars(carCount);

    DestroyCars(carsArray, carCount);

    free(carsArray);
}

int main(int argc, char** argv)
{
    int carCount = GetCarCount(argc, argv);

    pthread_cond_init(&brdigeCondition, NULL);

    pthread_t* carsArray = CreateCars(carCount);

    DestroyCars(carsArray, carCount);

    free(carsArray);

    return 0;
}
