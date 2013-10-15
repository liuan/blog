#include<iostream>
#include<sstream>
#include<map>

using namespace std;

class Cookie{
    private:
        map<string,string> mcookie;
    public:
        Cookie();
        Cookie(const string& str);
        string tostring() const;
};

Cookie::Cookie(const string& str){
    int begin = 0;
    int end, i;
    do{
        end = str.find(';',begin);
        if(end > 0){
            i = str.find('=',begin);
            if(i > 0)
                mcookie.insert(make_pair(string(str,begin, i - begin ),
                                    string(str,i+1,end - i - 1)));
        }
        if(i < 0 || end < 0){
            //cout << "check the input" << endl;
            break;
        }
        begin = end + 1;
    } while( end >= 0 && end < str.size());
    
}

string Cookie::tostring()const{
    stringstream ss;
    map<string,string>::const_iterator iter = mcookie.begin();
    while(iter != mcookie.end()){
        ss << "[" << iter->first << "] = " << iter->second << endl; 
        ++iter;
    }
    return ss.str();
}

void test_cookie(){
    string str("username=liuan;password=cool;");
    Cookie cookie(str);
    cout << cookie.tostring();
}

int main(){
    test_cookie();
    return 0;
}
