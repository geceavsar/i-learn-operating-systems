#include <stdio.h> 
#include <unistd.h>
#include <stdlib.h> 
#include <sys/wait.h>

int main(void){
    int root_pid = getpid();
    printf("Agacin kokunun PID'si: %d\n", root_pid);
    sleep(1);
    int pid1 = fork();
    if(pid1)  printf("Ben %d, annem %d, cocugum %d\n", getpid(), getppid(), pid1); 
    int pid2 = fork();
    if(pid2)  printf("Ben %d, annem %d, cocugum %d\n", getpid(), getppid(), pid2); 
    int pid3 = fork();    
    if(pid3)  printf("Ben %d, annem %d, cocugum %d\n", getpid(), getppid(), pid3);

    // bu asamaya kadar toplam 7 proses olusturuldu, kok ile birlikte 8 proses calisir durumda.
    // bundan sonra kalan 5 proses olusturulacak.

    if(getppid() == root_pid){

        if(!pid3){ //kokun en son olan cocugundan dallandirilacak prosesler bu kapsamda olusturulur.
            int pid4 = fork(); //ilk cocuk proses
            if(pid4)  printf("Ben %d, annem %d, cocugum %d\n", getpid(), getppid(), pid4); //anneden cagri
            else printf("Ben %d, annem %d, cocugum yok\n", getpid(), getppid()); //cocuktan cagri
            int pid5 = fork(); //ikinci cocuk proses ve ilk cocugun cocugu
            if(pid5)  printf("Ben %d, annem %d, cocugum %d\n", getpid(), getppid(), pid5); //anneden cagri
            else printf("Ben %d, annem %d, cocugum yok\n", getpid(), getppid()); //cocuktan cagri
        }
        
        if(!pid2){ //kokun ikinci cocugunun ikinci cocugu olusturulacak
            int pid4 = fork();  //ikinci cocuk
            if(pid4)  printf("Ben %d, annem %d, cocugum %d\n", getpid(), getppid(), pid4); //anneden cagri
            else{ //cocuksa bir tane daha cocugu olmalı
                int pid5 = fork();    
                if(pid5)  printf("Ben %d, annem %d, cocugum %d\n", getpid(), getppid(), pid5); //anneden cagri
                else printf("Ben %d, annem %d, cocugum yok\n", getpid(), getppid()); //cocuktan cagri
            }
        }
    }

    sleep(5);
    return 0;
}