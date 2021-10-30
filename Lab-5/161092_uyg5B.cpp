#include <iostream>
#include <string>
#include <sstream>
#include <thread>
#include <mutex>
#include <semaphore.h>

using namespace std;



/********************* Bölge isimleri ********************/
#define KAFA_STR    "(kafa )"
#define GOVDE_STR   "(govde)"
#define EL_STR      "(el   )"
#define BACAK_STR   "(bacak)"
#define AYAK_STR    "(ayak )"
/*********************************************************/



/**************** Senkronizasyon elemanları **************/
mutex indexAndMessage_MTX;
/* Mutex kullanmamın 2 sebebi var;
 *
 * 1) İşlemlerden biri index numarasını alırken, araya başka işlem girmemeli. İndexi artırarak mesajların başında kullanıyorum.
 *    Artırma işlemine de assembly seviyesinde bakıldığında 1 adımdan daha fazlası demektir. İşlem bitene kadar araya başkası girmemeli.
 * 
 * 2) Diğer sebep ise ben C++ ile yazdığım için mesajlarımı printf() kullanarak yazdırmayı tercih etmiyorum.
 *    Mesaj bütün hale getirilirken de atomiliği sağlamak için karşılıklı dışlama yapmalıyız ki aynı anda o bölgede tek işlem yürüsün.
 * 
 * NOT: printf() kullansaydım bile, index artırırken yine de bu atomikliği sağlamak zorundaydım. Full mesaj oluşturarak yazdırma ve
 *      index artırmayı bir fonksiyona bağlayıp critical section içinde bu işlemleri yaptırdım.
 * 
 */

sem_t sem1;
sem_t sem2;
sem_t sem3;
sem_t sem4;
sem_t sem5;
sem_t sem6;
/*********************************************************/



int index = 0; // İndex numaralarını tutmak için.



/***************** Fonksiyon prototipleri ****************/
void initializeSemaphores();

string getRemaininItemText (int);
void printFullMessage (string, string, string, string);

void printHeadItems ();
void printBodyItems ();
void printHandItems ();
void printLegItems ();
void printFootItems ();

void destroySemaphores();
/*********************************************************/





int main(int argc, char **argv){
    cout << " " << index << "." << "\t\t\t\t\t" << "--> Saglik calisanlarina sonsuz tesekkurler!.." << endl;

    initializeSemaphores(); // Semaphore'ları yapılandırır.

    thread threadHead(printHeadItems);
    thread threadBody(printBodyItems);
    thread threadHand(printHandItems);
    thread threadLeg (printLegItems );
    thread threadFoot(printFootItems);

    threadHead.join();
    threadBody.join();
    threadHand.join();
    threadLeg. join();
    threadFoot.join();

    destroySemaphores(); // Semaphore'ları yok eder.

    return 0;
}





/**
* \brief Semaphore yapılandırıcı
*
* Tanımlanmış semaphorelar burada toplu bir şekilde initialize edilir.
*
*/
void initializeSemaphores () {
    sem_init(&sem1, 0, 0);
    sem_init(&sem2, 0, 0);
    sem_init(&sem3, 0, 0);
    sem_init(&sem4, 0, 0);
    sem_init(&sem5, 0, 0);
    sem_init(&sem6, 0, 0);
}


/**
* \brief Kalan item bilgilendiricisi
*
* İtem sayısı yarıdan daha az kaldığı vakit, geriye kalan item sayısıyla alakalı bir string döndürür.
*
* \param incomingIndex: İşlemin o an elde ettiği index numarası
* \return : Koşula uygun olarak, kalan item bilgisi veya boş değer
*/
string getRemaininItemText (int incomingIndex) {
    double half;
    int rest;
    string halfInfo;



    half = 13.00/2.00;
    rest = 13-incomingIndex;

    if (rest == 0)
        return "--> Goreve hazirim!";

    if (rest < half) {
        halfInfo = "--> Kalan oge sayisi:  " + to_string(rest);
        return halfInfo;

    }

    return "";
}


/**
* \brief Full mesajı yazdırma ve index artırıcı
*
* C++ ta, printf() yerine cout kullanmayı tercih ettiğimden ötürü, mesaj yazdırmayı bölünemez (atomik) yapmak için kullanır.
* Atomikliğin asıl nedeni aslında index artırımını kontrol etmektir. printf() olsaydı bile yine de index++ için atomiklik zorunludur.
*
* \param bodyPart: Bölge ismi
* \param item    : İtem ismi
* \param space   : Boşluklar için gelen karakter katarı
*/
void printFullMessage (string bodyPart, string item, string space) {
    lock_guard <mutex> atomicPrintLock (indexAndMessage_MTX);
    string indexStr;
    string halfInfo;
    stringstream fullMessage;



    index++;

    if (index < 10) 
        indexStr = " " + to_string(index);

    else
        indexStr = to_string(index);


    halfInfo = getRemaininItemText(index);

    fullMessage << indexStr << ". " << bodyPart << " " << item << space << halfInfo << endl;
    cout << fullMessage.str();
    fullMessage.str(string());
}


/**
* \brief Kafa bölgesi yazdırıcı
*
* Kafa bölgesine ait olan elemanları, uygun senkronizasyon koşullarına uyarak olarak ekrana yazdırır.
*
*/
void printHeadItems () {
    sem_wait(&sem1);
    sem_wait(&sem5);
    printFullMessage(KAFA_STR, "YuzDezenfektani", "\t\t");

    sem_wait(&sem3);
    printFullMessage(KAFA_STR, "SaglikMaskesi", "\t\t");

    printFullMessage(KAFA_STR, "Bone", "\t\t\t");
    sem_post(&sem6);

    sem_wait(&sem1);
    printFullMessage(KAFA_STR, "KoruyucuPlastikYuzMaskesi", "\t");
}


/**
* \brief Gövde bölgesi yazdırıcı
*
* Gövde bölgesine ait olan elemanları, uygun senkronizasyon koşullarına uyarak olarak ekrana yazdırır.
*
*/
void printBodyItems () {
    printFullMessage(GOVDE_STR, "Atlet", "\t\t\t");
    sem_post(&sem1);
    sem_post(&sem1);
    sem_post(&sem1);
    sem_post(&sem1);

    printFullMessage(GOVDE_STR, "Gomlek", "\t\t\t");
    sem_post(&sem3);

    sem_wait(&sem6);
    printFullMessage(GOVDE_STR, "Tulum", "\t\t\t");
    sem_post(&sem1);
}


/**
* \brief El bölgesi yazdırıcı
*
* El bölgesine ait olan elemanları, uygun senkronizasyon koşullarına uyarak olarak ekrana yazdırır.
*
*/
void printHandItems () {
    sem_wait(&sem1);
    sem_wait(&sem5);
    printFullMessage(EL_STR, "ElDezenfektani", "\t\t");

    sem_wait(&sem1);
    printFullMessage(EL_STR, "Eldiven", "\t\t\t");
    sem_post(&sem3);
}


/**
* \brief Bacak bölgesi yazdırıcı
*
* Bacak bölgesine ait olan elemanları, uygun senkronizasyon koşullarına uyarak olarak ekrana yazdırır.
*
*/
void printLegItems () {
    sem_wait(&sem1);
    sem_wait(&sem2);
    sem_wait(&sem3);
    printFullMessage(BACAK_STR, "Pantolon", "\t\t\t");
    sem_post(&sem4);

    printFullMessage(BACAK_STR, "Kemer", "\t\t\t");
    sem_post(&sem1);
}


/**
* \brief Ayak bölgesi yazdırıcı
*
* Ayak bölgesine ait olan elemanları, uygun senkronizasyon koşullarına uyarak olarak ekrana yazdırır.
*
*/
void printFootItems () {
    sem_wait(&sem1);
    printFullMessage(AYAK_STR, "Corap", "\t\t\t");
    sem_post(&sem2);

    sem_wait(&sem4);
    printFullMessage(AYAK_STR, "Ayakkabi", "\t\t\t");
    sem_post(&sem5);
    sem_post(&sem5);
}


/**
* \brief Semaphore yok edici
*
* Tanımlanmış semaphorelar burada toplu bir şekilde destroy edilir.
*
*/
void destroySemaphores() {
    sem_destroy(&sem1);
    sem_destroy(&sem2);
    sem_destroy(&sem3);
    sem_destroy(&sem4);
    sem_destroy(&sem5);
    sem_destroy(&sem6);
}