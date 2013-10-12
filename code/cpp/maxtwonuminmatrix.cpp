#include<iostream>

using namespace std;

int maxtwonuminmatrix(const int matrix[][4], int dimen){
    if(matrix == NULL || dimen <= 1)
        return -1;
    int i = 0, j = 0, n = 0, m = 0; 
    int maxsum = 0, cursum = 0;
    for(i = 0; i < dimen-1; ++i){ // column
        for(j = 0; j < dimen; ++j){ // 
            for( m = 0; m < dimen; ++m){
                for(n = i + 1; n < dimen; ++n){ // column
                    if(m != j){
                        cursum = matrix[j][i] + matrix[m][n];
                        maxsum = (cursum > maxsum) ? cursum : maxsum;
                    }
                }
            }
        }
    }
    return maxsum;
}

int main(){
    int matrix[4][4] = {3,4,5,6,
                        7,20,9,10,
                        11,12,13,14,
                        40,16,17,18};
    cout << maxtwonuminmatrix(matrix,4) << endl;
}
