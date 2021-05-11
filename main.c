#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>

int InitializeParameters(int argc, char **argv)
{
    if(argc != 2)
    {
        printf("Niepoprawna liczba argumentow\nSyntax: liczba_pojazdów\n");
        exit(EXIT_FAILURE);
    }

    int N = atoi(argv[1]);

    if(N <= 0)
    {
        printf("Liczba pojazdów musi być dodatnia\n");
        exit(EXIT_FAILURE);
    }

    return N;
}

int main(int argc, char** argv)
{
    int liczbaPojazdow = InitializeParameters(argc, argv);
    printf("Liczba pojazdow: %d\n", liczbaPojazdow);
    return 0;
}
