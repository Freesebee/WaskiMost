#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

//makro opisujące prawa dostępu do pliku kolejki FIFO
#define FILE_MODE (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)

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

pthread_mutex_t bridgeMutex;
pthread_cond_t brdigeCondition;
int selectedCar = 0;

/*!
@param _carNumber wskażnik do numeru pojazdu przydzielonego przy tworzeniu
*/
void* CarMovement(void* _carNumber)
{
    int carNumber = *(int*)_carNumber;

    //konieczna dealokacja, gdyż CreateCars() alokuje pamięć specjalnie dla parametru
    free(_carNumber);

    for/*(;;)*/(int i = 0; i < 2; i++) //TODO: Zastąp nieskończoną pętlą
    {
        printf("#%d w A\n", carNumber);

        pthread_mutex_lock(&bridgeMutex);

        while(selectedCar != 0 && selectedCar != carNumber)
        {
            printf("#%d czeka, bo #%d na moście\n",carNumber,selectedCar);
            pthread_cond_wait(&brdigeCondition, &bridgeMutex);
        }

        selectedCar = carNumber;
        printf("Info od #%d: #%d na moście\n", carNumber, selectedCar);

        selectedCar = 0;
        pthread_mutex_unlock(&bridgeMutex);
        pthread_cond_broadcast(&brdigeCondition);

        printf("#%d w B\n", carNumber);

        pthread_mutex_lock(&bridgeMutex);

        while(selectedCar != 0 && selectedCar != carNumber)
        {
            printf("#%d czeka, bo #%d na moście\n",carNumber,selectedCar);
            pthread_cond_wait(&brdigeCondition, &bridgeMutex);
        }

        selectedCar = carNumber;
        printf("Info od #%d: #%d na moście\n", carNumber, selectedCar);

        selectedCar = 0;
        pthread_mutex_unlock(&bridgeMutex);
        pthread_cond_broadcast(&brdigeCondition);
    }

    printf("#%d w A\n", carNumber);

    return NULL;
}

/*!
@param count liczba pojazdów do utworzenia jako wątki
@returns tablica utworzonych wątków
@details tworzy tablicę adresów wątków oraz przydziela każdemu numer pojazdu jako atrybut
*/
pthread_t* CreateCars(int count)
{
    pthread_t* carsArray = (pthread_t*)malloc(sizeof(pthread_t) * count);

    for (int i = 0; i < count; ++i)
    {
        //wątki nie startują zaraz po utworzeniu, więc każdy musi odwoływać się
        //do adresu, który nie ulegnie zmianie w czasie przed wystartowaniem
        int* carNumber = (int*)malloc(sizeof (int));
        *carNumber = i+1;

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

/*!
@param cityA liczba pojazdów znajdujących się w mieście A
@param cityB liczba pojazdów znajdujących się w mieście B
@param queueA liczba pojazdów jadących z miasta A, czekających na przejazd
@param queueB liczba pojazdów jadących z miasta B, czekających na przejazd
@param direction kierunek ruchu pojazdu przejeżdżającego przez most
@param carOnBridge numer pojazdu przejeżdżającego przez most
@details wypisuje aktualny stan ruchu przez most
*/
void PrintCurrentState(int cityA, int cityB, int queueA, int queueB, char* direction, int carOnBridge)
{
    printf("A-%d %d>>> [%s %d %s] <<<%d %d-B\n",
           cityA,
           queueA,
           direction,
           carOnBridge,
           direction,
           queueB,
           cityB);
}

void Algorytm_MutexySemafory()
{

}

void Algorytm_ZmienneWarunkowe()
{
//    monitor CZYTELNIA;
//    var czyta, pisze: integer;
//    CZYTELNICY, PISARZE: condition;
//
//    export procedure POCZ.CZYTANIA;
//    begin
//    if not empty(PISARZE) or (pisze > 0) then
//    wait CZYTELNICY);
//    czyta := czyta + l
//    end; {POCZ.CZYTANIA}
//
//    export procedure KON_CZYTANIA;
//    begin
//    czyta := czyta - 1;
//    if czyta = O then
//    signal(PISARZE)
//    end; {KON_CZYTANIA}
//
//    export procedure POCZ_PISANIA;
//    begin
//    if czyta+pisze > O then wait(PISARZE);
//    pisze := l
//    end; {POCZ_PISANIA}

//    export procedure KON_PISANIA;
//    begin
//    pisze := 0;
//    if empty(CZYTELNICY) then
//    signal(PISARZE)
//    else
//    repeat
//    signal(CZYTELNICY)
//    until empty(CZYTELNICY)
//    end; {KON_PISANIA}
//
//    begin
//    czyta := 0;
//    pisze := O
//    end; {CZYTELNIA}
}

void CrossBridgeVersionA(int carCount)
{
    pthread_mutex_init(&bridgeMutex, NULL);

    pthread_t* carsArray = CreateCars(carCount);

    //TODO: Reszta kodu wersji a)

    pthread_mutex_destroy(&bridgeMutex);

    free(carsArray);
}

void CrossBridgeVersionB(int carCount)
{
    pthread_cond_init(&brdigeCondition, NULL);

    pthread_t* carsArray = CreateCars(carCount);

    mkfifo("FIFO_A", FILE_MODE);
    mkfifo("FIFO_B", FILE_MODE);

    int pid = fork();

    // PROCES MACIERZYSTY:
    int writefd = open("FIFO_A", O_WRONLY, 0);
    int readfd = open("FIFO_B", O_RDONLY, 0);
    //ProceduraOjca(...);,

    // PROCES POTOMNY:
    readfd = open("FIFO_A", O_RDONLY, 0);
    writefd = open("FIFO_B", O_WRONLY, 0);
    //ProceduraSyna(...);

    DestroyCars(carsArray, carCount);

    free(carsArray);
}

int main(int argc, char** argv)
{
//    CrossBridgeVersionA(GetCarCount(argc, argv));
    CrossBridgeVersionB(GetCarCount(argc, argv));

    return 0;
}
