#include<iostream>
#include<cstring>
#include<deque>
#include<stack>

using namespace std;

void handlestr(char * str, deque<char>& sign, deque<int>& numbers){
    if(str == NULL) 
        return;
    char *temp = str;
    while(*temp){
        if(*temp >= '0' && *temp <= '9'){
            int data = *temp++ - '0';
            while(*temp >= '0' && *temp <= '9'){
                data *= 10;
                data += *temp - '0';
                ++temp;
            }
            numbers.push_back(data);
            sign.push_back('n');
        }else{
            sign.push_back(*temp++);
        }
    }
}

template<typename T>
void print(deque<T> s){
    typename deque<T>::iterator iter = s.begin();
    while(iter != s.end()){
        cout << *iter++;
    }
    cout << endl;
}

void duplicate(const char* str){
    deque<char> sign;
    deque<int>  numbers;
    handlestr(const_cast<char*>(str),sign,numbers);
    print(sign);
    print(numbers);
}

int calculator(const char* str){
    if(str == NULL){
        cout << "Error!" << endl;
        return -1;
    }

    stack<char> sign;
    stack<int> numbers;
    int i = 0;
    while( i < strlen(str)){
        if(str[i] >= '0' && str[i] <= '9'){
            int data = str[i] - '0';
            int j = i + 1;
            while(j < strlen(str) && str[j] >= '0' && str[j] <= '9'){
                data *= 10; 
                data += str[j] - '0';
                ++j;
            }
            numbers.push(data);
            i = j;
        }else if(str[i] == '*' || str[i] == '/'){
            int data = 0;
            int j = i + 1;
            while(j < strlen(str) && str[j] >= '0' && str[j] <= '9'){
                data *= 10; 
                data += str[j] - '0';
                ++j;
            }
            if(j > i+1){
                int left = numbers.top();
                numbers.pop();
                if(str[i] == '*'){
                    numbers.push(left * data);
                }else{
                    numbers.push(left / data);
                }
                i = j;
            }else{
                sign.push(str[i++]);
                sign.push(str[i++]);
            }
        }else if(str[i] == '(' || str[i] == '+' || str[i] == '-'){
            sign.push(str[i++]);
        }else if(str[i] == ')'){
            while(sign.top() != '('){
                int right = numbers.top();
                numbers.pop();
                int left = numbers.top();
                numbers.pop();
                if(sign.top() == '+'){
                    numbers.push(left + right);
                }else if(sign.top() == '-'){
                    numbers.push(left - right);
                }else if(sign.top() == '*'){
                    numbers.push(left * right);
                }else if(sign.top() == '/'){
                    numbers.push(left / right);
                }
                sign.pop();
            }
            sign.pop(); // pop '('
            if(!sign.empty() && sign.top() == '/' || sign.top() == '*'){
                int right = numbers.top();
                numbers.pop();
                int left = numbers.top();
                numbers.pop();
                if(sign.top() == '*'){
                    numbers.push(left * right);
                }else if(sign.top() == '/'){
                    numbers.push(left / right);
                }
                sign.pop();
            }
            ++i;
        }

    }
    while(!sign.empty()){
        int right = numbers.top();
        numbers.pop();
        int left = numbers.top();
        numbers.pop();
        if(sign.top() == '+'){
            numbers.push(left + right);
        }else if(sign.top() == '-'){
            numbers.push(left - right);
        }else if(sign.top() == '*'){
            numbers.push(left * right);
        }else{ // '/'
            numbers.push(left / right);
        }
        sign.pop();
    }
    return numbers.top();
}

int main(){
    string str; 
    cin >> str;
    cout << str << " = " << calculator(str.c_str()) << endl;
    return 0;
}
