#include<iostream>
#include<vector>

using namespace std;

int lcsv1(const string& str1, const string& str2){
    int flen = str1.size();
    int slen = str2.size();

    vector<vector<int> > dp(flen+1, vector<int>(slen+1,0));
    int i = 0, j = 0, m = 0, n = 0;
    
    for(i = 1; i <= flen; ++i){
        for(j = 1; j <= slen; ++j){
            m = i;
            n = j;
            while(m >= 1 && n >= 1 && str1[--m] == str2[--n]){}
            dp[i][j] = max(i-m, dp[m][n]);
            dp[i][j] = max(dp[i-1][j],dp[i][j]);
            dp[i][j] = max(dp[i][j-1],dp[i][j]);
        }
    }         

    //back tracing
    i = flen;
    j = slen;
    while(i >= 1 && j >= 1){
        if(str1[i-1] == str2[j-1]){
            if(dp[i][j] == dp[flen][slen] && dp[i-1][j-1] + 1 == dp[flen][slen]){
                cout << string(str1,i-dp[flen][slen],dp[flen][slen]) << endl;
                break;
            }else{
                --i;
                --j;
            }
        }else if(dp[i-1][j] > dp[i][j-1]){
            --i;
        }else{
            --j;
        }
    }

    return dp[flen][slen];
}

int lcsv3(const string& str1, const string& str2){
    int flen = str1.size();
    int slen = str2.size();

    vector<vector<int> > dp(flen+1, vector<int>(slen+1,0));
    int i = 0, j = 0;

    int max = 0, index = 0;
    
    for(i = 1; i <= flen; ++i){
        for(j = 1; j <= slen; ++j){
            if(str1[i-1] == str2[j-1]){
                dp[i][j] = dp[i-1][j-1] + 1;
                if(dp[i][j] > max){
                    max = dp[i][j];
                    index = i - dp[i][j];
                }
            }
        }
    }

    cout << string(str1,index,max) << endl;
    return max;
}

int lcsv4(const string& str1, const string& str2){
    int flen = str1.size();
    int slen = str2.size();

    vector<int> dp(slen+1,0);
    int index = 0, max = 0;
    int i = 0, j = 0;
    
    for(i = 1; i <= flen; ++i){
        for(j = slen; j > 0; --j){
            if(str1[i-1] == str2[j-1]){
                dp[j] = dp[j-1] + 1;
                if(dp[j] > max){
                    max = dp[j];
                    index = i - dp[j];
                }
            }
        }
    }

    cout << string(str1,index,max) << endl;
    return max;
}

int lcsv2(const string& str1, const string& str2){
    int flen = str1.size();
    int slen = str2.size();

    int index = 0, max = 0, cur = 0;
    int i = 0, j = 0, k = 0;

    for(i = 0; i < flen; ++i){
        j = 0;
        while(j < slen){
            k = i;
            cur = 0;
            while( k < flen && j < slen && str1[k] == str2[j]){
                ++k;
                ++j;
                ++cur;
            }
            if(cur > max){
                max = cur;
                index = k - max;
            }
            //max = cur > max ? cur : max;
            ++j;
        }
    }
    cout << string(str1,index,max) << endl;
    
    return max;
}

void test_lcsv2(){
    string str1 = "xyzab";
    string str2 = "zabxycxyzab";
    cout << str1 << " " << str2 << endl;
    cout << "lcsv2: " << lcsv2(str1,str2) << endl;
}

void test_lcsv1(){
    string str1 = "xyzab";
    string str2 = "zabxycxyzab";
    cout << str1 << " " << str2 << endl;
    cout << "lcsv1: " << lcsv1(str1,str2) << endl;
}

void test_lcsv3(){
    string str1 = "xyzab";
    string str2 = "zabxycxyzab";
    cout << str1 << " " << str2 << endl;
    cout << "lcsv3: " << lcsv3(str1,str2) << endl;
}

void test_lcsv4(){
    string str1 = "xyzab";
    string str2 = "zabxycxyzab";
    cout << str1 << " " << str2 << endl;
    cout << "lcsv4: " << lcsv4(str1,str2) << endl;
}

int main(){

    test_lcsv1();    
    test_lcsv2();    
    test_lcsv3();    
    test_lcsv4();    

    return 0;
}
