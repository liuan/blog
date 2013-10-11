#include<iostream>
#include<iterator>
#include<vector>
#include<algorithm>

using namespace std;

void print(int *array, int len){
    if(!array || len <= 0)
        return;
    copy(array,array+len,ostream_iterator<int,char>(cout," "));
    cout << endl;
}


// 0-1
void problem01(int *array, int len, int target){
    if(!array || len <= 0) 
        return;
    int i = 0, j = 0;
    vector<vector<int> > dp(len+1,vector<int>(target+1,0));

    for(i = 0; i < len; ++i)
        dp[0][i] = (array[0] <= target) ? array[0] : 0;

    for(i = 1; i < len; ++i){
        for(j = 0; j <= target; ++j){
            dp[i][j] = dp[i-1][j];                   
            if(j >= array[i])
                dp[i][j] = max(dp[i-1][j-array[i]]+array[i], dp[i-1][j]);
        }
    } 

    cout << "max:" << dp[len-1][target] << endl;
}

// 0-1
void problem02(int *array, int len, int target){
    if(!array || len <= 0)
        return;
    int i = 0, j = 0;
    vector<int> dp(target+1, 0);
    //dp[0] = (array[0] <= target) ? array[0] : 0;

    for(i = 0; i < len; ++i){
        for(j = target; j >= 0; --j){
            if(j > array[i]){
                dp[j] = max(dp[j],dp[j-array[i]]+array[i]);
            }
        }
    }
     
    cout << "max:" << dp[target] << endl;
}

void test_problem01(){
    int array[] = {1,5,7,8,9};
    int len = sizeof(array) / sizeof(int);
    print(array,len);
    int target = 11;
    problem01(array,len,target);
}

void test_problem02(){
    int array[] = {1,5,7,8,9};
    int len = sizeof(array) / sizeof(int);
    print(array,len);
    int target = 11;
    problem02(array,len,target);
}

int main(){
    test_problem01();
    test_problem02();
    return 0;
}
