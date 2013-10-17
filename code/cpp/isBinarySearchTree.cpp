#include<iostream>

using namespace std;

struct BinaryTree{
    int value;
    struct BinaryTree* left;
    struct BinaryTree* right;
    BinaryTree(struct BinaryTree* l, struct BinaryTree* r, int va=0){
        left = l;
        right = r;
        value = va;
    }
};

bool isBinarySearchTree(BinaryTree *tree){
    if(tree == NULL)
        return true;
    bool right = true;
    if(tree->left && tree->value <= tree->left->value)
        right = false;
    if(right && tree->right && tree->value >= tree->right->value)
        right = false;
    return (right && isBinarySearchTree(tree->left)&&isBinarySearchTree(tree->right));
}

void test_isbinarysearchtree(){
    BinaryTree a(NULL,NULL,1); 
    BinaryTree b(&a,NULL,2); 
    BinaryTree c(&b,NULL,3); 
    BinaryTree d(NULL,NULL,4); 
    BinaryTree e(NULL,&d,5); 
    BinaryTree f(NULL,&e,6); 
    BinaryTree g(NULL,&f,7); 
    BinaryTree h(&c,&g,8); 
    BinaryTree i(NULL,NULL,9); 
    BinaryTree j(&h,&i,10); 
    cout << "result: " << (isBinarySearchTree(&c)? "true":"false") << endl;
    cout << "result: " << (isBinarySearchTree(&j)? "true":"false") << endl;
}

int main(){
    test_isbinarysearchtree();
    return 0;
}
