#include<iostream>
#include<iterator>
#include<algorithm>
#include<vector>

using namespace std;

bool next_permutationv1(vector<int>& vec){
    int len = vec.size();
    int i = 0, j = 0;
    for(i = len - 1; i > 0; --i){
        if(vec[i] > vec[i-1])
            break;
    }

    if(i == 0) // can't find next permutation
        return false;

    int pivot = i-1;
    for(i = len - 1; i >= 0; --i){
        if(vec[i] > vec[pivot])
            break;
    }
    swap(vec[i],vec[pivot]);
    //sort(vec.begin()+pivot+1,vec.end()); // we also can use sort instead of the next 5 lines
    i = pivot + 1;
    j = len - 1;
    while(i < j){
        swap(vec[i++],vec[j--]);
    }
    copy(vec.begin(),vec.end(),ostream_iterator<int,char>(cout, " "));
    cout << endl;

    return true; 
}

void test_next_permutationv1(){
    int array[] = {1,2,3,5,4,2,1}; 
    copy(array,array+sizeof(array)/sizeof(int),ostream_iterator<int,char>(cout, " "));
    cout << endl;
    cout << "**********************************" << endl;
    vector<int> vec(array,array + sizeof(array)/sizeof(int));
    while(next_permutationv1(vec)){}
    //next_permutation(vec);
}

void test_next_permutation(){
    int array[] = {1,2,3,5,4,2,1}; 
    copy(array,array+sizeof(array)/sizeof(int),ostream_iterator<int,char>(cout, " "));
    cout << endl;
    cout << "**********************************" << endl;
    vector<int> vec(array,array + sizeof(array)/sizeof(int));
    while(next_permutation(vec.begin(),vec.end())){
        copy(vec.begin(),vec.end(),ostream_iterator<int,char>(cout, " "));
        cout << endl;
    }
}

int main(){
    test_next_permutation();
    return 0;
}
