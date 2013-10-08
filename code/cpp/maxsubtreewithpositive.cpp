#include<iostream>
#include<climits>

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

void print(BinaryTree* tree){
    if(!tree)
        return;
    print(tree->left);
    cout << tree->value << " ";
    print(tree->right);
}

BinaryTree *maxsubtreewithpositive(BinaryTree* root, bool isnegative, int& sum){
    if(!root)
        return NULL;
    
    return NULL;

}

void maxsubtreewithpositive(BinaryTree* root, BinaryTree** subtree, int& sum, int& curmax){
    if(!root)
        return;
    int left = 0, right = 0;
    if(root->left)
        maxsubtreewithpositive(root->left,subtree,left,curmax);
    if(root->right)
        maxsubtreewithpositive(root->right,subtree,right,curmax);

    if(root->value > 0){
        sum = left + right + root->value;
    }
    else
        //sum = 0x80000000;
        sum = INT_MIN;
    if(sum > curmax){
        curmax = sum;
        *subtree = root;
    }

}

void maxsubtree(BinaryTree* root, BinaryTree** cursubtree, int& sum, int& curmax){
    if(!root)
        return;
    int left = 0, right = 0;
    //BinaryTree *lefttree = NULL, *righttree = NULL;
    if(root->left != NULL)
        //lefttree = maxsubtree(root->left,left);  
        maxsubtree(root->left,cursubtree,left,curmax);  
    if(root->right != NULL)
        //righttree = maxsubtree(root->right,right);  
        maxsubtree(root->right,cursubtree,right,curmax);  

    sum = left + right + root->value; 
    if(sum > curmax){
        *cursubtree = root;
        curmax = sum;
    }
         
}

void test_maxsubtreewithpositive(BinaryTree* tree){
    if(!tree)
        return;
    cout << "*******test max subtree with positive**********" << endl;
    print(tree);
    cout << endl;
    cout << "*****************************" << endl;
    BinaryTree* subtree = NULL;
    int curmax = 0, sum = 0;
    maxsubtreewithpositive(tree,&subtree,sum,curmax);
    cout << "curmax:" << curmax << endl;
    print(subtree);
    cout << endl << endl;
}

void test_maxsubtree(BinaryTree* tree){
    if(!tree)
        return;
    cout << "*******test max subtree**********" << endl;
    print(tree);
    cout << endl;
    cout << "*****************************" << endl;
    BinaryTree* subtree = NULL;
    int curmax = 0, sum = 0;
    maxsubtree(tree,&subtree,sum,curmax);
    cout << "curmax:" << curmax << endl;
    print(subtree);
    cout << endl << endl;
}

//BinaryTree* construct_binarytree(){
void construct_binarytree(){
    BinaryTree one(6,NULL,NULL);
    BinaryTree two(2,NULL,NULL);
    BinaryTree three(3,NULL,NULL);
    BinaryTree four(9,&two,&three);
    BinaryTree five(-7,&one,&four);
    BinaryTree six(-1,NULL,NULL);
    BinaryTree seven(-2,&five,&six);
    BinaryTree eight(10,&seven,NULL);
    //return &eight;
}

int main(){
    BinaryTree one(6,NULL,NULL);
    BinaryTree two(2,NULL,NULL);
    BinaryTree three(3,NULL,NULL);
    BinaryTree four(9,&two,&three);
    BinaryTree five(-7,&one,&four);
    BinaryTree six(-1,NULL,NULL);
    BinaryTree seven(-2,&five,&six);
    BinaryTree eight(10,&seven,NULL);

    BinaryTree *root = &eight;
    test_maxsubtree(root);
    test_maxsubtreewithpositive(root);
}

