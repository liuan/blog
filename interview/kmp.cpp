#include<iostream>
#include<cstring>

using namespace std;

template<typename T>
void print(T *set, int len){
    if(set == NULL || len <= 0)
        return;
    int i = 0;
    for(i = 0; i < len; i++){
        cout << set[i] << " ";
    }
    cout << endl;
}

void nextFunc(const char *pattern,int *next,int len){
    int i = 0;
    for(i = 0; i < len; i++)
        next[i] = 0;
    for(i = 2; i < len; i++){
        if(pattern[i-1] == pattern[next[i-1]])
            next[i] = next[i-1] + 1;
        else if(pattern[i-1] == pattern[0])
            next[i] = 1;
        else 
            next[i] = 0;
    }
    cout << "next:" << endl;
    print(next, len);
}

int match(const char *str, const char *pattern, const int* next,int len){
    int i = 0,j = 0;
    while(str[i] != '\0'){
        int temp = i;
        while(i < strlen(str) && j < len && str[i] == pattern[j]){
            i++;
            j++;
        }
        if(j == len){
            return i - j;
        }else if(i == strlen(str)){
            return -1;
        }else{
            if(i == temp)
                i++;
            j = next[j];
        }
    }
    return -1;
}

int main(){
    //const char str[] = {"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaab"};
    const char str[] = {"bbc abcdab abcdabcdabde"};
    cout << str << endl;
    string pattern;
    while(getline(cin,pattern)){
        cout << str << endl;
        cout << "pattern:" << pattern << endl;
        int *next = new int[pattern.size()];
        nextFunc(pattern.c_str(),next,pattern.size());
        int index = match(str,pattern.c_str(),next,pattern.size()); 
        if(index == -1)
            cout << "can't match this string:" << pattern << endl;
        else
            cout << "match index:" << index << endl;
        delete [] next;
    }

    return 0;
}
