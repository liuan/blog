#include<iostream>
#include<iterator>
#include<vector>

using namespace std;

struct BinaryTree{
    int value;
    BinaryTree* left;
    BinaryTree* right;
    BinaryTree(int v, BinaryTree* l, BinaryTree *r){
        value = v;
        left = l;
        right = r;
    }
    friend ostream& operator<<(ostream& os, const BinaryTree* tree);
};


ostream& operator<<(ostream& os, const BinaryTree* tree){
    return os << tree->value;
}

bool getpath(BinaryTree* tree, BinaryTree* node, vector<BinaryTree*>& vec){
    if(!tree)
        return false;
    vec.push_back(tree);
    if(tree->value == node->value)
        return true;
    bool found = false;
    if(tree->left){
        found = getpath(tree->left,node,vec);
    }
    if(!found && tree->right){
        //vec.pop_back();
        found = getpath(tree->right,node,vec);
    }
    if(!found)
        vec.pop_back();
    return found;
}

BinaryTree* nearestcommonancestor(BinaryTree* tree, BinaryTree* left, BinaryTree* right){
    if(!tree || !left || !right)
        return NULL;
    vector<BinaryTree* > vec1, vec2;
    if(!getpath(tree,left,vec1) || !getpath(tree,right,vec2)){
        return NULL;
    }

    copy(vec1.begin(),vec1.end(),ostream_iterator<BinaryTree*,char>(cout, " "));
    cout << endl;
    copy(vec2.begin(),vec2.end(),ostream_iterator<BinaryTree*,char>(cout, " "));
    cout << endl;

    int i = 0, j = 0;
    while(i < vec1.size() && j < vec2.size() && vec1[i] == vec2[j]){
        ++i; 
        ++j;
    }

    return vec1[i-1];
}

void test_nearestcommonancestor(){
    BinaryTree one(4,NULL,NULL);
    BinaryTree two(5,NULL,NULL);
    BinaryTree three(6,NULL,NULL);
    BinaryTree four(2,&one,&two);
    //BinaryTree four(2,&one,NULL);
    BinaryTree five(3,NULL,&three);
    BinaryTree six(1,&four,&five);
    BinaryTree *result = nearestcommonancestor(&six,&one,&four);
    cout << (result != NULL ? result->value : 0) << endl;
}

int main(){
    test_nearestcommonancestor();
    return 0;
}
