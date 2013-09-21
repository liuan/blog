#include<iostream>
#include<cstring>
#include<iterator>

using namespace std;

int min(int left, int right){
    return left < right ? left : right; 
}

int maxmanachersubstring(char *str, int len){
    if(str == NULL || len <= 0)
        return -1;
    int newlen = 2*len + 1; 
    char *newstr = new char[newlen];
    int i = 0;
    for(i; i < len; i++){
        newstr[2*i] = '#';
        newstr[2*i+1] = str[i];
    }
    newstr[2*i] = '#';

    int *record = new int[newlen];
    memset(record,0,sizeof(int)*newlen);
    int max = 0, id = 0;

    for(i = 0; i < newlen; i++){
        record[i] = max > i ? min(record[2*id - i], max - i) : 1;
        while(i - record[i] >= 0 && i + record[i] < newlen && 
                newstr[i+record[i]] == newstr[i-record[i]]){
            ++record[i];
        }
        if(i + record[i] > max){
            max = i + record[i];
            id = i;
        }
    }

    copy(record,record+newlen,ostream_iterator<int,char>(cout," "));
    cout << endl;

    max = 0;
    for(i = 0; i < newlen; ++i){  // the max record - 1 will the max sub manacher string
        if(max < record[i]){
            max = record[i];
            id = i;
        }
    }

    delete [] record;
    return max - 1;
}

int main(){
    char str[] = {"abcdcfcdcbcdcfc"};    
    //char str[] = {"abcba"};
    //char str[] = {"abba"};
    cout << str << endl;
    cout << maxmanachersubstring(str,strlen(str)) << endl;
    return 0;
}
