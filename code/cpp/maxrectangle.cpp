#include<iostream>
#include<iterator>
#include<stack>

using namespace std;

void print(stack<pair<int,int> > mstack){
    while(!mstack.empty()){
        cout << "<" << mstack.top().first << "," << mstack.top().second << ">" << endl;
        mstack.pop();
    }
}

int maxrectangle(int *array, int len){
    if(array == NULL || len <= 0)
        return 0;
    stack<pair<int,int> > mstack;
    int top = 0,max = 0;
    int i = 0;
    while(i < len){
        if(array[i] > top){
            mstack.push(make_pair(array[i],1));
        }else{
            int pre = 0;
            while(array[i] < top){
                int time = mstack.top().second;
                pre += time;
                max = top*pre > max ? top*pre : max;
                mstack.pop();
                if(!mstack.empty())
                    top = mstack.top().first;
                else
                    break;
            }
            if(!mstack.empty() && mstack.top().first == array[i])
                mstack.top().second += pre+1;
            else{
                mstack.push(make_pair(array[i],pre+1));
            }
        }
        print(mstack);
        cout << "***********" << endl;
        top = mstack.top().first;
        ++i;
    }

    return max;
}

void test(int *array, int len){
    cout << "max:" << maxrectangle(array,len) << endl;
}

int main(){
    //int array[] = {1,2,3,4,3,4,3,2,1};
    int array[] = {2,1,4,5,1,3,3};
    //int array[] = {1,2,3,4,1};
    int len = sizeof(array) / sizeof(int);
    copy(array,array+len,ostream_iterator<int,char>(cout," "));
    cout << endl;
    test(array,len);
    return 0;
}
