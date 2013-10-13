#include<iostream>
#include<iterator>
#include<vector>
#include<algorithm>

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

struct Point{
    int value;
    int i;
    int j;
    Point(int va, int x, int y){
        value = va;
        i = x;
        j = y;
    }
    Point(int va=0x80000000){
        value = va;
    }
    friend ostream& operator<<(ostream& os, const Point& point);
};

ostream& operator<<(ostream& os, const Point& point){
    return os << point.value << " " << point.i << " " << point.j;
}

int compare(const Point& left, const Point& right){
    return left.value < right.value;
}

int maxtwonuminmatrixv2(const int matrix[][4], int dimen){
    if(matrix == NULL || dimen <= 1)
        return -1;
    vector<Point> record(2*dimen);

    int i = 0, j = 0;
    for(i = 0; i < dimen; ++i){
        for(j = 0; j < dimen; ++j){
            if(matrix[i][j] > record[i].value){ // max num in this row
                record[i].value = matrix[i][j];
                record[i].i = i;
                record[i].j = j;
            }
            if(matrix[i][j] > record[j+dimen].value){
                record[j+dimen].value = matrix[i][j]; 
                record[j+dimen].i = i;
                record[j+dimen].j = j;
            }
        }
    }
     
    sort(record.begin(),record.end(),compare);

    int max = 0x80000000;
    for(i = 2*dimen - 1; i >= 0; --i){
        for(j = i-1; j >= 0; --j){
            if(record[i].i != record[j].i && 
                    record[i].j != record[j].j){
                if(record[i].value + record[j].value > max){
                    max = record[i].value + record[j].value;
                }
            }
        }
    }

    return max; 
}

int main(){
    int matrix[4][4] = {3,4,5,6,
                        7,20,9,10,
                        11,12,13,14,
                        40,16,17,18};
    cout << maxtwonuminmatrix(matrix,4) << endl;
    cout << maxtwonuminmatrixv2(matrix,4) << endl;
    return 0;
}
