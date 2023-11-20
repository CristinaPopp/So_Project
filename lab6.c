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

typedef struct header
{
    int file_size;
    int width;
    int height;
    int image_size;
} header;


int main(int argc, char *argv[])
{

    if (argc != 2)
    {
        printf("Nu avem suficiente argumente in linia de comada\n");
        return EXIT_FAILURE;
    }

    int fisier = open(argv[1], O_RDONLY);
    
    char signature[2];
    read(fisier, signature, 2);

    if (signature[0] != 'B' || signature[1] != 'M')
    {
       return EXIT_FAILURE;
    }

    
    int statistica = open("statistica.txt", O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU);
    if (statistica == -1)
    {
        perror("Eroare deschidere statistica.txt");
        return EXIT_FAILURE;
    }

    int fis = open(argv[1] , O_RDONLY);
    header header ;
    
    lseek(fis, 2, SEEK_SET);
                                        
    read(fis, header.file_size, 4); 

    lseek(fis, 18, SEEK_SET); 
                                
    read(fis, header.width, 4);
    read(fis, header.height, 4);

    
    lseek(fis, 34, SEEK_SET); 
    read(fis, header.image_size,4);


    struct stat BMPinfo;
    char timp[200];
    strftime(timp, sizeof(timp), "%d-%m-%Y", localtime(&BMPinfo.st_mtime));

    if (stat(fis, &BMPinfo) == -1) //daca nu am reuist sa aflu toate info
    {
        perror("Eroare la obtinere info");
        exit(1);
    }
    char buffer[1024];
    int var = sprintf(buffer,
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
                    argv[1], header.height, header.width, header.file_size, BMPinfo.st_uid,
                    timp,
                    BMPinfo.st_nlink,
                    (BMPinfo.st_mode & S_IRUSR) ? 'R' : '-',
                    (BMPinfo.st_mode & S_IWUSR) ? 'W' : '-',
                    (BMPinfo.st_mode & S_IXUSR) ? 'X' : '-',
                    (BMPinfo.st_mode & S_IRGRP) ? 'R' : '-', //operator ternal
                    (BMPinfo.st_mode & S_IWGRP) ? 'W' : '-',
                    (BMPinfo.st_mode & S_IXGRP) ? 'X' : '-',
                    (BMPinfo.st_mode & S_IROTH) ? 'R' : '-',
                    (BMPinfo.st_mode & S_IWOTH) ? 'W' : '-',
                    (BMPinfo.st_mode & S_IXOTH) ? 'X' : '-');
                    int verify = write(statistica, buffer, var);
                    if (verify < 0)
                    {
                        perror("Eroare la scrierea datelor despre director in fisier");
                        exit(1);
                    }
                    close(fis);
                    close(statistica);
}
