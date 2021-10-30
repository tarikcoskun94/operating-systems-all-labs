#include <stdio.h>
extern end;
extern etext;
extern edata;

extern void dummyFunc();
extern void testAddress();

main()
{
    char *s;
    char c;

    printf("&dummyFunc() \t= 0x%lx\n", dummyFunc);
    printf("&testAddress() \t= 0x%lx\n", testAddress);
    printf("&main() \t= 0x%lx\n", main);

    printf("&etext \t\t= 0x%lx\n", &etext); // kod segmentinin bittiği yer. Aynı zamanda initialize data segmentin başlangıcı.
    printf("&edata \t\t= 0x%lx\n", &edata); // initialize data segmentinin bittiği yer. Aynı zamanda uninitialize data(bss) segmentin başlangıcı.
    printf("&end \t\t= 0x%lx\n", &end); // uninitialize data(bss) segmentinin bittiği yer. Aynı zamanda heap başlangıcı.
    printf("sbrk(0) \t= 0x%lx\n", sbrk(0)); // hesap sonu. / dinamik olarak
    printf("&c \t\t= 0x%lx\n", &c);
    printf("\n");
    printf("Enter memory location in hex (start with 0x): ");
    fflush(stdout);
    scanf("0x%lx", &s);
    printf("Reading 0x%lx: ", s);
    fflush(stdout);
    c = *s;
    printf("%d\n", c);
    printf("Writing %d back to 0x%lx: ", c, s);
    fflush(stdout);
    *s = c;
    printf("ok\n");
}


void dummyFunc(){

}

void testAddress(){
    
}