#include <iostream>
#include <stdio.h>
#include <string>
#include <unistd.h>
#include <sys/wait.h>

using namespace std;



//! Gelen stringin içeriğinin sayı olup olmadığına karar verir. (Hazır olarak kullanıldı: StackOverFlow)
/*!
  \param s string değer
  \return true/false
*/
bool is_number(const std::string& s)
{
    std::string::const_iterator it = s.begin();
    while (it != s.end() && std::isdigit(*it)) ++it;
    return !s.empty() && it == s.end();
}



//! Argümanların uygunluğunu test edip geriye true/false olarak sonuç bildirir.
/*!
  \param argc argüman sayısı
  \param *argv[] argümanların tutuldugu dizi
  \return argümanların uygunluğuna göre true/false
*/
bool controlArgs(int argc, char *argv[]){
    if (argc < 2){
        cout << "Parametre vermelisiniz." << endl;
        return false; // parametre yok.
    }

    if (argc > 2){
        cout << "Sadece 1 adet parametre vermelisiniz." << endl;
        return false; // parametre fazla
    }

    if (!is_number(argv[1])){
        cout << "Parametreyi integer olarak vermelisiniz." << endl;
        return false; // sayı değil
    }

    if (stoi(argv[1]) < 2){
        cout << "Parametreye 1 den buyuk bir deger vermelisiniz." << endl;
        return false; // 2 den küçük
    }

    return true;
}




int main(int argc, char *argv[]){
    if(!controlArgs(argc, argv)){
        exit(0);
    }



    int depth = stoi(argv[1]);
    bool isForked = false;
    bool isOkeyFirstChilds = false;

    string terminalCode;

    int pid = 0;
    int mainPID = getpid();
    int thisPID = 0;


    // Klasör ve dosya açma kısmı
    terminalCode = "mkdir logs; cd logs; touch logAll.log";
    system(terminalCode.c_str());


    // !!! NOT: Processlerin hepsi oluşup bittiğinde istenen ağaç modelinde olmuş oluyor ancak senkronizasyon sağlayamadığım için doğru sırada bitmiyorlar.
    //          Sleep() leri doğru konumlandıramadım sanırım. Bu sebeple pstree eksik gösteriyor. Çalışma sıralarını düşünemediğim için debug etmekte de zorlandım.
    for (int i = 0; i < depth; i++) {
        thisPID = getpid();

        if ((isForked == false) || (mainPID == thisPID && i == 1)){ // Fork yapan agaçlara bir daha fork yaptırmaz. En üstteki process için bir kez daha izin verir, dallanma uygun şekilde yapılır.
            if (mainPID == thisPID && i == 1){ // En üstteki processin 2 adet çocuğu olduğunda bu bilgi tutulur.
                isOkeyFirstChilds = true;
            }
            pid = fork();
        }else{
            continue;
        }




        if (pid > 0) { // parent
            isForked = true;
            cout << "PID: " << pid << ", " << "PPID: " << getpid() << " forked..." << endl;
            if (mainPID == thisPID && i == 0){
                sleep(1);
                terminalCode = "pstree " + to_string(mainPID) + " -p";
                cout << endl << endl;
                system(terminalCode.c_str());
                cout << endl << endl;

                terminalCode = "date >> logs/logAll.log; pstree " + to_string(mainPID) + " -p >> logs/logAll.log";
                system(terminalCode.c_str());
            }else{
                sleep(2);
            }
        }else if (pid == 0) { // child
            if (isOkeyFirstChilds == true) { // Sol çocuğun daha fazla fork yapmasına müsade etmez.
                isForked = true;
                sleep(2);
            } else {
                isForked = false;
            }

            if(i == (depth-1)){
                sleep(2);
            }
        }else { // error
            cout << "An error has occurred during forking the process." << endl;
        }

    }




    cout << "PID: " << getpid() << " terminated..." << endl;




    return 0;
}
