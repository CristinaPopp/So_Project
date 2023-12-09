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

void process_file(char *numefisier, int statistica) 
{
    int fisier = open(numefisier, O_RDONLY);
    header header;

    lseek(fisier, 2, SEEK_SET);
    read(fisier, &(header.file_size), 4);

    lseek(fisier, 18, SEEK_SET);
    read(fisier, &(header.width), 4);
    read(fisier, &(header.height), 4);

    lseek(fisier, 34, SEEK_SET);
    read(fisier, &(header.image_size), 4);

    struct stat file_info;
    if (stat(numefisier, &file_info) == -1) 
    {
        perror("Eroare la obtinerea informatiilor despre fisier.");
        exit(1);
    }

    char time_buffer[200];
    strftime(time_buffer, sizeof(time_buffer), "%d-%m-%Y", localtime(&(file_info.st_mtime)));

    char buffer[1024];
    int alo;

    if (strstr(numefisier, ".bmp") != NULL) 
    {
        alo = sprintf(buffer,
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
                      numefisier, header.height, header.width, header.file_size, file_info.st_uid,
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
    else 
    {
        alo = sprintf(buffer,
                      "nume fisier: %s\n"
                      "identificatorul utilizatorului: %d\n"
                      "timpul ultimei modificari: %s\n"
                      "contorul de legaturi: %ld\n"
                      "drepturi de acces user: %c%c%c\n"
                      "drepturi de acces grup: %c%c%c\n"
                      "drepturi de acces altii: %c%c%c\n"
                      "\n",
                      numefisier, file_info.st_uid,
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

    int verifica = write(statistica, buffer, alo);
    if (verifica < 0) 
    {
        perror("Error writing file data to statistics file");
        exit(1);
    }

    close(fisier);
}

void process_directory(char *numedir, int statistica) 
{
    struct stat dir_info;
    if (stat(numedir, &dir_info) == -1) 
    {
        perror("Error obtaining directory info");
        exit(1);
    }

    char buffer[1024];
    int alo = sprintf(buffer,
                      "nume director: %s\n"
                      "identificatorul utilizatorului: %d\n"
                      "drepturi de acces user: RWX\n"
                      "drepturi de acces grup: R--\n"
                      "drepturi de acces altii: ---\n\n",
                      numedir, dir_info.st_uid);

    int verifica = write(statistica, buffer, alo);
    if (verifica < 0) 
    {
        perror("Eroare la scrierea informatiilor despre director in fisierul de statistica");
        exit(1);
    }
}

void process_symbolic_link(char *numelegatura, int statistica)
{
    struct stat link_info;
    if (lstat(numelegatura, &link_info) == -1) 
    {
        perror("ErOARE la obtinerea informatiilor despre legatura simbolica");
        exit(1);
    }

    struct stat target_info;
    if (stat(numelegatura, &target_info) == -1) 
    {
        perror("Eroare la obtinerea informatiilor despre fisierul tinta din legatura simbolica");
        exit(1);
    }

    char buffer[1024];
    int alo = sprintf(buffer,
                      "nume legatura: %s\n"
                      "dimensiune: %ld\n"
                      "dimensiune fisier: %ld\n"
                      "drepturi de acces user: RWX\n"
                      "drepturi de acces grup: R--\n"
                      "drepturi de acces altii: ---\n\n",
                      numelegatura, link_info.st_size, target_info.st_size);

    int verifica = write(statistica, buffer, alo);
    if (verifica < 0) 
    {
        perror("Eroare la scrierea datelor legaturii simbolice in fisierul de statistici.");
        exit(1);
    }
}

int main(int argc, char *argv[]) 
{
    if (argc != 2) 
    {
        printf("Usage: %s <director_intrare>\n", argv[0]);
        return 1;
    }

    int statistica = open("statistica.txt", O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU);
    if (statistica == -1) 
    {
        perror("Eroare la deschiderea statistica.txt");
        exit(1);
    }

    DIR *dir = opendir(argv[1]);
    if (dir == NULL) 
    {
        perror("Eroare la deschiderea directorului");
        exit(1);
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) 
    {
        if (entry->d_type == DT_REG) 
        {
            char numefisier[1024];
            sprintf(numefisier, "%s/%s", argv[1], entry->d_name);
            process_file(numefisier, statistica);
        } 
        else if (entry->d_type == DT_DIR) 
        {
            char numedir[1024];
            sprintf(numedir, "%s/%s", argv[1], entry->d_name);
            process_directory(numedir, statistica);
        } 
        else if (entry->d_type == DT_LNK) 
        {
            char numelegatura[1024];
            sprintf(numelegatura, "%s/%s", argv[1], entry->d_name);
            process_symbolic_link(numelegatura, statistica);
        }
    }

    closedir(dir);
    close(statistica);

    return EXIT_SUCCESS;
}
