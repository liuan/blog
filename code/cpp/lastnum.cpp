#include<iostream>
#include<iterator>
#include<list>

using namespace std;

int lastnumv1(int *array, int len, int steps){
    if(!array || len <= 0 || steps < 0)
        return -1;
    list<int> circle;
    int i = 0, k = 0;
    for(i; i < len; i++){
        circle.push_back(array[i]);
    }

    list<int>::iterator iter = circle.begin();
    while(circle.size() > 1){
        for(k = 1; k < steps; ++k){
            if(iter == circle.end()){
                iter = circle.begin();
            }
            ++iter;
        }
        if(iter == circle.end()){
            iter = circle.begin();
        }
        circle.erase(iter++);
    }

    iter = circle.begin();

    return *iter;
}

int lastnumv2(int *array, int len, int steps){
    if(!array || len <= 0 || steps < 0)
        return -1;
    int last = 0, i = 0;
    for(i = 2; i <= len; i++)
        last = (last + steps) % i;
    return array[last];
}

int lastnumv3(int *array, int len, int steps){
    if(array == NULL || len <= 0)
        return -1;

    int i = 0, j = 0, k = 0, circle = 0;
    int *temp = new int[len];
    for(i; i < len; i++)
        temp[i] = array[i];

    i = 0;

    while(circle < len - 1){
        k = 0;
        while(k < steps){
            if(j == len){
                j = 0;
            }
            if(temp[j] != 0){
                ++k;
            }
            ++j;
        } 
        temp[j-1] = 0; // we do not need think out j-1 will make segmentfault. because with the ending of while loop, ++j must > 0
        ++circle;
    }

    for(i = 0; i < len; i++){
        if(temp[i] != 0)
            break;
    }
    
    delete [] temp;

    return array[i]; 
}

void test_v1(int *array, int len, int steps){
    cout << "list version, last numbers:" << lastnumv1(array,len,steps) << endl;
}

void test_v2(int *array, int len, int steps){
    cout << "func version, last numbers:" << lastnumv2(array,len,steps) << endl;
}

void test_v3(int *array, int len, int steps){
    cout << "array version, last numbers:" << lastnumv3(array,len,steps) << endl;
}

int main(){
    int array[] = {1,2,3,4,5};
    int len = sizeof(array) / sizeof(int);
    copy(array,array+len,ostream_iterator<int,char>(cout," "));
    cout << endl;
    int steps;
    while(cin>>steps){
        test_v1(array,len,steps);
        test_v2(array,len,steps);
        test_v3(array,len,steps);
    }
    return 0;
}
