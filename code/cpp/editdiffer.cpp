#include<iostream>
#include<vector>
#include<iterator>
#include<climits>

using namespace std;

void printv(vector<string>& vec){
    copy(vec.begin(),vec.end(),ostream_iterator<string,char>(cout," "));
    cout << endl;
}

int differ(string string1, string string2){
    if(string1.size() == 0 && string2.size() == 0)
        return 0;

    vector<string> fvec,svec;
    int flen = string1.size();
    int slen = string2.size();

    int start = 0;
    int i = 0, j = 0;

    for(i = 0; i < flen; ++i){
        if(string1[i] == ' ' && i == 0){
            start += 1;
        }else if( string1[i] == ' '){
            fvec.push_back(string1.substr(start,i-start));
            start = i + 1;
        }else if(i == flen -1){
            fvec.push_back(string1.substr(start,i+1-start));
        }
    }

    start = 0;
    for(i = 0; i < slen; ++i){
        if(string2[i] == ' ' && i == 0){
            start += 1;
        }else if( string2[i] == ' '){
            svec.push_back(string2.substr(start,i-start));
            start = i + 1;
        }else if(i == slen -1){
            svec.push_back(string2.substr(start,i+1-start));
        }
    }


    flen = fvec.size();
    slen = svec.size();

	vector<vector<int> > state(flen + 1, vector<int>(slen + 1, INT_MAX));
     
	for(i = 0; i <= flen; ++i)
	{
		state[i][0] = i;
	}

	for(j = 0; j <= slen; ++j)
	{
		state[0][j] = j;
	}

	for(i = 1; i <= flen; ++i)
	{
		for(j = 1; j <= slen; ++j)
		{
			if(fvec[i - 1].compare(svec[j - 1]) == 0)
			{
				state[i][j] = state[i - 1][j - 1];
			}
			else
			{
				state[i][j] = state[i - 1][j - 1] + 1;
				state[i][j] = min(state[i][j], state[i - 1][j] + 1);
				state[i][j] = min(state[i][j], state[i][j - 1] + 1);
			}
		}
	}

	return state[flen][slen];
}

int main(){
    string one = "dao xiang dong";
    string two = "dong dao xiang";
    cout << "differ:" << differ(one,two) << endl;
    return 0;

}
