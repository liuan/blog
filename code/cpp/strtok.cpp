#include<iostream>
#include<cstring>

using namespace std;

char * __strtok;

char* mystrtok(char *s, const char *ct){
    char *sbegin, *send;
    sbegin = s ? s : __strtok;
    if(!sbegin) // NULL
        return NULL;
    
    while(*sbegin == ' ' && *sbegin != '\0')
        ++sbegin;
    if(*sbegin == '\0'){
        __strtok = NULL;
        return NULL;
    }
    send = sbegin;
    while(*send != '\0' && *send != *ct){
        ++send;
    }
    if(*send != '\0'){
        *send++ = '\0'; 
        __strtok = send;
    }else{
        __strtok = NULL;
    }
    return sbegin;
}

void test_strtok(){
    char s[] = " fuck  you ";
    char split = ' ';
    char *token = strtok(s,&split);
    while(token){
        cout << token << endl;
        strtok(NULL,&split);
    }

}

void test_mystrtok(){
    //char s[] = "where can I go?";
    char s[] = " fuck  you ";
    char split = ' ';
    char *token;
    for(token = mystrtok(s,&split); token != NULL; token = mystrtok(NULL,&split)){
        cout << token << endl;
    }
}

int main(){
    test_strtok();
    cout << "****************************" << endl;
    test_mystrtok();
    return 0;
}
