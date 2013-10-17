/*****************************************************************
 * 1. 给定一个数组，我们需要从中找出和最接近某个指定数的元素集合。
 *    例如：1，2，3，4，5，6， 从其中查找和最接近15的元素集合，那么
 *    可以得到这样的集合{4,5,6}或者{2,3,4,6}等等，将所有的集合输出,
 *    同样对于数字22，那么只能是所有元素的集合了。
 * 2. 此问题可以再此扩展，对于给定元素个数是2n的集合，将其划分为长度
 *    相等的两个子数组，其和最接近。
 *    例如：1，5，7，8，9，6，3，11，20，17，那么可以得到这样的子数组
 *    1，3，11，8，20 与 5，7，9，6，17
******************************************************************/

#include<iostream>
#include<iomanip>
#include<iterator>
#include<vector>
#include<algorithm>
#include<numeric>

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

//dp[i][j] //表示是否可以找到i个数构成和为j
void problem10(int *array, int len, int target){
    if(!array || len <= 0)
        return;
    vector<vector<int> > dp(len+1,vector<int>(target+1,0));    
    dp[0][0] = 1;
    int i = 0, j = 0, k = 0;
    for(k = 1; k <= len; ++k){
        for(i = k; i >= 1; --i){ // 前i个数
            for(j = 1; j <= target; ++j){
                if(j >= array[k-1] && dp[i-1][j - array[k-1]]){
                    dp[i][j] = 1; 
                }
            }
        }
    }

    for(i = 0; i <= len; ++i){
        for(j = 0; j <= target; ++j){
            cout << setw(4) << dp[i][j];
        }
        cout << endl;
    }
    
    bool find = false;
    for(i = 1; i <= len; ++i){
        if(dp[i][target]){
            find = true;
            break;
        }
    }
    cout << (find ? "true" : "false") << endl;
}

// split the array to two part;
void problem11(int *array, int len){
    if(!array || len <= 0)
        return;
    int sum = 0;
    accumulate(array,array+len,sum);
    vector<vector<int> > dp(len+1,vector<int>((sum>>1)+1,0)); 

    vector<vector<vector<int> > > path(len+1,vector<vector<int> >((len>>1)+1, vector<int>((sum>>1)+1,0)));

    dp[0][0] = 1;
    int i, j, k;
    for(k = 1; k <= len; ++k){
        for(i = min(k,len>>1); i >= 1; ++i){
            for(j = 1; j <= (sum>>1); ++j){
                if(j > array[k-1] && dp[i-1][j - array[k]])
                    dp[i][j] = 1;
            }
        }
    }



}


void test_problem01(int *array, int len){
    print(array,len);
    int target = 11;
    problem01(array,len,target);
}

void test_problem02(int *array, int len){
    print(array,len);
    int target = 11;
    problem02(array,len,target);
}

void test_problem10(int *array, int len){
    print(array,len);
    int target = 6;
    problem10(array,len,target);
}

int main(){
    int array[] = {1,5,7,8,9};
    int len = sizeof(array) / sizeof(int);
    test_problem01(array, len);
    test_problem02(array,len);
    test_problem10(array,len);
    return 0;
}
