#include<iostream>
#include<vector>

using namespace std;

int editdistance(const string& str1, const string& str2){
    int flen = str1.size();
    int slen = str2.size();

    vector<vector<int> > dp(flen+1, vector<int>(slen+1,0xFFFFFFFF));

    int i = 0, j = 0;
    
    for(i = 0; i <= flen; ++i)  
        dp[i][0] = i;

    for(i = 0; i <= slen; ++i)  
        dp[0][i] = i;

    for(i = 1; i <= flen; ++i){
        for(j = 1; j <= slen; ++j){
            if(str1[i-1] == str2[j-1]){
                dp[i][j] = dp[i-1][j-1];
            }else{
                dp[i][j] = dp[i-1][j-1] + 1;
                dp[i][j] = min(dp[i-1][j]+1,dp[i][j]);
                dp[i][j] = min(dp[i][j-1]+1,dp[i][j]);
            }
        }
    }

    return dp[flen][slen];
}

void test_editdistance(){
    string str1 = "choudan";
    string str2 = "danchou";
    cout << str1 << endl; 
    cout << str2 << endl; 
    cout << "distance: " << editdistance(str1,str2) << endl;
}

int main(){
    test_editdistance();
    return 0;
}
