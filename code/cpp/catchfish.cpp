#include<iostream>
#include<vector>

using namespace std;

int catchfish(int buckets, int fishs, int limit=10){
    if(buckets <= 0 || fishs <= 0 || limit <= 0)
        return 0;
    vector<vector<int> > dp(buckets+1, vector<int>(fishs+1,0));
    int i,j,k;
    
    dp[0][0] = 1;

    for(i = 1; i <= buckets; ++i){
        for(j = 0; j <= fishs; ++j){
            for(k = 0; k <= limit && k <= j; ++k){
                dp[i][j] += dp[i-1][j-k];
            }
        }
    }

    return dp[buckets][fishs];
}

void test_catchfish(){
    int buckets;
    int fishs;
    while(cin>>buckets>>fishs){
        cout << "r:" << catchfish(buckets,fishs) << endl;
    }
}

int main(){
    test_catchfish();
    return 0;
}
