#include<iostream>
#include<iterator>
#include<cstdlib>

using namespace std;

void print(int* array, int len){
    if(array == NULL || len <= 0)
        return;
    copy(array,array+len, ostream_iterator<int,char>(cout," "));
    cout << endl;
}

void shufflearray(int* array, int len){
    if(array == NULL || len <= 0)
        return;
    int i = len, j;
    while(--i){
        j = rand() % (i+1);
        swap(array[i],array[j]);
    }
}

void test_shufflearray(){
    int array[] = {1,2,3,4,5,6,7}; 
    int len = sizeof(array) / sizeof(int);
    print(array, len);
    shufflearray(array,len);
    print(array, len);
}

int main(){
    test_shufflearray();
    return 0;
}
