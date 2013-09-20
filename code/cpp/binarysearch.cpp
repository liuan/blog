#include<iostream>
#include<iterator>

using namespace std;

void swap(int& left, int& right){
    if(&left == &right)
        return;
    left = left^right;
    right = left^right;
    left = left^right;
}

void print(int *array, int len){
    if(array == NULL || len <= 0)
        return;
    copy(array, array+len, ostream_iterator<int,char>(cout," "));
    cout << endl;
}

void swap(int *array, int start, int end){
    if(array == NULL || start > end){
        return;
    }
    while(start < end){
        swap(array[start++],array[end--]);
    }
}

void rotate(int *array, int len, int position){
    if(array == NULL || len <= 0 || position >= len || position <= 0)
        return;
    swap(array,0,position);
    swap(array,position+1,len-1);
    swap(array,0,len-1);
}

void test_rotate(int *array, int len){
    if(array == NULL || len <= 0)
        return;
    int position = 3;
    rotate(array,len,2);
    print(array,len);
}

int binarysearch(int *array, int len, int target){
    if(array == NULL || len <= 0)
        return -1;
    int start = 0, end = len - 1;
    while(start <= end){
        int middle = (start + end) >> 1;
        if(array[middle] == target)
            return middle;
        else{
            if(array[middle] < array[end]){
                if(array[middle] < target && target <= array[end]){
                    start = middle + 1;
                }else{
                    end = middle - 1;
                }
            }else if(array[middle] == array[end]){
                if(array[middle] == array[start]){
                    // tran
                    while(start < end){
                        if(array[start] == target){
                            return start; 
                        }else{
                            ++start;
                        }
                    }
                    return -1;
                }else{
                    //
                    end = middle - 1;
                }
            }else{ //array[middle] > array[end]
                if(array[start] < target &&  target < array[middle]){
                    end = middle - 1;
                }else{
                    start = middle + 1;
                }
            }
        }
    }
    return -1;
}

void test_binarysearch(int *array, int len){
    int target;
    while(cin >> target){
        cout << "index:" << binarysearch(array,len,target) << endl;
    }
}

int main(){
    //int array[] = {1,2,3,4,5};
    //int array[] = {1,2,1,1,1,1,1};
    int array[] = {6,0,1,3,4,4};
    int len = sizeof(array) / sizeof(int);
    print(array,len);
    //test_rotate(array,len);
    test_binarysearch(array,len);
    return 0;
}
