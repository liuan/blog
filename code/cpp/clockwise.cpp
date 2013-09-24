#include<iostream>
#include<sstream>

using namespace std;

string clockwise(int size){
    if(size <= 0 || size & 1 == 0){
        return string("");
    }
    int i = 0, j = 0;
    int** array = new int*[size];
    for(i = 0; i < size; i++){
        for(j = 0; j < size; j++){
            array[i] = new int[size];
        }
    }

    int circle = size / 2;
    i = circle, j = circle;
    int num = 1, steps = 2;
    int m = 0;
    while(m < circle){
        int k = 0;
        for(k = 0; k < steps; ++k){
            array[i][j++] = num++;
        }
        --j;
        ++i;
        for(k = 0; k < steps - 1; ++k){
            array[i++][j] = num++;
        }
        --i;
        --j;
        for(k = 0; k < steps ; ++k){
            array[i][j--] = num++;
        }
        ++j;
        --i;
        for(k = 0; k < steps - 1; ++k){
            array[i--][j] = num++;
        }
        steps += 2;
        ++m;
    }

    for(i = 0; i < size; ++i){
        array[0][i] = num++;
    }

    stringstream ss;
    for(i = 0; i < size; ++i){
        for(j = 0; j < size - 1; ++j){
            ss << array[i][j] << "*";
        }
        ss << array[i][j] << endl;
    }

    for(i = 0; i < size; i++){
        delete [] array[i];
    }
    delete [] array;

    return ss.str();
}

int main(){
    int size = 3;
    cout << clockwise(size);
    return 0;
}
