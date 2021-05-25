#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>

void* CarGoesBrrrrr(void* arg)
{
    int carNumber = *(int*)arg;

    //konieczna dealokacja, gdyż CreateCars() alokuje pamięć specjalnie dla parametru
    free(arg);

    printf("Car number %d goes brrrr\n", carNumber);
    return NULL;
}

///@param count Liczba pojazdów do utworzenia jako wątki
///@returns (pthread_t*) Tablica wątków
pthread_t* CreateCars(int count)
{
    pthread_t* carsArray = (pthread_t*)malloc(sizeof(pthread_t) * count);

    for (int i = 0; i < count; ++i)
    {
        //wątki nie startują zaraz po utworzeniu, więc każdy musi odwoływać się
        //do adresu, który nie ulegnie zmianie w czasie przed wystartowaniem
        int* carNumber = (int*)malloc(sizeof (int));
        *carNumber = i+1;

        if(pthread_create(&carsArray[i], NULL, CarGoesBrrrrr, carNumber) != 0)
        {
            perror("Nie udało się utworzyć wątku");
        }
    }

    return carsArray;
}

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

/// @param argc Liczba argumentów programu
/// @param argv Tablica argumentow programu
/// @returns Liczba pojazdów do utworzenia
int GetCarCount(int argc, char **argv)
{
    if(argc != 2)
    {
        perror("Niepoprawna liczba argumentow\nSyntax: liczba_pojazdów\n");
        exit(EXIT_FAILURE);
    }

    int N = strtol(argv[1], NULL, 0);

    if(N <= 0)
    {
        printf("Liczba pojazdów musi być dodatnia\n");
        exit(EXIT_FAILURE);
    }

    return N;
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

int main(int argc, char** argv)
{
    int carCount = GetCarCount(argc, argv);
    printf("Liczba pojazdow: %d\n", carCount);

    pthread_t* carsArray = CreateCars(carCount);
    DestroyCars(carsArray, carCount);

    return 0;
}
