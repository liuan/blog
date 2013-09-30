#include<iostream>
#include<vector>

using namespace std;

int longestcommonsequence(const string& str1, const string& str2){
    int flen = str1.size();
    int slen = str2.size();

    vector<vector<int> > dp(flen+1, vector<int>(slen+1,0));

    int i = 0, j = 0;

    for(i = 1; i <= flen; ++i){
        for(j = 1; j <= slen; ++j){
            if(str1[i-1] == str2[j-1]){
                dp[i][j] = dp[i-1][j-1] + 1;
            }else{
                //dp[i][j] = dp[i-1][j-1];
                //dp[i][j] = max(dp[i-1][j],dp[i][j]);
                //dp[i][j] = max(dp[i][j-1],dp[i][j]);
                dp[i][j] = max(dp[i][j-1],dp[i-1][j]);
            }
        }
    }

    // back tracing
    i = flen, j = slen;
    while(i >= 1 && j >= 1){
        if(str1[i-1] == str2[j-1]){
            cout << str1[i-1];
            --i;
            --j;
        }else{
            if(dp[i-1][j] > dp[i][j-1])
                --i;
            else
                --j;
        }
    } 
    cout << endl;

    return dp[flen][slen];
}

void test_lcs(){
    string str1 = "abcdefg";
    string str2 = "baefkg";
    cout << str1 << "  " << str2 << endl;
    cout << "lcs:" << longestcommonsequence(str1,str2) << endl;
}

int main(){
    test_lcs();
    return 0;
}
