#include<iostream>
#include<vector>
#include<stack>

using namespace std;

template<typename T>
void print(stack<int> s, vector<T>& vec){
    while(!s.empty()){
        cout << vec[s.top()] << " ";
        s.pop();
    }
    cout << endl;
}

template<typename T>
void permutation(vector<T>& vec, int index){
    if(index == vec.size()){
        typename vector<T>::iterator iter = vec.begin(); 
        while(iter != vec.end()){
            cout << *iter++;
        }
        cout << endl;
        return;
    }

    int i = index;
    for(i; i < vec.size(); ++i){
        swap(vec[index],vec[i]);
        permutation(vec,index+1);
        swap(vec[index],vec[i]);
    }
}

template<typename T>
void combination(vector<T>& vec, int m){
    if(vec.empty() || m <= 0)
        return;
    int n = vec.size(); // Cn m
    stack<int> s; // record the index
    int temp = n - 1;
    s.push(temp);
    while(true){
        if(temp == 0){
            if(s.empty()){
                break;
            }
            temp = s.top();
            s.pop();
        }else{
            s.push(--temp);
            if(s.size() == m){
                print(s,vec);
                temp = s.top();
                s.pop();
            }
        }
    }
}

template<typename T>
void combination(vector<T>& vec, int index, vector<T>& choosed, int m){
    if(choosed.size() == m){
        typename vector<T>::iterator iter = choosed.begin();
        while(iter != choosed.end()){
            cout << *iter++ << " ";
        }
        cout << endl;
        return;
    }
    int i = index;
    for(i; i < vec.size(); ++i){
        choosed.push_back(vec[i]);
        combination(vec,i+1,choosed,m);
        choosed.pop_back();
    }
}



int main(){
    char ch[] = "abcd";
    vector<char> vec(ch,ch+sizeof(ch)-1); 
    permutation(vec,0);
    cout << "++++++++++++++++++++++++++++++" << endl;
    combination(vec,3);
    cout << "++++++++++++++++++++++++++++++" << endl;
    vector<char> choosed; 
    combination(vec,0,choosed,3);
    return 0;

}
