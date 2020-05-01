//
// BLG312 Bilgisayar Isletim Sistemleri - Odev 2
// Gizem Ece Avsar 040140303
//
// Gelistirme ortami: ITU Linux serveri
// Thread model: posix
// gcc version 4.8.5 20150623 (Red Hat 4.8.5-39) (GCC) 
// Linux kernel version: 3.10.0-1062.12.1.el7.x86_64
// Derleme komutu: gcc os-hw2.c -pthread
// Calistirma komutu: ./a.out <alt-sinir> <ust-sinir> <proses-sayisi> <iplik-sayisi>
//

#include <stdio.h>
#include <math.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h> 
#include <sys/wait.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <semaphore.h>
#include <fcntl.h>

//essiz anahtarlar
#define ARRAY_KEY ftok(get_current_dir_name(), 6)
#define INDEX_KEY ftok(get_current_dir_name(), 7)
#define SEM_KEY ftok(get_current_dir_name(), 8)

//isaretcilerin ve id'lerin ilk deger atamalari
int * iter = NULL;
int * arrayIndex = NULL;
sem_t * semaphore = NULL;

int ARRAY_ID = 0;
int INDEX_ID = 0;
int SEM_ID = 0;

void * findPrimes(void * args){
    int number = 0;
    int * argsInInt = (int*)args;
    for(number = argsInInt[0]; number<=argsInInt[1]; number++){
        int i = 0;
        int flag = 0;
        if(number == 0 || number == 1) continue;
        if(number == 2 || number == 3) flag = 0;
        //her bir sayi icin basit bolme ile kontrol saglanir
        //ilk asal olan 2'den baslanip sayinin karekokune kadar bakilirsa yeterli olacaktir
        for(i = 2; i * i <= number; i++){ 
            //derlerken math basligini linklemek gerektigi icin sqrt fonksiyonu yerine bu gosterim tercih edildi
            if(number % i == 0){
                flag = 1;
                break;
            }
        }
        if(!flag){
            sem_wait(semaphore); //semafor bekleme durumunda
            iter[*arrayIndex] = number;
            *arrayIndex = (*arrayIndex) + 1;
            sem_post(semaphore); //semafor isaret gonderme durumunda (signal)
        }
    }
    pthread_exit(NULL);
}

int main(int argc, const char * argv[]){

    if(argc < 5){
        printf("Yeterli sayida giris yok\n");
        return 0;
    }
    int min = atoi(argv[1]);
    int max = atoi(argv[2]);
    int np = atoi(argv[3]);
    int nt = atoi(argv[4]);
    
    //asal sayilarin saklanacagi dizi icin paylasimli bellegin alinmasi
    ARRAY_ID = shmget(ARRAY_KEY, (sizeof(int) * max - min + 1), 0700|IPC_CREAT);
    //bu bellek alaninda yazma islemi yapabilmek icin indeks degerinin paylasimli bellege yazilmasi
    INDEX_ID = shmget(INDEX_KEY, sizeof(int), 0700|IPC_CREAT);
    //semafor icin de paylasimli bellekte yer alinmasi
    SEM_ID = shmget(SEM_KEY, sizeof(sem_t), 0700|IPC_CREAT);

    //yukaridaki alanlarin baslangic adreslerini gösteren isaretciler
    iter = (int*)shmat(ARRAY_ID, NULL, 0);
    arrayIndex = (int*)shmat(INDEX_ID, NULL, 0);
    semaphore = (sem_t*)shmat(INDEX_ID, NULL, 0);

    *arrayIndex = 0;
    *iter = 0;

    //posix semaforlari
    //fonksiyon argumanlari:
    //1. sem_t turune ait bir paylasimli bellek alani isaretcisi
    //2. bu semaforun prosesler arası paylasilacagi bilgisi
    //3. semaforun ilk degeri
    if(sem_init(semaphore, 1, 1)== -1){
        printf("Semafor atamasinda hata.\n");
        return 0;
    } 

    //asal sayilarin bulunacagi araliklarin belirlenmesi
    int i = 0;
    int minBoundForProcess = 0;
    int maxBoundForProcess = 0;
    int minBoundForThread = 0;
    int maxBoundForThread = 0;
    int processInterval = (max - min)/np;
    int threadInterval = processInterval/nt;
    
    printf("Ana proses basladi.\n");
    for(; i<np; i++){ 
        minBoundForProcess = min + (i * (processInterval+1));
        maxBoundForProcess = minBoundForProcess + processInterval;
        printf("Proses %d basladi, aralik %d-%d\n", i+1, minBoundForProcess, maxBoundForProcess);
        int child = fork(); //proses yaratilir
        if(child == 0){ //cocuk prosesin icinde olunursa islem yapilir ve cikilir. anne ise fork() islemine devam eder.
                        //bu sayede istenen sayida proses dallanma olmadan elde edilir.
            pthread_t threads[nt];
            int j = 0;
            void * threadResult;
            int args[nt][2]; //olasi bir yaris kosulunu engellemek icin arguman dizileri önceden tanimlandi
            for(; j < nt; j++) {
                minBoundForThread = minBoundForProcess + (j*(threadInterval+1));
                maxBoundForThread = minBoundForThread + threadInterval;
                printf("Iplik %d.%d basladi, aralik %d-%d\n", i+1, j+1, minBoundForThread, maxBoundForThread);
                args[j][0] = minBoundForThread; //iplik fonksiyonu icin argumanlar
                args[j][1] = maxBoundForThread;
                pthread_create(&threads[i], NULL, findPrimes, (void *)(args[j])); //iplik olusturulur
                printf("Iplik %d.%d sonlandi.\n", i+1, j+1);
            }            
            for(j = 0; j < nt; j++ ) { 
                pthread_join(threads[i],  &threadResult); //prosese ait iplikler birlestirilir
            }
            printf("Proses %d sonlandi.\n", i+1);
            exit(0);
        }
    }   
    sleep(5); //butun proseslerin calismasinin tamamlanmasi beklenir

    int k = 0; //bulunan sayilar paylasimli bellek uzerinde siralanir (bubble sort)
    int m = 0;
    int temp = 0;


    for(; k<*arrayIndex && iter[k]; k++){
        for(m = k; m<*arrayIndex && iter[m]; m++){
            if(iter[m] < iter[k]){
                temp = iter[k];
                iter[k] = iter[m];
                iter[m] = temp;
            }
        }
    }

    printf("Ana proses sonlandi. Bulunan asal sayilar: \n"); //asal sayilar paylasimli bellekten yazdirilir
    for(k = 0; k<*arrayIndex; k++){
        if(iter[k]) printf("%d ", iter[k]);
    }

    printf("\n");

    //kaynaklar iade edilir
    shmdt(arrayIndex);
    shmdt(iter);
    shmctl(ARRAY_ID, IPC_RMID, 0);
    shmctl(INDEX_ID, IPC_RMID, 0);
    sem_close(semaphore);
    sem_destroy(semaphore);
    return 0;
}
