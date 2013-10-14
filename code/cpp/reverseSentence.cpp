#include<iostream>

using namespace std;

void reverse(char *begin, char* end){
    if(!begin || !end)
        return;
    while(begin < end){
        swap(*begin++,*end--);
    }
}

char* reverseSentence(char *str, const char ch = ' '){
    if(!str)
        return NULL;
    char *begin = str, *end = str;
    while(*end++ != '\0'){}
    end -= 2;
    reverse(begin,end);

    begin = end = str;
    while(*begin != '\0'){
        if(*begin == ch){
            ++begin;
            ++end;
        }else if(*end == ch || *end == '\0'){
            reverse(begin,--end);
            begin = ++end;
        }else{
            ++end;
        }
    }
    return str;
}

void test_reverse(){
    char str[] = {"world hello!"};
    cout << str << endl;
    cout << reverseSentence(str) << endl;
}

int main(){
    test_reverse();
    return 0;
}
