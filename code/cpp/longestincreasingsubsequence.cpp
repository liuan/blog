#include<iostream>
//#include<iterator>
#include<vector>

using namespace std;

int lis(const vector<int>& vec){
    int len = vec.size();
    vector<int> dp;
    dp.push_back(vec[0]); 
    int i = 0, last = 0;
    for(i = 1; i < len; ++i){
        if(vec[i] > dp.back()){
            dp.push_back(vec[i]);
        }else if(vec[i] < dp.back()){
            // then we should find first one bigger than this number; 
            int start = 0, end = dp.size();
            while(start < end){
                int middle = (start + end) >> 1;
                if(dp[middle] >= vec[i]){
                    end = middle;
                }else{
                    start = middle + 1; 
                }
            }
            dp[start] = vec[i];
        }
    }
    
    i = len - 1;

    return dp.size();
}

int lisv2(const vector<int>& vec){
    int len = vec.size();
    vector<int> dp(len,0);
    dp[0] = 1;
    int i = 0, j = 0, lis = 0;
    for(i = 0; i < len; ++i){
        for(j = i - 1; j >= 0; --j){
            if(vec[i] > vec[j] && dp[i] < dp[j] + 1){
                dp[i] = dp[j] + 1;
                if(dp[i] > lis){
                    lis = dp[i];
                }
            }
        }
    }

    int temp = lis;
    for(i = len - 1; i >= 0; --i){
        if(dp[i] == temp){
            cout << vec[i] << " ";
            --temp;
        }
    }
    cout << endl;

    return lis;
} 

void test_lis(){
    int array[] = {1,-1,2,-3,4,-5,6,-7};
    vector<int> vec(array,array+sizeof(array) / sizeof(int));
    cout << "lis:" << lis(vec) << endl;
}

void test_lisv1(){
    int array[] = {1,-1,2,-3,4,-5,6,-7};
    vector<int> vec(array,array+sizeof(array) / sizeof(int));
    //cout << "lisv1:" << lisv1(vec) << endl;
}

void test_lisv2(){
    int array[] = {1,-1,2,-3,4,-5,6,-7};
    vector<int> vec(array,array+sizeof(array) / sizeof(int));
    cout << "lisv2:" << lisv2(vec) << endl;
}

int main(){
    test_lis();
    test_lisv1();
    test_lisv2();
    return 0;
}
