#include<iostream>
#include<deque>

using namespace std;

struct BinaryTree{
    int value;
    BinaryTree *left;
    BinaryTree *right;
    BinaryTree(int va, BinaryTree* lh, BinaryTree* rh){
        value = va;
        left = lh;
        right = rh;
    }
};

void printBinaryTreeByFloor(const BinaryTree* tree){
    if(!tree)
        return;
    deque<const BinaryTree* > queue;
    BinaryTree sign(0,NULL,NULL);
    queue.push_back(tree);
    queue.push_back(&sign);
    while(!queue.empty()){
        const BinaryTree* temp = queue.front();
        if(temp == &sign){
            cout << endl;
            if(queue.back() != &sign)
                queue.push_back(&sign);
        }else{
            cout << temp->value << " ";
        }
        queue.pop_front();
        if(temp->left)
            queue.push_back(temp->left);
        if(temp->right)
            queue.push_back(temp->right);
    }
}

void test_printbinarytreebyfloor(){
    BinaryTree node1(1,NULL,NULL);
    BinaryTree node3(3,NULL,NULL);
    BinaryTree node2(2,&node1,&node3);
    BinaryTree node5(5,NULL,NULL);
    BinaryTree node6(6,&node5,NULL);
    BinaryTree node4(4,&node2,&node6);

    printBinaryTreeByFloor(&node4);
}

int main(){
    test_printbinarytreebyfloor();
    return 0;
}
