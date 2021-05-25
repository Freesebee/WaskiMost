#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>

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
int bridge = 0;

/*!
@param _carNumber wskażnik do numeru pojazdu przydzielonego przy tworzeniu
*/
void* CarMovement(void* _carNumber)
{
    int carNumber = *(int*)_carNumber;

    //konieczna dealokacja, gdyż CreateCars() alokuje pamięć specjalnie dla parametru
    free(_carNumber);

    for(int i = 0; i < 2; i++) //TODO: Zastąp nieskończoną pętlą
    {
        printf("#%d w A\n", carNumber);

        pthread_mutex_lock(&bridgeMutex);

        while(bridge != 0)
        {
            printf("#%d czeka, bo #%d na moście\n",carNumber,bridge);
            pthread_cond_wait(&brdigeCondition, &bridgeMutex);
        }

        bridge = carNumber;
        printf("Info od #%d: #%d na moście\n",carNumber,bridge);
        bridge = 0;
        pthread_mutex_unlock(&bridgeMutex);
        pthread_cond_signal(&brdigeCondition);
//        sleep(1);

        printf("#%d w B\n", carNumber);

        pthread_mutex_lock(&bridgeMutex);

        while(bridge != 0)
        {
            printf("#%d czeka, bo #%d na moście\n",carNumber,bridge);
            pthread_cond_wait(&brdigeCondition, &bridgeMutex);
        }

        bridge = carNumber;
        printf("Info od #%d: #%d na moście\n",carNumber,bridge);
        bridge = 0;
        pthread_mutex_unlock(&bridgeMutex);
        pthread_cond_signal(&brdigeCondition);
//        sleep(1);

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
@param cityACarsCount liczba pojazdów znajdujących się w mieście A
@param cityBCarsCount liczba pojazdów znajdujących się w mieście B
@param queueACarsCount liczba pojazdów jadących z miasta A, czekających na przejazd
@param queueBCarsCount liczba pojazdów jadących z miasta B, czekających na przejazd
@param direction kierunek ruchu pojazdu przejeżdżającego przez most
@param carOnBridgeNumber numer pojazdu przejeżdżającego przez most
@details wypisuje aktualny stan ruchu przez most
*/
void PrintCurrentState(int cityACarsCount,
                       int cityBCarsCount,
                       int queueACarsCount,
                       int queueBCarsCount,
                       char* direction,
                       int carOnBridgeNumber)
{
    printf("A-%d %d>>> [%s %d %s] <<<%d %d-B\n",
           cityACarsCount,
           queueACarsCount,
           direction,
           carOnBridgeNumber,
           direction,
           queueBCarsCount,
           cityBCarsCount);
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

    //Pomocnicze kolekcje
//    int* aCityQueue = (int*)calloc(carCount,sizeof(int));
//    int* bCityQueue = (int*)calloc(carCount,sizeof(int));

    DestroyCars(carsArray, carCount);

    free(carsArray);
}

int main(int argc, char** argv)
{
//    CrossBridgeVersionA(GetCarCount(argc, argv));
    CrossBridgeVersionB(GetCarCount(argc, argv));

    return 0;
}
