#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <stdbool.h>
#include <time.h>

typedef struct header
{
    int file_size;
    int width;
    int height;
    int image_size;
} header;

// Functie pentru procesarea informatiilor despre un fisier obisnuit
void process_file(char *filename, int statistica) {
    int fis = open(filename, O_RDONLY);
    header header;

    lseek(fis, 2, SEEK_SET);
    read(fis, &(header.file_size), 4);

    lseek(fis, 18, SEEK_SET);
    read(fis, &(header.width), 4);
    read(fis, &(header.height), 4);

    lseek(fis, 34, SEEK_SET);
    read(fis, &(header.image_size), 4);

    struct stat file_info;
    if (stat(filename, &file_info) == -1) {
        perror("Eroare la obtinerea informatiilor despre fisier");
        exit(1);
    }

    char time_buffer[200];
    strftime(time_buffer, sizeof(time_buffer), "%d-%m-%Y", localtime(&(file_info.st_mtime)));

    char buffer[1024];
    int var;

    // Verifica daca fisierul are extensia .bmp
    if (strstr(filename, ".bmp") != NULL) {
        var = sprintf(buffer,
                      "nume fisier: %s\n"
                      "inaltime: %d\n"
                      "lungime: %d\n"
                      "dimensiune: %d\n"
                      "identificatorul utilizatorului: %d\n"
                      "timpul ultimei modificari: %s\n"
                      "contorul de legaturi: %ld\n"
                      "drepturi de acces user: %c%c%c\n"
                      "drepturi de acces grup: %c%c%c\n"
                      "drepturi de acces altii: %c%c%c\n"
                      "\n",
                      filename, header.height, header.width, header.file_size, file_info.st_uid,
                      time_buffer,
                      file_info.st_nlink,
                      (file_info.st_mode & S_IRUSR) ? 'R' : '-',
                      (file_info.st_mode & S_IWUSR) ? 'W' : '-',
                      (file_info.st_mode & S_IXUSR) ? 'X' : '-',
                      (file_info.st_mode & S_IRGRP) ? 'R' : '-',
                      (file_info.st_mode & S_IWGRP) ? 'W' : '-',
                      (file_info.st_mode & S_IXGRP) ? 'X' : '-',
                      (file_info.st_mode & S_IROTH) ? 'R' : '-',
                      (file_info.st_mode & S_IWOTH) ? 'W' : '-',
                      (file_info.st_mode & S_IXOTH) ? 'X' : '-');
    } else {
        // Daca fisierul nu are extensia .bmp, afiseaza informatiile fara lungime si inaltime
        var = sprintf(buffer,
                      "nume fisier: %s\n"
                      "identificatorul utilizatorului: %d\n"
                      "timpul ultimei modificari: %s\n"
                      "contorul de legaturi: %ld\n"
                      "drepturi de acces user: %c%c%c\n"
                      "drepturi de acces grup: %c%c%c\n"
                      "drepturi de acces altii: %c%c%c\n"
                      "\n",
                      filename, file_info.st_uid,
                      time_buffer,
                      file_info.st_nlink,
                      (file_info.st_mode & S_IRUSR) ? 'R' : '-',
                      (file_info.st_mode & S_IWUSR) ? 'W' : '-',
                      (file_info.st_mode & S_IXUSR) ? 'X' : '-',
                      (file_info.st_mode & S_IRGRP) ? 'R' : '-',
                      (file_info.st_mode & S_IWGRP) ? 'W' : '-',
                      (file_info.st_mode & S_IXGRP) ? 'X' : '-',
                      (file_info.st_mode & S_IROTH) ? 'R' : '-',
                      (file_info.st_mode & S_IWOTH) ? 'W' : '-',
                      (file_info.st_mode & S_IXOTH) ? 'X' : '-');
    }

    int verify = write(statistica, buffer, var);
    if (verify < 0) {
        perror("Eroare la scrierea datelor despre fisier in fisierul de statistica");
        exit(1);
    }

    close(fis);
}

// Functie pentru procesarea informatiilor despre un director
void process_directory(char *dirname, int statistica) {
    struct stat dir_info;
    if (stat(dirname, &dir_info) == -1) {
        perror("Eroare la obtinerea informatiilor despre director");
        exit(1);
    }

    char buffer[1024];
    int var = sprintf(buffer,
                      "nume director: %s\n"
                      "identificatorul utilizatorului: %d\n"
                      "drepturi de acces user: RWX\n"
                      "drepturi de acces grup: R--\n"
                      "drepturi de acces altii: ---\n\n",
                      dirname, dir_info.st_uid);

    int verify = write(statistica, buffer, var);
    if (verify < 0) {
        perror("Eroare la scrierea datelor despre director in fisierul de statistica");
        exit(1);
    }
}

// Functie pentru procesarea informatiilor despre o legatura simbolica
void process_symbolic_link(char *linkname, int statistica) {
    struct stat link_info;
    if (lstat(linkname, &link_info) == -1) {
        perror("Eroare la obtinerea informatiilor despre legatura simbolica");
        exit(1);
    }

    struct stat target_info;
    if (stat(linkname, &target_info) == -1) {
        perror("Eroare la obtinerea informatiilor despre fisierul tinta al legaturii simbolice");
        exit(1);
    }

    char buffer[1024];
    int var = sprintf(buffer,
                      "nume legatura: %s\n"
                      "dimensiune: %ld\n"
                      "dimensiune fisier: %ld\n"
                      "drepturi de acces user: RWX\n"
                      "drepturi de acces grup: R--\n"
                      "drepturi de acces altii: ---\n\n",
                      linkname, link_info.st_size, target_info.st_size);

    int verify = write(statistica, buffer, var);
    if (verify < 0) {
        perror("Eroare la scrierea datelor despre legatura simbolica in fisierul de statistica");
        exit(1);
    }
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Utilizare: %s <director_intrare>\n", argv[0]);
        return EXIT_FAILURE;
    }

    int statistica = open("statistica.txt", O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU);
    if (statistica == -1) {
        perror("Eroare la deschiderea fisierului statistica.txt");
        return EXIT_FAILURE;
    }

    DIR *dir = opendir(argv[1]);
    if (dir == NULL) {
        perror("Eroare la deschiderea directorului");
        return EXIT_FAILURE;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_REG) {
            char filename[1024];
            sprintf(filename, "%s/%s", argv[1], entry->d_name);
            process_file(filename, statistica);
        } else if (entry->d_type == DT_DIR) {
            char dirname[1024];
            sprintf(dirname, "%s/%s", argv[1], entry->d_name);
            process_directory(dirname, statistica);
        } else if (entry->d_type == DT_LNK) {
            char linkname[1024];
            sprintf(linkname, "%s/%s", argv[1], entry->d_name);
            process_symbolic_link(linkname, statistica);
        }
    }

    closedir(dir);
    close(statistica);

    return EXIT_SUCCESS;
}
