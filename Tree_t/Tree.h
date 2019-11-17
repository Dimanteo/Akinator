#ifndef TREE_H
#define TREE_H

#include <cstddef>
#include <cstdio>

const size_t LEFT_CHILD = 0;
const size_t RIGHT_CHILD = 1;
int DOT_FILE_COUNTER = 0;
const size_t NUMBER_OF_CHILDREN = 2;
const char TREE_LOG_NAME[] = "Tree.log";
const char ERROR_STATE[] = "ERROR";
const char OK_STATE[] = "ok";
const int DOT_QUERY_SIZE = 30;
char TREE_DOT[] = "TreeGraph.dot";
char TREE_PNG[] = "_Tree.png";

template <class T>
class Tree {

private:
    Tree<T>** children;
    Tree<T>* parent;
    T value;
    size_t* size;

    Tree(T value, Tree<T>* parent);

    void setChild(size_t index, Tree<T>* node);
    void setParent(Tree<T>* node);
    void incSize();
    void decSize();
    void genDot(Tree<T> *node, FILE *file);

public:

    const Tree<T>* ROOT_PARENT = nullptr;
    const Tree<T>* NIL = nullptr;

    Tree(T value);

    ~Tree();

    Tree<T>* getChild(size_t index);

    Tree<T>* getParent();

    bool isRoot();

    size_t getSize();

    Tree<T>* getRoot();

    T getValue();
    void setValue(T value);
    void valuePrint(FILE *file);
    void valueDestruct();

    Tree<T>* growChild(size_t index, T value);
    void removeSubTree(size_t index);
    bool childIsEmpty(size_t index);

    Tree<T>* put(T _value);

    Tree<T>** allocTree();

    Tree<T>** preorder(Tree<T>** sequence);

    Tree<T>** inorder(Tree<T>** sequence);

    Tree<T>** postorder(Tree<T>** sequence);

    void nodeDump(FILE *log, const char *state, const char *message, const char *filename, const char *function, int line);

    void treeDump(FILE *log, const char *state, const char *message, const char *filename, const char *function, int line, Tree<T>** sequence);

    void graphDump(const char *outFIle, Tree<T>** sequence);

    bool treeVerify(const char filename[], const char function[], int line);
};

#endif //TREE_H
