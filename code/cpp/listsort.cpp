#include<iostream>

using namespace std;

struct List{
    int value;
    List* next;
    List(int va, List* ne){
        value = va;
        next = ne;
    }
};

void print(List *list,List *tail){
    if(list == NULL)
        return;
    while(list != NULL && list != tail){
        cout << list->value << " "; 
        list = list->next;
    }
    cout << endl;
}

List* partition(List* start, List *end){
    if(start == NULL || start->next == end) 
        return NULL;
    int key = start->value;
    List* small = start, *big = start->next;
    while(big && big != end){
        if(big->value <= key){
            small = small->next;
            int temp = small->value;
            small->value = big->value;
            big->value = temp;
        }
        big = big->next;
    }
    
    if(small && small->value <= key){
        start->value = small->value;
        small->value = key;
    }

    return small;
}

void quicksort(List* start, List* end){
    if(start == NULL || start->next == end)
        return;
    List *pivot = partition(start,end);
    if(start != pivot)
        quicksort(start,pivot);
    if(pivot)
        quicksort(pivot->next,end);
}

void test_quicksort(){
    //3 6 17 15 13 15 6 12 9 1
    List list5(1,NULL);
    List list1(9,&list5);
    List list6(12,&list1);
    List list7(6,&list6);
    List list3(15,&list7);
    List list2(13,&list3);
    List list0(15,&list2);
    List list8(17,&list0);
    List list9(6,&list8);
    List list(3,&list9);
    print(&list,NULL);
    quicksort(&list,NULL);
    print(&list,NULL);
}

void test_quicksortv1(){
    // 5 3 7 4 8
     
    List list1(8,NULL);
    List list2(4,&list1);
    List list3(7,&list2);
    List list4(3,&list3);
    List list5(5,&list4);
    List list = list5;
    print(&list,NULL);
    quicksort(&list,NULL);
    print(&list,NULL);

}

int main(){
    test_quicksort();
    test_quicksortv1();
    return 0;
}
