#include <windows.h>

#define OK_DUMP
#define DO_GRAPH
#include "Tree_t\Tree.cpp"
#include "My_Headers\txt_files.h"

#define MIPT_FORMAT " \"%[^\"]\" "

struct Phrase {
    char* phrase;
};
bool operator>=(Phrase a, Phrase b) {
    return true;
}
bool operator<(Phrase a, Phrase b) {
    return true;
}

const char DATA_BASE[] = "Test.txt";
const size_t PHRASE_LENGTH = 1000;

template <>
void Tree<Phrase>::valueDestruct() {
    value.phrase = nullptr;
}
template <>
void Tree<Phrase>::valuePrint(FILE *file) {
    fprintf(file, "%s", value.phrase);
}

char* readNode(char* ptr, Tree<Phrase>* node, size_t index) {
    ptr = strchr(ptr, '{') + 1;
    char phrase[PHRASE_LENGTH] = "";
    int length = 0;
    sscanf(ptr, MIPT_FORMAT "%n", phrase, &length);
    ptr += length;
    if (node == nullptr) {
        node = new Tree<Phrase>({phrase});
        if (strchr(ptr, '{') != nullptr && strchr(ptr, '{') < strchr(ptr, '}')) {
            ptr = readNode(strchr(ptr, '{'), node, LEFT_CHILD);
            ptr = readNode(strchr(ptr, '{'), node, RIGHT_CHILD);
        }
    } else {
        node->growChild(index, {phrase});
        if (strchr(ptr, '{') != nullptr && strchr(ptr, '{') < strchr(ptr, '}')) {
            ptr = readNode(strchr(ptr, '{'), node->getChild(index), LEFT_CHILD);
            ptr = readNode(strchr(ptr, '{'), node->getChild(index), RIGHT_CHILD);
        }
    }
    ptr = strchr(ptr, '}');
    return ptr;
}

Tree<Phrase>* parser() {
    size_t length = 0;
    char* buffer = read_file_to_buffer_alloc(DATA_BASE, "r", &length);
    char* ptr = buffer;
    Tree<Phrase>* node = nullptr;
    readNode(ptr, node, 0);
    return node;
}

int main() {
    SetConsoleOutputCP(CP_UTF8);
    Tree<Phrase>* ackinator = parser();
    Tree<Phrase>** preorder = (Tree<Phrase>**)calloc(ackinator->getSize(), sizeof(preorder[0]));
    ackinator->preorder(preorder);
    for (int i = 0; i < ackinator->getSize(); ++i) {
        preorder[i]->valuePrint(stdout);
        printf(" ");
    }
    delete (ackinator);
    return 0;
}
