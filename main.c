#include <stdio.h>

void InitializeParameters(int argc, char **argv)
{
    if(argc < 2 || argc > 8)
    {
//        TODO: Popraw
        printf("Niepoprawna liczba argumentow\nSyntax: *source *dest [-R] [-st sleepTime] [-fs fileSizeThreshold]\n");
        exit(EXIT_FAILURE);
    }

    // Sprawdzanie czy sciezka zrodlowa to katalog
    if (isDirectory(argv[1]) == 0)
    {
        printf("Sciezka zrodlowa nie jest katalogiem\n");
        exit(EXIT_FAILURE);
    }
    else {
        source = realpath(argv[1], NULL);
    }

    // Sprawdzanie czy sciezka docelowa to katalog
    if (isDirectory(argv[2]) == 0)
    {
        printf("Sciezka docelowa nie jest katalogiem\n");
        exit(EXIT_FAILURE);
    }
    else {
        dest = realpath(argv[2], NULL);
    }

    if(strcmp(dest,source) == 0)
    {
        printf("Sciezka docelowa nie moze byc taka sama jak zrodlowa\n");
        exit(EXIT_FAILURE);
    }

    int i;
    bool fs_set = false, st_set = false, R_set = false;

    for(i = 3; i < argc; i++)
    {
        if (strcmp(argv[i], "-R") == 0)
        {
            if(R_set)
            {
                printf("Parametr allowRecursion zosal juz zainicjalizowany\n");
                exit(EXIT_FAILURE);
            }

            allowRecursion = true;
            R_set = true;
        }
        else if (strcmp(argv[i], "-fs") == 0)
        {
            if(fs_set)
            {
                printf("Parametr fileSizeThreshold zosal juz zainicjalizowany\n");
                exit(EXIT_FAILURE);
            }

            if(++i > argc-1 || (fileSizeThreshold = atoi(argv[i])) <= 0)
            {
                printf("Parametr fileSizeThreshold musi byc liczba dodatnia\n");
                exit(EXIT_FAILURE);
            }

            fs_set = true;
        }
        else if (strcmp(argv[i], "-st") == 0)
        {
            if(st_set)
            {
                printf("Parametr sleepTime zostal juz zainicjalizowany\n");
                exit(EXIT_FAILURE);
            }

            if (++i > argc-1 || (sleepTime = atoi(argv[i])) <= 0)
            {
                printf("Parametr sleepTime musi byc liczba dodatnia\n");
                exit(EXIT_FAILURE);
            }

            st_set = true;
        }
        else
        {
            printf("Blad %d argumentu\nSyntax: *source *dest [-R] [-st sleepTime] [-fs fileSizeThreshold]\n",i);
            exit(EXIT_FAILURE);
        }
    }
}

int main(int argc, char** argv) {
    InitializeParameters(argc, argv)
    printf("Hello, World!\n");
    return 0;
}
