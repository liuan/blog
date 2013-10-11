#include<iostream>
#include<deque>

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

void printbyfloor(BinaryTree* tree){
    if(!tree)
        return;
    deque<BinaryTree*> queue;
    queue.push_back(tree);
    BinaryTree temp(NULL,NULL);
    queue.push_back(&temp);
    BinaryTree* node = NULL;
    while(!queue.empty()){
        node = queue.front();
        queue.pop_front();
        if(node == &temp){
            cout << endl;
            if(!queue.empty())
                queue.push_back(&temp);
        }else{
            cout << node->value << " ";
            if(node->left)
                queue.push_back(node->left);
            if(node->right)
                queue.push_back(node->right);
        }
    }
}

int maxdistanceinbinarytree(BinaryTree* tree, int& distance){
    if(tree == NULL){
        distance = 0;
        return 0;
    }
    
    int left = 0, right = 0;
    int lmax = 0, rmax = 0;
    if(tree->left)
        lmax = maxdistanceinbinarytree(tree->left,left);
    if(tree->right)
        rmax = maxdistanceinbinarytree(tree->right,right);

    distance = max(left,right) + 1;

    return max(lmax,max(rmax,left+right));
}

void test_distance(){
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
   printbyfloor(&j);

   int distance = 0;
   cout << maxdistanceinbinarytree(&j,distance) << endl;
}

int main(){
    test_distance();
    return 0;
}
