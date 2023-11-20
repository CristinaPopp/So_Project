#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <ctype.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <stdbool.h>
#include <time.h>

typedef struct antet
{
    int dimensiune_fisier;
    int latime;
    int inaltime;
    int dimensiune_imagine;
} antet;

void proceseazaIntrare(const char *caleIntrare, int fisierStatistica) {
    int var;
    struct stat informatiiFisier;
    if (lstat(caleIntrare, &informatiiFisier) == -1) {
        perror("Eroare la obținerea informațiilor despre fișier");
        return;
    }

    char buffer[1024];
    if (S_ISREG(informatiiFisier.st_mode)) { // Fișier obișnuit
        int fisier = open(caleIntrare, O_RDONLY);
        if (fisier == -1) {
            perror("Eroare la deschiderea fișierului");
            return;
        }

        char semnatura[2];
        read(fisier, semnatura, 2);

        if (semnatura[0] == 'B' && semnatura[1] == 'M') { // Fișier BMP
            antet antetFisier;
            // Citește antetul BMP
            lseek(fisier, 2, SEEK_SET);
            read(fisier, &(antetFisier.dimensiune_fisier), 4);
            lseek(fisier, 18, SEEK_SET);
            read(fisier, &(antetFisier.latime), 4);
            read(fisier, &(antetFisier.inaltime), 4);
            lseek(fisier, 34, SEEK_SET);
            read(fisier, &(antetFisier.dimensiune_imagine), 4);

            // Scrie în fișierul de statistică pentru fișierul BMP
             var = sprintf(buffer,
                              "nume fisier: %s\n"
                              "inaltime: %d\n"
                              "latime: %d\n"
                              "dimensiune: %d\n"
                              "identificatorul utilizatorului: %d\n"
                              "timpul ultimei modificări: %s\n"
                              "contorul de legături: %ld\n"
                              "drepturi de acces user: %c%c%c\n"
                              "drepturi de acces grup: %c%c%c\n"
                              "drepturi de acces alții: %c%c%c\n"
                              "\n",
                              caleIntrare, antetFisier.inaltime, antetFisier.latime, antetFisier.dimensiune_fisier, informatiiFisier.st_uid,
                              ctime(&(informatiiFisier.st_mtime)),
                              informatiiFisier.st_nlink,
                              (informatiiFisier.st_mode & S_IRUSR) ? 'R' : '-',
                              (informatiiFisier.st_mode & S_IWUSR) ? 'W' : '-',
                              (informatiiFisier.st_mode & S_IXUSR) ? 'X' : '-',
                              (informatiiFisier.st_mode & S_IRGRP) ? 'R' : '-',
                              (informatiiFisier.st_mode & S_IWGRP) ? 'W' : '-',
                              (informatiiFisier.st_mode & S_IXGRP) ? 'X' : '-',
                              (informatiiFisier.st_mode & S_IROTH) ? 'R' : '-',
                              (informatiiFisier.st_mode & S_IWOTH) ? 'W' : '-',
                              (informatiiFisier.st_mode & S_IXOTH) ? 'X' : '-');
        } else { // Fișier obișnuit, fără extensia .bmp
            // Scrie în fișierul de statistică pentru fișierul obișnuit
                var = sprintf(buffer,
                              "nume fisier: %s\n"
                              "identificatorul utilizatorului: %d\n"
                              "timpul ultimei modificări: %s\n"
                              "contorul de legături: %ld\n"
                              "drepturi de acces user: %c%c%c\n"
                              "drepturi de acces grup: %c%c%c\n"
                              "drepturi de acces alții: %c%c%c\n"
                              "\n",
                              caleIntrare, informatiiFisier.st_uid,
                              ctime(&(informatiiFisier.st_mtime)),
                              informatiiFisier.st_nlink,
                              (informatiiFisier.st_mode & S_IRUSR) ? 'R' : '-',
                              (informatiiFisier.st_mode & S_IWUSR) ? 'W' : '-',
                              (informatiiFisier.st_mode & S_IXUSR) ? 'X' : '-',
                              (informatiiFisier.st_mode & S_IRGRP) ? 'R' : '-',
                              (informatiiFisier.st_mode & S_IWGRP) ? 'W' : '-',
                              (informatiiFisier.st_mode & S_IXGRP) ? 'X' : '-',
                              (informatiiFisier.st_mode & S_IROTH) ? 'R' : '-',
                              (informatiiFisier.st_mode & S_IWOTH) ? 'W' : '-',
                              (informatiiFisier.st_mode & S_IXOTH) ? 'X' : '-');
        }

        int verificare = write(fisierStatistica, buffer, var);
        if (verificare < 0) {
            perror("Eroare la scrierea datelor în fișier");
        }

        close(fisier);
    } else if (S_ISDIR(informatiiFisier.st_mode)) { // Director
        // Scrie în fișierul de statistică pentru director
        var = sprintf(buffer,
                          "nume director: %s\n"
                          "identificatorul utilizatorului: %d\n"
                          "drepturi de acces user: RWX\n"
                          "drepturi de acces grup: R--\n"
                          "drepturi de acces alții: ---\n"
                          "\n",
                          caleIntrare, informatiiFisier.st_uid);
        int verificare = write(fisierStatistica, buffer, var);
        if (verificare < 0) {
            perror("Eroare la scrierea datelor în fișier");
        }
    } else if (S_ISLNK(informatiiFisier.st_mode)) { // Legătură simbolică
        char caleDestinatie[PATH_MAX];
        ssize_t dimensiuneDestinatie = readlink(caleIntrare, caleDestinatie, PATH_MAX - 1);
        if (dimensiuneDestinatie == -1) {
            perror("Eroare la citirea legăturii simbolice");
            return;
        }
        caleDestinatie[dimensiuneDestinatie] = '\0';

        int var = sprintf(buffer,
                          "nume legatura: %s\n"
                          "dimensiune: %ld\n"
                          "dimensiune fisier: %ld\n"
                          "drepturi de acces user: RWX\n"
                          "drepturi de acces grup: R--\n"
                          "drepturi de acces alții: ---\n"
                          "\n",
                          caleIntrare, dimensiuneDestinatie, informatiiFisier.st_size);
        int verificare = write(fisierStatistica, buffer, var);
        if (verificare < 0) {
            perror("Eroare la scrierea datelor în fișier");
        }
    }
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Nu avem suficiente argumente în linia de comandă\n");
        return EXIT_FAILURE;
    }

    int fisierStatistica = open("statistica.txt", O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU);
    if (fisierStatistica == -1) {
        perror("Eroare deschidere statistica.txt");
        return EXIT_FAILURE;
    }

    DIR *dir = opendir(argv[1]);
    if (dir == NULL) {
        perror("Eroare la deschiderea directorului");
        return EXIT_FAILURE;
    }

    struct dirent *intrare;
    while ((intrare = readdir(dir)) != NULL) {
        if (strcmp(intrare->d_name, ".") != 0 && strcmp(intrare->d_name, "..") != 0) {
            char caleIntrare[PATH_MAX];
            snprintf(caleIntrare, PATH_MAX, "%s/%s", argv[1], intrare->d_name);
            proceseazaIntrare(caleIntrare, fisierStatistica);
        }
    }

    closedir(dir);
    close(fisierStatistica);

    return 0;
}
