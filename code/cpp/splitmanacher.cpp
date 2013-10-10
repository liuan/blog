#include<iostream>
#include<vector>

using namespace std;

// dp[i][j] = i==j;
// dp[i][j] = str[i] == str[j] && i+1 == j;
// dp[i][j] = dp[i+1][j-1] && str[i] == str[j];
//

void manacher(const string& str){
    vector<vector<bool> > dp(str.size()+1, vector<bool>(str.size()+1, false));
    int i = 0, j = 0;
    for(i = 0; i < str.size(); ++i)
        dp[i][i] = true;

    for(i = 2; i <= str.size(); ++i){
        for(j = 0; j < str.size() - i + 1; ++j){
            int k =  j + i - 1;
            if(i == 2)
                dp[j][k] = (str[j] == str[k]);
            else
                dp[j][k] = (str[j] == str[k]) && dp[j+1][k-1];
        }
    }

    // output the dp
    for(i = 0; i < str.size(); ++i){
        for(j = 0; j < str.size(); ++j){
            cout << dp[i][j] << " ";
        }
        cout << endl;
    }

    // get the max manacher substring
    int max = 1, index = 0;
    for(i = 0; i < str.size(); ++i){
        j = i + index + max;
        for(j; j < str.size(); ++j){
            if(dp[i][j] == 1 && (j-i + 1) > max){
                max = j - i + 1;
                index = i;
            }
        }
    }

    cout << string(str,index,max) << endl;
}

void test_manacher(){
    const string str("ababbaabbababa");
    //const string str("ababba");
    cout << str << endl;
    manacher(str);
}

int main(){
    test_manacher();
    return 0;
}
