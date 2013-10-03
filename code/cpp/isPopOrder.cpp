#include<iostream>
#include<stack>
#include<iterator>

using namespace std;

bool isPopOrder(int *push, const int *pop, int len){
    if(push == NULL || pop == NULL || len <=0)
        return false;
    stack<int> mstack;  
    int i = 0, k = 0;
    while(i < len){
        if(!mstack.empty() && mstack.top() == pop[i]){
            mstack.pop();
        }else{
            if(k < len && push[k] != pop[i]){
                mstack.push(push[k++]);
            }
            if(k == len)
                return false;
        }
        ++i;
    }
}

int main(){
    int push[] = {1,2,3,4,5};
    //int pop[] = {4,5,3,2,1};
    //int pop[] = {3,4,2,5,1};
    int pop[] = {4,3,5,1,2};
    int len = sizeof(push) / sizeof(int);
    copy(push,push+len,ostream_iterator<int,char>(cout," "));
    cout << endl;
    copy(pop,pop+len,ostream_iterator<int,char>(cout," "));
    cout << endl;

    cout << "is pop order :" << (isPopOrder(push,pop,len) ? "true" : "false") << endl;

    return 0;
}
