#include<iostream>

using namespace std;

typedef bool T;

typedef struct segtree{
    int start;
    int end;
    T   value;
}SegTree;

void construct(SegTree *segtree,int len, int index, int left, int right){
    if(segtree == NULL || index >= len || index < 0)
        return;
    segtree[index].start = left;
    segtree[index].end = right;
    if(right - left == 1){
        segtree[index].value = false;
        return;
    }
    int middle = (left + right) >> 1;
    construct(segtree,len,(index<<1) + 1,left,middle);
    construct(segtree,len,(index<<1) + 2,middle,right);
    segtree[index].value = false;
}

void insert(SegTree *segtree, int len, int index, int left, int right){
    if(segtree == NULL || index >= len || index < 0)
        return;
    if(segtree[index].value == true)
        return;
    if(segtree[index].start == left && segtree[index].end == right){
        segtree[index].value = true;
        return;
    }

    int middle = (segtree[index].start + segtree[index].end) >> 1;
    if(right <= middle){
        insert(segtree,len,(index<<1)+1,left,right);
    }else if(left >= middle){
        insert(segtree,len,(index<<1)+2,left,right);
    }else{
        insert(segtree,len,(index<<1)+1,left,middle); 
        insert(segtree,len,(index<<1)+2,middle,right); 
    }
}

int count(SegTree *segtree, int index){
    if(segtree[index].value == true)
        return segtree[index].end - segtree[index].start;
    else if(segtree[index].end - segtree[index].start == 1)
        return 0;
    return count(segtree,(index<<1)+1) + count(segtree,(index<<1)+2);
}

void print(SegTree *segtree, int len){
    int i = 0;
    for(i; i < len; i++){
        cout << "index:" << i << " left:" << segtree[i].start << " right:" << segtree[i].end 
            << " value:" << segtree[i].value << endl;
    }

}

int main(){
    int segment[10][2] = {5,8, 10,45, 0,7, 2,3, 3,9, 13,26, 15,38, 50,67, 39,42, 70,80};
    SegTree segtree[4*100];
    construct(segtree,4*100,0,0,100);
    int i = 0;
    for(i; i < 10; i++){
        insert(segtree,4*100,0,segment[i][0],segment[i][1]);
    }

    print(segtree,50);
    //cout << "result:" << count(segtree,0) << endl;

    return 0;
}
