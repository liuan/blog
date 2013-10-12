#include<iostream>

using namespace std;

struct BinaryTree{
    int value;
    BinaryTree* left;
    BinaryTree* right;
    BinaryTree(int va, BinaryTree* l, BinaryTree* r){
        value = va;
        left = l;
        right = r;
    }
};

bool symmetrical(BinaryTree* left, BinaryTree* right){
    if(!left && !right)
        return true;
    if(left != NULL && right == NULL)
        return false;
    if(left == NULL && right != NULL)
        return false;
    return symmetrical(left->left,right->right) && symmetrical(left->right,right->left);
}

bool isSymmetrical(BinaryTree* root){
    if(!root)
        return true;
    return symmetrical(root->left,root->right);
}

void test_issymmetrical(){
    BinaryTree one(4,NULL,NULL);
    BinaryTree two(5,NULL,NULL);
    BinaryTree three(6,NULL,NULL);
    //BinaryTree four(2,&one,&two);
    BinaryTree four(2,&one,NULL);
    BinaryTree five(3,NULL,&three);
    BinaryTree six(6,&four,&five);
    cout << "is symmetrical:" << (isSymmetrical(&six) ? "true" : "false") << endl; 

}

int main(){
    test_issymmetrical();
    return 0;
}
