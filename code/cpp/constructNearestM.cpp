#include<iostream>
#include<climits>
#include<cstring>
#include<stack>
#include<iomanip>

using namespace std;

void print(int **state, int row, int column){
    int i = 0,j = 0;
    for(i = 0; i < row; i++){
        for(j = 0; j < column; j++){
            if(state[i][j] == INT_MIN)
                cout << setw(3) << "*";
            else
                cout << setw(3) << state[i][j];
        }
        cout << endl;
    }
    cout << endl;
}

int abs(int first, int second){
    if(first > second)
        return first - second;
    else
        return second - first;
}

int abs(int data){
    if(data > 0)
        return data;
    else
        return -data;
}

void dfs(const int *array, int index, int sum,int *diff,int target){
    if(array == NULL || diff == NULL) 
        return;
    if(index < 0){
        if(abs(sum,target) < *diff){
            *diff = abs(sum,target); 
        }
        return;
    }
    dfs(array,index-1,sum+array[index],diff,target);
    dfs(array,index-1,sum-array[index],diff,target);
} 

void dfsV2(const int *array, int index, int target, int *diff){
    if(array == NULL || diff == NULL)
        return;
    if(index < 0){
        if(abs(target) < *diff){
            *diff = abs(target);
        }
        return;
    }
    dfsV2(array,index-1,target-array[index],diff);
    dfsV2(array,index-1,target+array[index],diff);
}

int dfsV3(const int* array, int index, int target){
    if(index < 0)
        return target;
    int plus = dfsV3(array, index-1,target+array[index]);
    int minus = dfsV3(array, index-1,target-array[index]);
    return abs(plus) < abs(minus) ? abs(plus) : abs(minus);
    //return abs(plus,0) < abs(minus,0) ? plus : minus;
    //return abs(plus) < abs(minus) ? plus : minus;
}

void testDfsV3(int *array, int len,int target){
    if(array == NULL || len <= 0)
        return;
    cout <<"testDfsV3:"<< dfsV3(array,len-1,target) << endl;
}

int dfsV4(const int* array, int index, int target,int offset, int **state){
    if(index == 0){
        return abs(target-1,0) < abs(target+1,0) ? target - 1 : target + 1;
    }
    if(state[index][target+offset] > INT_MIN)
        return state[index][target+offset];
    
    int plus = dfsV4(array,index-1,target+array[index],offset,state);
    state[index-1][target + array[index] + offset] = plus;

    int minus = dfsV4(array,index-1,target-array[index],offset,state);
    state[index-1][target - array[index] + offset] = minus;

    state[index][target+offset] = abs(plus,0) < abs(minus,0) ? abs(plus) : abs(minus); 
    return state[index][target+offset];
}


void backtracing(const int* array,int index,int **state,int offset,int target){
    if(array == NULL || state == NULL)
        return;
    std::stack<bool> symbol;
    int diff = state[index][target+offset];
    while(index > 0){
        if(state[index][target + offset] == state[index-1][target - array[index] + offset]){
            symbol.push(false);
            target -= array[index];
        }else{
            symbol.push(true);
            target += array[index];
        }
        index--;
    }
    if(target + 1 == diff)
        symbol.push(true);
    else
        symbol.push(false);

    int i = 0;
    while(!symbol.empty()){
        if(!symbol.top()){
            if(i == 0){
                cout << array[i++];
            }else{
                cout <<"+"<< array[i++];
            } 
        }else{
            cout << "-" << array[i++];
        }
        symbol.pop();
    }
    cout << endl;
}

void backtracing(const int *array, int index, int **state, int offset, int target,stack<bool> symbol){
    if(index == 0){
        if(abs(target + 1,0) < abs(target - 1,0))
            symbol.push(true);
        else
            symbol.push(false);
        int i = 0;
        while(!symbol.empty()){
            if(!symbol.top()){
                if(i == 0){
                    cout << array[i++];
                }else{
                    cout <<"+"<< array[i++];
                } 
            }else{
                cout << "-" << array[i++];
            }
            symbol.pop();
        }
        cout << endl;
        return;
    }
    int left = state[index-1][target-array[index] + offset];
    int middle = state[index][target+offset];
    if(middle == left){
        symbol.push(false);
        backtracing(array,index-1,state,offset,target-array[index],symbol);
    }else{
        symbol.push(true);
        backtracing(array,index-1,state,offset,target+array[index],symbol);
    }

    int right = state[index-1][target+array[index] + offset];
    if(middle == left && middle == right){
        symbol.pop();
        symbol.push(true);
        backtracing(array,index-1,state,offset,target+array[index],symbol);
    }
}

void testDfsV4(int *array, int len,int target){
    if(array == NULL || len <= 0)
        return;
    int **state = new int*[len];
    int i,j,sum = 0;
    for(i = 0; i < len; i++)
        sum += array[i];
    //sum +=  target + 1;
    int offset = sum - target;
    sum += offset + target + 1;
    cout << "len:" << len << " sum:" << sum << " target:"<<target<< endl;

    for(i = 0; i < len; i++){
        state[i] = new int[sum];
        //memset(state[i],0,sizeof(int)*sum);
        for(j = 0; j < sum; j++)
            state[i][j] = INT_MIN;
    }
    
    dfsV4(array,len-1,target,offset,state);
    print(state,len,sum+1);
    cout<<"testDfsv4:" << state[len-1][target + offset] << endl;
    backtracing(array,len-1,state,offset,target);
    cout << "******************************" << endl;
    std::stack<bool> symbol;
    backtracing(array,len-1,state,offset,target,symbol);
    
    
    for(i = 0; i < len; i++){
        delete [] state[i];
    }
    delete [] state;

}

int main(){
    int array[] = {1,2,3,4,5};
    int len = sizeof(array) / sizeof(int);
    int target = 6;
    testDfsV3(array,len,target);
    testDfsV4(array,len,target);
    return 0;
}
