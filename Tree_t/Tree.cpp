#include <cstdlib>
#include <time.h>
#include <assert.h>
#include "Tree.h"
#define VERIFY_CONTEXT __FILE__ , __PRETTY_FUNCTION__ , __LINE__

#define TREE_ASSERT(condition, message) \
if (!(condition)) {\
    nodeDump(file, ERROR_STATE, #message, filename, function, line);\
    treeDump(file, ERROR_STATE, #message, filename, function, line, sequence);\
    assert(condition);\
    return false;\
}

template <class T>
Tree<T>::Tree(T value) : value(value){
    parent = (Tree<T>*)ROOT_PARENT;
    children = (Tree<T>**)calloc(NUMBER_OF_CHILDREN, sizeof(children[0]));
    assert(children);
    for (int i = 0; i < NUMBER_OF_CHILDREN; ++i) {
        children[i] = (Tree<T>*)NIL;
    }
    size = (size_t*)calloc(1, sizeof(size));
    assert(size);
    *size = 1;
    treeVerify(VERIFY_CONTEXT);
}

template <class T>
Tree<T>::Tree(T _value, Tree<T>* _parent) : value(_value), parent(_parent), size(_parent->size){
    assert(size);
    children = (Tree<T>**)calloc(NUMBER_OF_CHILDREN, sizeof(children[0]));
    assert(children);
    for (int i = 0; i < NUMBER_OF_CHILDREN; ++i) {
        children[i] = (Tree<T>*)NIL;
    }
    incSize();
}

template <class T>
Tree<T>::~Tree() {
    treeVerify(VERIFY_CONTEXT);
    for (int i = 0; i < NUMBER_OF_CHILDREN; ++i) {
        if (!childIsEmpty(i)) {
            delete(getChild(i));
            setChild(i, (Tree<T>*)NIL);
        }
    }
    free(this->children);
    this->children = nullptr;
    decSize();
    if (isRoot()){
        free(size);
    }
    size = nullptr;
    this->parent = nullptr;
    valueDestruct();
}

template <class T>
size_t Tree<T>::getSize() {
    return *size;
}

template <class T>
void Tree<T>::incSize() {
    (*size)++;
}

template <class T>
void Tree<T>::decSize() {
    (*size)--;
}

template <class T>
Tree<T>* Tree<T>::getChild(size_t index) {
    return children[index];
}

template <class T>
bool Tree<T>::childIsEmpty(size_t index) {
    return getChild(index) == (Tree<T>*)NIL;
}

template <class T>
void Tree<T>::setChild(size_t index, Tree<T> *node) {
    children[index] = node;
}

template <class T>
Tree<T>* Tree<T>::growChild(size_t index, T _value){
    treeVerify(VERIFY_CONTEXT);
    if (childIsEmpty(index)) {
        setChild(index, new Tree<T>(_value, this));
        treeVerify(VERIFY_CONTEXT);
        return getChild(index);
    }
    treeVerify(VERIFY_CONTEXT);
    return nullptr;
}
template <class T>
void Tree<T>::removeSubTree(size_t index) {
    treeVerify(VERIFY_CONTEXT);
    delete(getChild(index));
    setChild(index, (Tree<T>*)NIL);
    treeVerify(VERIFY_CONTEXT);
}
template <class T>
Tree<T>* Tree<T>::getParent(){
    return parent;
}

template <class T>
void Tree<T>::setParent(Tree<T> *node) {
    parent = node;
}

template <class T>
T Tree<T>::getValue() {
    return this->value;
}

template <class T>
void Tree<T>::setValue(T _value) {
    this->value = _value;
}

template<class T>
Tree<T> *Tree<T>::getRoot() {
    if (isRoot()) {
        return this;
    } else {
        return getParent()->getRoot();
    }
}

template<class T>
bool Tree<T>::isRoot() {
    return getParent() == (Tree<T>*)ROOT_PARENT;
}

template<class T>
Tree<T>** Tree<T>::preorder(Tree<T> **sequence) {
    assert(this);
    sequence++[0] = this;
    if (!childIsEmpty(LEFT_CHILD)) {
        sequence = getChild(LEFT_CHILD)->preorder(sequence);
    }
    if (!childIsEmpty(RIGHT_CHILD)) {
        sequence = getChild(RIGHT_CHILD)->preorder(sequence);
    }
    return sequence;
}

template<class T>
Tree<T> **Tree<T>::inorder(Tree<T> **sequence) {
    assert(this);
    if (!childIsEmpty(LEFT_CHILD)) {
        sequence = getChild(LEFT_CHILD)->inorder(sequence);
    }
    sequence++[0] = this;
    if (!childIsEmpty(RIGHT_CHILD)) {
        sequence = getChild(RIGHT_CHILD)->inorder(sequence);
    }
    return sequence;
}

template<class T>
Tree<T> **Tree<T>::postorder(Tree<T> **sequence) {
    assert(this);
    if (!childIsEmpty(LEFT_CHILD)) {
        sequence = getChild(LEFT_CHILD)->postorder(sequence);
    }
    if (!childIsEmpty(RIGHT_CHILD)) {
        sequence = getChild(RIGHT_CHILD)->postorder(sequence);
    }
    sequence++[0] = this;
    return sequence;
}

template<class T>
Tree<T>* Tree<T>::put(T _value) {
    treeVerify(VERIFY_CONTEXT);
    if (_value < this->value) {
        if (childIsEmpty(LEFT_CHILD)) {
            return growChild(LEFT_CHILD, _value);
        } else {
            return getChild(LEFT_CHILD)->put(_value);
        }
    } else /*if (_value >= this->value)*/{
        if (childIsEmpty(RIGHT_CHILD)) {
            return growChild(RIGHT_CHILD, _value);
        } else {
            return getChild(RIGHT_CHILD)->put(_value);
        }
    }
}

template<class T>
Tree<T> **Tree<T>::allocTree() {
    Tree<T>** buffer = (Tree<T>**)calloc(getSize(), sizeof(buffer[0]));
    return buffer;
}

template<class T>
void Tree<T>::nodeDump(FILE *log, const char *state, const char *message, const char *filename, const char *function, int line) {
#ifndef NDEBUG
    time_t now = time(nullptr);
    fprintf(log, "\nLog from %s"
                 "Tree node Dump(%s) from %s %s %d\n"
                 "\tTree node [%p] (%s)\n"
                 "\t{\n"
                 "\t\tparent = %p (%s)\n"
                 "\t\tsize[%p] = %d\n"
                 "\t\tchildren[%p]\n"
                 "\t\t{\n",
                 ctime(&now), message, filename, function, line, this, state, parent, isRoot() ? "ROOT" : "", size, getSize(), children);
    for (int i = 0; i < NUMBER_OF_CHILDREN; ++i) {
        fprintf(log, "\t\t\t[%d]: %p\n", i, children[i]);
    }
    fprintf(log, "\t\t}\n"
                 "\t\tvalue = ");
    valuePrint(log);
    fprintf(log, "\n"
                 "\t}\n");
#endif
}

template<class T>
void Tree<T>::genDot(Tree<T> *node, FILE *file) {
#ifndef NDEBUG
    fprintf(file, "T%p [shape = record, label = \" {value\\n", node);
    node->valuePrint(file);
    fprintf(file, " | this\\n%p | parent\\n%p  | {left\\n%p | right\\n%p}} \"];\n\t", node, node->getParent(), node->getChild(LEFT_CHILD), node->getChild(RIGHT_CHILD));
    for (int i = 0; i < NUMBER_OF_CHILDREN; ++i) {
        if (!node->childIsEmpty(i))
            fprintf(file, "T%p -> T%p[label = \"%s\"];", node, node->getChild(i), i ? "Right" : "Left");
    }
    fprintf(file, "\n\t");
#endif
}

template <class T>
void Tree<T>::graphDump(const char *outFIle, Tree<T>** sequence) {
    FILE* dotlog = fopen(TREE_DOT, "wb");
    fprintf(dotlog, "digraph G{\n"
                    "\trankdir = TB;\n"
                    "\t");
    for (int i = 0; i < getSize(); ++i) {
        genDot(sequence[i], dotlog);
    }
    fprintf(dotlog, "\n}");
    fclose(dotlog);
    char sys_query[sizeof(TREE_DOT) + sizeof(outFIle) + DOT_QUERY_SIZE];
    sprintf(sys_query, "dot -Tpng %s -o %s", TREE_DOT, outFIle);
    system(sys_query);
    DOT_FILE_COUNTER++;
}

template <class T>
void Tree<T>::treeDump(FILE *log, const char *state, const char *message, const char *filename, const char *function, int line, Tree<T>** sequence) {
#ifndef NDEBUG
    fprintf(log, "\nbegin {Tree Dump} ROOT = [%p]\n"
                 "(%s) %s\n", getRoot(), state, message);

#ifdef DO_GRAPH
    fprintf(log, "Graph in file: %d%s", DOT_FILE_COUNTER, TREE_PNG);
    char autoName[sizeof(DOT_FILE_COUNTER) + sizeof(TREE_PNG)] = "";
    sprintf(autoName, "%d%s", DOT_FILE_COUNTER, TREE_PNG);
    graphDump(autoName, sequence);
#endif

    for (int i = 0; i < getSize(); ++i) {
        sequence[i]->nodeDump(log, state, "Tree dump member", filename, function, line);
    }
    fprintf(log, "end {treeDump}\n");
#endif //NDEBUG
}

template <class T>
bool Tree<T>::treeVerify(const char filename[], const char function[], int line) {
#ifndef NDEBUG
    assert(this);
    FILE* file = fopen(TREE_LOG_NAME, "ab");
    Tree<T>** sequence = (Tree<T>**)calloc(getSize(), sizeof(sequence[0]));
    getRoot()->preorder(sequence);

    TREE_ASSERT(children != nullptr, Children pointer is NULL)
    TREE_ASSERT(size != nullptr, size pointer is NULL)
    TREE_ASSERT(getSize() > 0, incorrect size)
    if (!childIsEmpty(LEFT_CHILD)) {
        TREE_ASSERT(getChild(LEFT_CHILD)->value < value, Left child bigger then parent)
        TREE_ASSERT(getChild(LEFT_CHILD)->getSize() == getSize(), Child size variable mismatch)
    }
    if (!childIsEmpty(RIGHT_CHILD)) {
        TREE_ASSERT(getChild(RIGHT_CHILD)->value >= value, Right child less then parent)
        TREE_ASSERT(getChild(RIGHT_CHILD)->getSize() == getSize(), Child size variable mismatch)
    }
#ifdef OK_DUMP
    treeDump(file, OK_STATE, "It's ok ^..^", filename, function, line, sequence);
#endif

    free(sequence);

    fclose(file);
#endif
    return true;
}

template <class T>
void Tree<T>::valuePrint(FILE *file) {
    fprintf(file, "/!specialize valuePrint() method for output value!/");
}

template<>
void Tree<int>::valueDestruct() {
    value = 0;
}

template <>
void Tree<int>::valuePrint(FILE *file) {
    fprintf(file, "%d", value);
}

#undef TREE_ASSERT
#undef VERIFY_CONTEXT