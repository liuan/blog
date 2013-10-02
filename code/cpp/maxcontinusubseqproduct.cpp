#include<iostream>
#include<iterator>
#include<vector>

using namespace std;

double maxcontinuesubseqproduct(const vector<double>& vec){
    int len = vec.size();
    if(len <= 0)
        return 0;
    vector<double> dpmin(len,0), dpmax(len,0);
    dpmin.push_back(vec[0]);
    dpmax.push_back(vec[0]);

    int i = 1;
    double res = 0;
    for(; i < len; ++i){
        dpmax[i] = max(vec[i],dpmax[i-1]*vec[i]);
        dpmax[i] = max(dpmax[i],dpmin[i-1]*vec[i]);

        dpmin[i] = min(vec[i],dpmax[i-1]*vec[i]);
        dpmin[i] = min(dpmin[i],dpmin[i-1]*vec[i]);

        res = dpmax[i] > res ? dpmax[i] : res;
    }

    if(res >= 0)
        return res;
    else
        return -1;
}

void test_maxcontinuesubseqproduct(){
    double array[] = {-2.5, 4, 0, 3, 0.5, 8, -1};
    copy(array,array + sizeof(array)/sizeof(double), ostream_iterator<double, char>(cout, " "));
    cout << endl;
    vector<double> vec(array,array + sizeof(array)/sizeof(double));
    cout << "max: "  << maxcontinuesubseqproduct(vec) << endl;
}

int main(){
    test_maxcontinuesubseqproduct();
    return 0;
}
