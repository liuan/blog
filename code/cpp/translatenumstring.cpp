#include<iostream>
#include<sstream>
#include<vector>

using namespace std;

void translate(const string& str, int index, vector<char>& vec){
    if(index == str.size()){
        vector<char>::iterator iter = vec.begin();
        while(iter != vec.end()){
            cout << *iter++;
        }
        cout << endl;
        return;
    }

    int i = index;
    vec.push_back(str[i] - '1' + 'a');
    translate(str,i+1,vec);
    vec.pop_back();
    if(i+1 < str.size()){
        int data = str[i] - '0';
        data *= 10;
        data += str[i+1] - '0';
        if(data > 0 && data <= 26){
            vec.push_back(data - 1 + 'a');
            translate(str,i+2,vec);
            vec.pop_back();
        }
    }
}

void translatenumstring(int num){
    if(num <= 0){
        cout << "should be a positive number" << endl;
        return;
    } 

    stringstream ss;
    ss << num;
    
    cout << "num:" << num << endl;
    string str(ss.str());
    vector<char> vec;
    translate(str,0,vec);
}

int main(){
    translatenumstring(12259);
}
