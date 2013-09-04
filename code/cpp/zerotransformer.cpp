/***********************************************************
 * Given two random sequences which contain N elments.
 * The elements is in a range of 0 to n-1 and the two
 * sequences has no repeated elements. 
 * eg: S = [1,2,0], T = [2,1,0]
 * Now, we need tranform the S sequence to T sequence,
 * but only can use the '0' element to exchange with 
 * other elements.
 * eg: T --> S. 1. [1,2,0] -> [1,0,2] -> [0,1,2] -> [2,1,0]
 ***********************************************************/ 


#include<iostream>
#include<iterator>

using namespace std;

void swap(int& a, int& b){
    if(&a == &b)
        return;
    a = a^b;
    b = a^b;
    a = a^b;
}

void print(const int *array, int len, const string& tag){
    if(array == NULL || len <= 0)
        return;
    cout << tag << ": ";
    copy(array,array+len,ostream_iterator<int,char>(cout," "));
    cout << endl;
}

void zerotransformer(int *source, const int *target, int len){
    if(source == NULL || target == NULL || len <= 1)
        return;
    int *index = new int[len];
    int i = 0;
    for(i; i < len; i++){
        index[source[i]] = i;
    }
    print(index,len,"index");
    int temp;
    for(i = len - 1; i >= 0; i--){
        if(target[i] != source[i]){
            if(source[i] == 0){
                temp = index[target[i]]; 
                swap(source[index[target[i]]],source[i]);
                index[target[i]] = target[i]; 
                index[0] = temp;
            }else{
                // first swap(source[i],source[index[0]])
                temp = index[0];
                swap(index[0],index[source[i]]);
                swap(source[i],source[temp]);

                // second swap(source[index[target[i]]],source[index[0]]);
                swap(source[index[target[i]]],source[index[0]]);
                swap(index[target[i]],index[0]); 
            }
        }
    }

    print(target,len,"target");
    print(source,len,"source");

    delete [] index;
}

int main(){
    //int source[] = {0,1,2};
    //int target[] = {0,2,1};
    int source[] = {3,2,0,1,4};
    int target[] = {0,2,4,1,3};
    int len = sizeof(source) / sizeof(int);

    print(source,len,"source");
    print(target,len,"target");

    zerotransformer(source,target,len);
    return 0;
}
