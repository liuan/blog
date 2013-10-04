#include<iostream>

using namespace std;

enum{
    ERROR = - 1,
    SUCCESS,
    READING_NUM,
    READING_DOT
};

int iptounint(const string& str, unsigned int& result){

    if(str.empty() || str.size() < 7){
        return ERROR;
    }


    unsigned int digit = 0;
    int dotNum = 0;
    int digitNum = 0;
    char input;
    int i = 0;
    int state = READING_NUM;

    result = 0;

    for(i = 0; ; ++i){
        input = str[i];
        if(state == READING_NUM){
            if(input >= '0' && input <= '9'){
                digit =  digit * 10 + input - '0';
                state = READING_DOT;
            }else{
                result = 0;
                return ERROR;
            }
        }else{
            if(input >= '0' && input <= '9'){
                digit =  digit * 10 + input - '0';
                if(digit < 0 || digit > 255){
                    result = 0;
                    return ERROR;
                }
            }else if(input == '.'){
                if(dotNum == 3){
                    result = 0;
                    return ERROR;
                }
                ++dotNum;
                result = (result << 8) + digit;
                digit = 0;
                state = READING_NUM;
            }else{
                if(i == str.size()){
                    if(dotNum != 3){
                        result = 0;
                        return ERROR;
                    }
                    result = (result << 8) + digit;
                    return SUCCESS;
                }else{
                    result = 0;
                    return ERROR;
                } 
            }
        }
    }
}

int main(){
    unsigned int result = 0;
    int re = iptounint("10.125.235.245",result);
    cout << "mask:" << (result & 0xFF) << endl;
    cout << "mask:" << ((result & (0xFF << 8)) >> 8) << endl;
    cout << "mask:" << ((result & (0xFF << 16)) >> 16) << endl;
    cout << "mask:" << ((result & (0xFF << 24)) >> 24) << endl;
    cout << "result:" << result << endl;
    cout << "re:" << re << endl;
}
