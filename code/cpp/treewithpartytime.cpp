#include<iostream>
#include<vector>
#include<iterator>


using namespace std;

typedef struct tree{
    int value;
    struct tree *left;
    struct tree *right;
    tree(int val, tree* l, tree* r){
        value = val;
        left = l;
        right = r;
    }
}BinaryTree;

void inorder(BinaryTree *root){
    if(root == NULL)
        return;
    inorder(root->left);
    cout << root->value << " ";
    inorder(root->right);
}

void partytime(BinaryTree *root,int *weight){
    if(root == NULL)
        return;
    int left = 0, right = 0;  
    partytime(root->left,&left); 
    partytime(root->right,&right); 

    if(left + right > root->value){
        *weight = left + right;
    }else{
        *weight = root->value;    
    }
}

void partytime(BinaryTree *root,int *weight,vector<int>& vec){
    if(root == NULL)
        return;
    int left = 0, right = 0;  
    partytime(root->left,&left,vec); 
    partytime(root->right,&right,vec); 

    if(left + right > root->value){
        *weight = left+right;
    }else
        *weight = root->value;    
    vec.push_back(*weight);
}

void tracingback(BinaryTree *node,vector<int>& vec,int index){
    if(node == NULL || vec.size() <= index)
        return;
    if(node->value == vec.at(index)){
        cout << node->value << " ";
        return;
    }
    int i = 0;
    for(i; i < index; i++){
        if(vec[i] == vec.at(index)-vec.at(index-1))
            break;
    }
    tracingback(node->left,vec,i);     
    tracingback(node->right,vec,index-1);     
}

int main(){
    BinaryTree one(4,NULL,NULL);
    BinaryTree tmp(5,NULL,NULL);
    BinaryTree two(3,&one,&tmp);
    BinaryTree thr(2,NULL,NULL);
    BinaryTree fou(12,&thr,&two);
    BinaryTree fiv(3,NULL,NULL);
    BinaryTree six(6,NULL,&fiv);
    BinaryTree sev(2,NULL,NULL);
    BinaryTree egi(5,&six,&sev);
    BinaryTree root(10,&fou,&egi);

    inorder(&root);
    cout << endl;
    int weight = 0;
    vector<int> vec;
    partytime(&root,&weight);
    partytime(&root,&weight,vec);
    copy(vec.begin(),vec.end(),ostream_iterator<int,char>(cout," "));
    cout << endl;
    cout << "tracing back: ";
    tracingback(&root,vec,vec.size()-1);
    cout << endl;
    
    return 0;
}
