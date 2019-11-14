#include <windows.h>

#define OK_DUMP
#define DO_GRAPH
#include "Tree_t\Tree.cpp"
#include "My_Headers\txt_files.h"

#define MIPT_FORMAT " \"%[^\"]\" "
const char BEGIN_SEPARATOR = '{';
const char END_SEPARATOR = '}';
const char VALUE_SEPARATOR[] = "\"";

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

//Специализация методов класса, для корректной работы с фразами.
template <>
void Tree<Phrase>::valueDestruct() {
    value.phrase = nullptr;
}
template <>
void Tree<Phrase>::valuePrint(FILE *file) {
    fprintf(file, "%s" , value.phrase);
}

char* readNode(char* ptr, Tree<Phrase>* node);
Tree<Phrase>* parse(char** buff);

int main() {
    SetConsoleOutputCP(CP_UTF8);
    FILE* log = fopen64(TREE_LOG_NAME, "wb"); //clearing log file
    fclose(log);

    char* buffer = nullptr;
    Tree<Phrase>* ackinator = parse(&buffer);
    ackinator->treeVerify(__FILE__, __PRETTY_FUNCTION__, __LINE__);

    delete (ackinator);
    free(buffer);
    return 0;
}

Tree<Phrase>* parse(char** buff) {
    size_t length = 0;
    char* buffer = read_file_to_buffer_alloc(DATA_BASE, "r", &length);
    Tree<Phrase>* node = new Tree<Phrase>({nullptr});
    readNode(buffer, node);
    *buff = buffer;
    return node;
}

char *readNode(char *ptr, Tree<Phrase> *node) { //format { "quest" { "answer" }{ "answer" }}   //RIGHT - no, LEFT - YES
    assert(node);
    assert(ptr);
    ptr = strchr(ptr, *VALUE_SEPARATOR);
    node->setValue({strtok(ptr, VALUE_SEPARATOR)});
    ptr = strchr(ptr, '\0') + 1;
    while (strchr(ptr, BEGIN_SEPARATOR) != nullptr && strchr(ptr, BEGIN_SEPARATOR) < strchr(ptr, END_SEPARATOR)) {
        ptr = strchr(ptr, BEGIN_SEPARATOR);
        for (int i = 0; i < NUMBER_OF_CHILDREN; ++i) {
            if (node->getChild(i) == (Tree<Phrase>*)node->NIL) {
                node->growChild(i, {nullptr});
                ptr = readNode(ptr, node->getChild(i)) + 1;
                break;
            }
        }
    }
    ptr = strchr(ptr, END_SEPARATOR);
    return ptr;
}
