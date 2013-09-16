#include<iostream>
#include<iomanip>

using namespace std;

void print(int **matrix, int row, int col){
    if(matrix == NULL)
        return;
    int i = 0, j = 0;
    for(i; i < row; i++){
        for(j = 0; j < col; j++){
            cout << setw(4) << matrix[i][j] << " ";
        }
        cout << endl;
    }
}

void print(int **matrix, int dimension){
    if(matrix == NULL || dimension <= 0){
        return;
    }
    int i = 0, j = 0;     
    for(i;i < dimension; i++){
        for(j = 0; j < dimension; j++){
            cout << setw(4) << matrix[i][j] << " ";
        }
        cout << endl;
    }
}

void printMatrix(int **matrix, int dimension){
    if(matrix == NULL || dimension <= 0)
        return;
    int i = 0, j = 0, steps = dimension;
    int circle = steps >> 1;
    int num = 1, k = 0;
    while(k < circle){
        i = j = k;
        while(j < steps){ // fisrt print steps-j elements
            matrix[i][j++] = num++;
        }
        --j;

        while(i < steps - 1){ // 0,1,2,3
            matrix[++i][j] = num++;
        }

        while(j > k){
            matrix[i][--j] = num++;
        }

        while(i > k+1){
            matrix[--i][j] = num++;
        }
        ++k;
        --steps;
    }
    if(dimension & 0x1 == 1)
        matrix[circle][circle] = num;
}

void printMatrix(int **matrix,int row, int col){
    if(matrix == NULL || row <= 0 || col <= 0)
        return;
    int i = 0, j = 0,k = 0;
    int left = col >> 1, down = row >> 1;
    int num = 1;
    while(k < left && k < down){
        i = j = k;
        while(j < col){
            matrix[i][j++] = num++;
        }
        --j;
        ++i;
        while(i < row){
            matrix[i++][j] = num++;
        } 
        --i;
        --j;
        while(j >= k){
            matrix[i][j--] = num++;
        }
        ++j;
        --i;
        while(i >= k+1){
            matrix[i--][j] = num++;
        }
        --row;
        --col;
        ++k;
    }
    if(k == left && k == down){
        if(k != row && k != col)
            matrix[k][k] = num;
    }else if(k == left && k < down){
        if(k != col){
            i = j = k;
            while(i > row){
                matrix[i++][j] = num++;
            }
        }
    }else if(k < left && k == down){
        if(k != row){
            i = j = k;
            while(j < col){
                matrix[i][j++] = num++;
            }
        }
    }
}


void test_printv1(){
    int row;
    int col;
    cout << "row:";
    cin >> row;
    cout << "col:";
    cin >> col;
    int** matrix = new int*[row];
    int i = 0;
    for(i = 0; i < row; i++){
        matrix[i] = new int[col];
    }

    printMatrix(matrix,row);
    print(matrix,row);

    for(i = 0; i < row; i++){
        delete [] matrix[i];
    }
    delete [] matrix;

}

void test_printv2(){
    int row;
    int col;
    cout << "row:";
    cin >> row;
    cout << "col:";
    cin >> col;
    int** matrix = new int*[row];
    int i = 0;
    for(i = 0; i < row; i++){
        matrix[i] = new int[col];
    }

    printMatrix(matrix,row,col);
    print(matrix,row,col);

    for(i = 0; i < row; i++){
        delete [] matrix[i];
    }
    delete [] matrix;
}

int main(){
    test_printv2();
    return 0;
}
