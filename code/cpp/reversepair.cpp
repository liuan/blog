#include<iostream>
#include<cstdlib>
#include<iterator>

using namespace std;


void print(const string& str,int *array, int len){
    cout << str << ":" << endl;
    copy(array,array+len,ostream_iterator<int,char>(cout," "));
    cout << endl;
}

void swap(int& a, int& b){
    if(&a == &b)
        return;
    a = a^b;
    b = a^b;
    a = a^b;
}

void merge(int *array, int start,int middle, int end,int* temp, int* pairs){
    //cout << "pairs: " << *pairs  << " middle:" << array[middle] << endl;
    //print("test",array+start,end-start+1);
    if(array == NULL || start < 0 || end < 0 || start == end)
        return;
    int i = middle, j = end,index = 0; 
    while(i >= start && j > middle){
        if(array[i] <= array[j]){
            temp[index++] = array[j--];
        }else{
            temp[index++] = array[i--];
            *pairs += j - middle;
        }
    }
    while(i >= start){
        temp[index++] = array[i--];
    }
    while(j > middle){
        temp[index++] = array[j--];
    }
    for(i = start; i <= end; i++)
        array[i] = temp[--index];
}

void reversepair(int *array, int start, int end,int* temp, int* pairs){
    if(array == NULL || start < 0 || end < 0)
        return;
    if(start < end){
        int middle = (start+end)>>1;
        reversepair(array,start,middle,temp,pairs);
        reversepair(array,middle+1,end,temp,pairs);
        merge(array,start,middle,end,temp,pairs);
    }
}

void generatedata(int *array, int len){
    if(array == NULL || len <= 0)
        return;
    //generate random numbers;
    int upper = len * 3;
    int size = len;
    int i = 0;
    for(i; i < len * 3; i++){
        int ran = rand() % upper;
        if(ran < size){
            array[--size] = i; 
        }
        --upper;
    }

    print("generate",array,len);

    // swap the data
    for(i = 0; i < len; i++){
        swap(array[i],array[rand()%(len-i) + i]);
    }
    print("data:",array,len);
}

void testreversepairv2(){
    enum {len = 7};
    int array[] = {1,2,15,17,7,8,16,18}; 
    int temp[len];
    int pairs = 0;
    cout << "*******************************" << endl;
    print("array",array,len);
    reversepair(array,0,len-1,temp,&pairs);
    cout << "pairs:" << pairs << endl;
}

void testreversepair(){
    enum {len = 10};
    int array[len];
    generatedata(array,len);
    int temp[len];
    int pairs = 0;
    reversepair(array,0,len-1,temp,&pairs);
    print("merge",array,len);
    cout << "reversepairs:" << pairs << endl;
}

int main(){
    testreversepair();
    testreversepairv2();
    return 0;
}
