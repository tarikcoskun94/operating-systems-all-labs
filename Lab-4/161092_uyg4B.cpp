#include <iostream>
#include <string>
#include <cmath>
#include <fstream>
#include <unistd.h>
#include <csignal>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <ctime>
#include <chrono>
#include <thread>
#include <mutex>
#include <condition_variable>

using namespace std;



/* Kritik bölgelerde threadlar arasında senkronizasyon sağlamak için */
mutex mtx;
condition_variable conVar;
/************************************************************************/


bool changeIs = false; // Dosya içeriği değişti ise, uygunluk kontrolü yapan threade şunu söyler: "Değişim oldu, yeni değeri kontrol et."
bool suitableValue = false; // Dosya içeriği uygun ise, değişim kontrolü yapan threade şunu söyler: "Bu değer uygundur, kontrol etmeyi bırak."

bool alarmHadler = false; // Signal handle edildiğinde, döngüyü kırmak için.

int pid;


/************ Fonksiyon prototipleri ***********/
void fakeHandler(int);
void killChild (int);
void protectTermSusp (int);

void printLastModifiedTime(string);
void printLastUpdatedTime();

string readFile (const string&);

int* parseTheInputValue (const long int&);
void printParsedInput (const int*);

void checkTheChanges(const string&, long int&);
void checkTheInputValue(const long int&);
/***********************************************/





int main(int argc, char **argv){
    string theFilePath; // Argümandan gelen dosya yolu için. Okunurluğu artırmak amaçlı değişkene eşitliyorum.
    long int inputValue;  // Değişim kontrol eden ve uygunluk kontrol eden threadlerin ortak kullanımı için. Okunan input, adres üzerinden buna eşitlenir.
    int* parsedInput;   // Parse edilen değerlerin tutulması için.

    int pipeManager[2];


    
    if (argc == 1) { // Argümansız giriş hatası
        cout << "You did not enter any argument!" << endl;
        exit(0);
    }
    
    if (argc > 2) { // 1'den fazla argüman hatası
        cout << "You only need to enter one argument." << endl;
        exit(0);
    }

    if(pipe(pipeManager) == -1){ // Pipe oluşturma hatası
        cout << "pipe failed" << endl;
        exit(0);
    }

    cout << "Pipe was created successfully..." << endl;

    cout << "Parent " << "\t\t" << " -> My PID is " << getpid() << ", PPID is " << getppid() << endl;

    pid = fork();

    if (pid < 0) { // Fork hatası
        cout << "fork failed" << endl;
        exit(0);
    }

    if (pid > 0) { // Parent process
        long int tmp;

        close(pipeManager[1]);
        read(pipeManager[0], &tmp, 8); // 8 byte ile buffer size yapılandırılıp pipe tan veri bekleniyor.
        close(pipeManager[0]);

        cout << "Parent " << "\t\t" << " -> Incoming message from child is: " << tmp << endl;

        cout << "Parent " << "\t\t" << " -> Parsing and printing Student ID (Hardcodded, just for info)" << endl; 

        parsedInput = parseTheInputValue(tmp); // Matematiksel olarak parse ediliyor.

        printParsedInput(parsedInput); // Ayrıştırılmış bilgileri ekrana basar.

        int faculty = parsedInput[0];
        int department = parsedInput[1];
        int entYear = parsedInput[2];

        if ((faculty == 15) && (department == 21)) {
            cout << "Parent " << "\t\t" << " -> Childe process is being waited." << endl;

            wait(NULL);

            cout << "Parent " << "\t\t" << " -> Waiting is done." << endl;

        } else if (entYear >= 2017) {
            signal(SIGALRM, killChild); // Alarm sinyali yakalar.
            alarm(3);

            cout << "Parent " << "\t\t" << " -> I'll kill you when your normal time is up." << endl;

            cout << "Parent " << "\t\t" << " -> Alarm clock signal was scheduled in 4 seconds..." << endl;

            while (!alarmHadler){}

            cout << "Parent " << "\t\t" << " -> Child is killed. Or not?" << endl;

        } else if (faculty != 15) {
            signal(SIGALRM, fakeHandler); // Alarm sinyali yakalar.
            signal(SIGTSTP, protectTermSusp); // Suspend sinyali yakalar
            signal(SIGINT, protectTermSusp); // Termination sinyali yakalar
            alarm(6);

            cout << "Parent " << "\t\t" << " -> Alarm clock signal was scheduled in 10 seconds..." << endl;

            cout << "Parent " << "\t\t" << " -> Only the God can judge me! (freedom for 10 seconds)." << endl;

            while (!alarmHadler){}

        } else {
            signal(SIGTSTP, protectTermSusp); // Suspend sinyali yakalar
            signal(SIGINT, protectTermSusp); // Termination sinyali yakalar
        }

        cout << "Terminating [PID: " << getpid() << "], [PPID: " << getppid() << "]" << endl;

    } else { // Child process
        cout << "Child " << "\t\t" << " -> My PID is " << getpid() << ", PPID is " << getppid() << endl;

        theFilePath = argv[1]; // Dosya yolu. Bu atama işlemi, tamamen kod okunurluğu için.

        thread checkerOfChange(checkTheChanges, ref(theFilePath), ref(inputValue)); // İnput değişim kontrolcüsü. 
        thread checkerOfInput(checkTheInputValue, ref(inputValue)); // İnput uygunluk kontrolcüsü. 
                                                                    // inputValue değişkeni, 2 thread arasında okunan değerin paylaşımı için.
        
        checkerOfChange.join();
        checkerOfInput.join();

        close(pipeManager[0]);
        cout << "Child " << "\t\t" << " -> The message is being send. Warning: Type Constraint!" << endl;
        write(pipeManager[1], &inputValue, 8); // 8 byte ile buffer size yapılandırılıp pipe tan veri yazılıyor.
        close(pipeManager[1]);

        cout << "Child " << "\t\t" << " -> Sleeping 4 sec.s." << endl;

        sleep(4);

        cout << "Child " << "\t\t" << " -> Slept 4 sec.s." << endl;

        cout << "Terminating [PID: " << getpid() << "], [PPID: " << getppid() << "]" << endl;

        exit(0);

    }



    return 0;
}





/****************************** Sinyal handle edilince çalışacak fonksiyonlar ******************************/
void fakeHandler(int sig){
    alarmHadler = true; // Bekleme yapan döngüyü sonlandırır.
    cout << "Alarm Handler " << "\t" << " -> Alarm clock signal has been received." << endl;
}


void killChild (int sig) {
    alarmHadler = true; // Bekleme yapan döngüyü sonlandırır.
    cout << "Alarm Handler " << "\t" << " -> Alarm clock signal has been received." << endl;
    kill(pid, SIGKILL);
}


void protectTermSusp (int sig) {
    cout << "Sig. Handler -> I'll always get suffering (Protection from CTRL+C | CTRL+Z)!!!" << endl;
}
/***********************************************************************************************************/


/**
* \brief Dosyanın en son değişim zamanı
*
* Verilen dosyanın son değişim tarihini ekrana bastırır.
*
* \param filePath: Dosya yolu
*/
void printLastModifiedTime(string filePath) {
    struct stat attr;



    stat(filePath.c_str(), &attr);

    cout << "\t\t" << " -> Last modified time: " << ctime(&attr.st_mtime);
}


/**
* \brief Dosyanın değişikliği için güncel zaman
*
* Dosyada herhangi bir değişiklik olduğunda bu fonksiyon ile güncel zaman ekrana bastırılır.
*
*/
void printLastUpdatedTime() {
    auto timenow = chrono::system_clock::to_time_t(chrono::system_clock::now()); 

    cout << "\t\t" << " --> Last updated time: " << ctime(&timenow);
}


/**
* \brief Dosya okuyucu
*
* Gelen dosya yoluna göre dosyayı açar, veriyi okur ve geriye döndürür.
*
* \param filePath: Dosya yolu
* \return myText : Dosya içeriği
*/
string readFile (const string& filePath) {
    ifstream theFile;
    string myText;



    theFile.open(filePath);

    if (!theFile.is_open()) {
        cout << "The file cannot be opened." << endl;
        exit(0);

    }

    getline (theFile, myText);

    theFile.close();



    return myText;
}


/**
* \brief İnput parse edici
*
* İnput değerini matematiksel olarak parse edip, numeric bir dizi olarak geriye döndürür.
* Sayının başından her defasında kaç digit alınacağı biliniyor. Sayının kendisini, a = [10 ^ (toplam digit adedi - seçilecek digit adedi)] ne bölersek en baştan istediğimiz digitleri alırız.
* Sonra sayıyının başından bu diğitler şu şekilde atılır; Sayının kendisinden, (alınan digit * a) yı çıkarırsak geriye kalan kısmı bırakmış oluruz.
* Aynı mantıkta son kısım kalıncaya kadar işlemler yapılıyor.
* \param inputValue   : Kabul görmüş input değeri
* \return parsedInput : Parse edilmiş numeric dizi
*/
int* parseTheInputValue (const long int& inputValue) {
    long int _inputValue;
    int* parsedInput = new int[4];



    _inputValue = inputValue;

    parsedInput[0] = _inputValue / pow(10, 10);
    _inputValue -= (parsedInput[0] * pow(10, 10));

    parsedInput[1] = _inputValue / pow(10, 8);
    _inputValue -= (parsedInput[1] * pow(10, 8));

    parsedInput[2] = _inputValue / pow(10, 4);
    _inputValue -= (parsedInput[2] * pow(10, 4));

    parsedInput[3] = _inputValue;



    return parsedInput;
}


/**
* \brief Bilgi yazdırıcı
*
* Parse edilmiş verileri mantıklı biçimde ekrana bastırır.
*
* \param parsedInput: Parse edilmiş veriler
*/
void printParsedInput (const int* parsedInput) {
    switch (parsedInput[0]) { // Fakülte
        case 15:
            cout << "\t\t" << "    " << "Faculty: " << "\t\t" << "Engineering Faculty" << endl; 
            break;

        case 30:
            cout << "\t\t" << "    " << "Faculty: " << "\t\t" << "Foreign university" << endl;
            break;

        default:
            cout << "\t\t" << "    " << "Faculty: " << "\t\t" << "Other" << endl;
    }

    switch (parsedInput[1]) { // Bölüm
        case 21:
            cout << "\t\t" << "    " << "Department: " << "\t" << "Computer Engineering" << endl; 
            break;

        case 12:
            cout << "\t\t" << "    " << "Department: " << "\t" << "Electrical Engineering" << endl;
            break;

        default:
            cout << "\t\t" << "    " << "Department: " << "\t" << "Other" << endl;
    }

    cout << "\t\t" << "    " << "Entrance year: " << "\t" << parsedInput[2] << endl; // Giriş yılı
}


/**
* \brief İnput değişim kontrolcüsü
*
* Sürekli olarak dosyayı kontrol eder ve değişim arar. İnput uygunluk kontrolü yapan thread ile senkronizedir ve böylece busy waiting olmaz, CPU israf edilmez.
*
* \param filePath  : Dosya yolu
* \param inputValue: 2 thread arasında okunan değerin paylaşımı için: referans ile return.
*/
void checkTheChanges(const string& filePath, long int& inputValue) {
    string oldValue;
    string newValue;



    newValue = readFile(filePath);
    oldValue = newValue;

    printLastModifiedTime(filePath);

    while(suitableValue == false){ // Okunan değer uygun olmadığı müddetçe...        
        if (newValue != oldValue) {
            oldValue = newValue;

            /*-----<CRITICAL SECTION>-----*/
            unique_lock <mutex> uLock(mtx);
            conVar.wait(uLock, []{return !changeIs;}); // Lock'ı alıp, diğer threadden ilgili değişikliklerin kaydedildiğinin onayı gelmesini bekler. İşlem waiting queue ya atılır.

            inputValue = stol(newValue, nullptr, 10); // Yeni input, long int dönüşümü yapılarak referansa döndürülür.

            changeIs = true; // Değişim olduğu bildirilir.

            printLastUpdatedTime();
            cout << "\t\t" << "     " << filePath << ": " << inputValue << endl;

            conVar.notify_one(); // O halde, sinyal verilerek diğer threadin uygunluk kontolüne devam etmesi sağlanır.
            /*-----</CRITICAL SECTION>-----*/

        }

        newValue = readFile(filePath);

        this_thread::sleep_for(std::chrono::milliseconds(300)); // Değişim olmayan anlarda işlemciyi yormamak adına kullanılan ufak bir gecikme.
    }  
}


/**
* \brief İnput uygunluk kontrolcüsü
*
* Değişim haberi gelince input değerinin uygunluğunu tespit eder. İnput değişim kontrolü yapan thread ile senkronizedir ve böylece busy waiting olmaz, CPU israf edilmez.
*
* \param inputValue: 2 thread arasında okunan değerin paylaşımı için: Değişen veri buradan okunuyor.
*/
void checkTheInputValue(const long int& inputValue) {
    long int _inputValue;
    int digitCTRL; // 12 hane kontrolü için.



    while(1){
        /*-----<CRITICAL SECTION>-----*/
        unique_lock <mutex> uLock(mtx);
        conVar.wait(uLock, []{return changeIs;}); // Lock'ı alıp, diğer threadden değişim onayı gelmesini bekler. İşlem waiting queue ya atılır.

        _inputValue = inputValue; // Yeni input local olarak hafızaya alındı.

        changeIs = false; // Artık yeni bir değişim beklenebileceği bildirilir.

        conVar.notify_one(); // O halde, sinyal verilerek diğer threadin sürekli değişim aramasına devam etmesi sağlanır.
        /*-----</CRITICAL SECTION>-----*/

        digitCTRL = _inputValue / 100000000000; // Tam sayı kısmı kullanılarak kontrol etmek yeterli; [Sayı / (10 ^ (digit - 1))]: 1.00 <= sonuç <= 9,99;

        if (digitCTRL < 1)
            cout << "\t\t" << "     " << "-- Wrong input! - Few digits." << endl;

        else if (digitCTRL > 9)
            cout << "\t\t" << "     " << "-- Wrong input! - Too many digits." << endl;

        else {
            suitableValue = true; // Uygun değer bulunduğunda, diğer threadin değişim arama işlemini bitirmesi için kullanılır.
            break;

        }
    }
}