#include <iostream>
#include <string>
using namespace std;

int main ( ){
    int X;

    cout<<"Bir sayi giriniz: ";
    cin>>X;

    for(int i=2; i<=X; i++){
        if(X%i==0){
            cout<<i<< " ";
            X=X/i;
            i--;
        }
    }

    cout << endl;

    return 0;
}
