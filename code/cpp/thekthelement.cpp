#include<iostream>
#include<set>
#include<iterator>

using namespace std;

void print(int *array, int len){
    if(array == NULL || len <= 0)
        return;
    copy(array,array+len,ostream_iterator<int, char>(cout," "));
    cout << endl;
}

int partition(int *array, int start, int end){
    if(!array || start > end)
        return -1;
    int key = array[start];
    while(start < end){
        while(end > start && array[end] >= key){
            --end;
        }
        array[start] = array[end];
        while(start < end && array[start] <= key){
            ++start;
        }
        array[end] = array[start];
    }
    array[start] = key;
    return start;
}

int thekthelement(int* array, int len, int k){
    if(array == NULL || len <= 0 || k > len || k < 0)
        return -1;
    int start = 0, end = len - 1;
    
    int index = partition(array,start,end);
    while(index != k-1){
        if(index < k - 1){
            start = index + 1;
            index = partition(array,start,end);
        }else if(index > k - 1){
            end = index - 1;
            index = partition(array,start,end);
        }
    }
    return array[index];
}

int thekthelementv2(int *array, int len, int k){
    if(array == NULL || len <= 0 || k > len || k < 1)
        return -1;
    multiset<int,greater<int> > kset;
    multiset<int>::iterator iter;
    int i = 0;
    while(i < len){
        if(kset.size() < k){
            kset.insert(array[i]);
        }else{
            iter = kset.begin();
            if(*iter > array[i]){
                kset.erase(iter);
                kset.insert(array[i]);
            }
        }
        ++i;
    }
    iter = kset.begin();
    return *iter;
}

void test_thekthelement(){
    int array[] = {4,5,1,6,2,7,3,8};    
    int len = sizeof(array) / sizeof(int);
    print(array,len);
    int k;
    while(cin>>k){
        cout << "kth v1:" << thekthelement(array,len,k) << endl;
        cout << "kth v2:" << thekthelementv2(array,len,k) << endl;
        print(array,len);
    }
}

int main(){
    test_thekthelement();
    return 0;
}
