#include<iostream>
#include<vector>

using namespace std;

int knapsack(const vector<int>& value, const vector<int>& weight, int maxweight){
    if(value.size() <= 0 || value.size() != weight.size()) 
        return -1;
    int len = value.size();
    vector<vector<int> > dp(len+1, vector<int>(maxweight+1,0));
    int i = 0, j = 0;
    for(i = 1; i <= len; ++i){
        for(j = 1; j <= maxweight; ++j){
            if(weight[i-1] <= j){
                dp[i][j] = max(dp[i-1][j],value[i-1]+dp[i-1][j-weight[i-1]]);
            }else{
                dp[i][j] = dp[i-1][j];
            }
        }
    }

    return dp[len][maxweight];
}

void test_knapsack(){
    //int v[] = {4,5,6};
    //int w[] = {3,4,5};
    int v[] = {8,10,4,5,5};
    int w[] = {600,400,200,200,300};
    vector<int> value(v,v+sizeof(v)/sizeof(int));
    vector<int> weight(w,w+sizeof(w)/sizeof(int));
    int maxweight = 1000;
    cout << "knapsack: " << knapsack(value,weight,maxweight) << endl;
}

int main(){
    test_knapsack();
    return 0;
}
