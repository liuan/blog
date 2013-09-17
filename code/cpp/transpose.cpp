#include<iostream>
#include<iterator>

using namespace std;

int pre(int *matrix, int index, int row, int col){
    return (index%row)*col + (index/row);
}

int next(int *matrix, int index, int row, int col){
    return (index%col)*row + (index/col);
}

void move(int *matrix, int index, int row, int col){
    int curvalue = matrix[index];
    int curindex = index;
    int preindex = pre(matrix,curindex,row,col);   
    while(index != preindex){
        matrix[curindex] = matrix[preindex];
        curindex = preindex;
        preindex = pre(matrix,curindex,row,col);
    }
    matrix[curindex] = curvalue;
}

void transpose(int *matrix, int row, int col){
    if(matrix == NULL || row <= 0 || col <= 0)
        return;
    int i = 0,temp;
    for(i; i < row*col; i++){
        temp = next(matrix,i,row,col);
        while(i < temp){
            temp = next(matrix,temp,row,col);
        }
        if(i == temp){
            move(matrix,i,row,col);
        }
    }
}

int main(){
    int matrix[] = {0,1,2,3,4,5,6,7,8,9};
    int len = sizeof(matrix) / sizeof(int);
    copy(matrix,matrix+len,ostream_iterator<int,char>(cout, " "));
    cout << endl;
    transpose(matrix,2,5); 
    copy(matrix,matrix+len,ostream_iterator<int,char>(cout, " "));
    cout << endl;
    return 0;
}
