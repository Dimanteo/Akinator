#include <windows.h>
#include <cmath>

#define OK_DUMP
#include "Tree_t\Tree.cpp"
#include "My_Headers\txt_files.h"

//Переменные формата
#define MIPT_FORMAT " \"%[^\"]\" "
const char BEGIN_SEPARATOR = '{';
const char END_SEPARATOR = '}';
const char VALUE_SEPARATOR[] = "\"";
const char YES[] = "Y";
const char NO[] = "N";
const char ANSWER_FORMAT[] = "%1s";

const int TRIES = 20;
const size_t ANSWER_SIZE = 4000;

//Стрктура элемента дерева
struct Phrase {
    char* phrase;
    bool allocated = 0;
};

bool operator>=(Phrase a, Phrase b) {
    return true;
}
bool operator<(Phrase a, Phrase b) {
    return true;
}

//Имена фалов с базой данных
const char DATA_BASE_INPUT[] = "Test.txt";
const char DATA_BASE_OUTPUT[] = "AUTO_GEN_DATABASE.txt";


//Специализация методов класса, для корректной работы с фразами.
template <>
void Tree<Phrase>::valueDestruct() {
    if (value.allocated)
        free(value.phrase);

    value.phrase = nullptr;
}
template <>
void Tree<Phrase>::valuePrint(FILE *file) {
    fprintf(file, "%s" , value.phrase);
}
template <>
void Tree<Phrase>::genDot(Tree<Phrase> *node, FILE *file) {
    fprintf(file, "T%p [shape = record, label = \" {value\\n", node);
    node->valuePrint(file);
    fprintf(file, " | this\\n%p | parent\\n%p  | {left\\n%p | right\\n%p}} \"];\n\t", node, node->getParent(), node->getChild(LEFT_CHILD), node->getChild(RIGHT_CHILD));
    for (int i = 0; i < NUMBER_OF_CHILDREN; ++i) {
        if (!node->childIsEmpty(i))
            fprintf(file, "T%p -> T%p[label = \"%s\"];", node, node->getChild(i), i ? "Нет" : "Да");
    }
    fprintf(file, "\n\t");
}


//Методы акинтаора
char* readNode(char* ptr, Tree<Phrase>* node);

Tree<Phrase>* parse(char** buff);

void printDatabase(Tree<Phrase>* node, FILE* stream);

void play (Tree<Phrase>* akinator, FILE* in, FILE* out);

bool findCollision(Tree<Phrase> *akinator, const char *phrase, FILE *in, FILE* out);

void newAnswer(Tree<Phrase>* mistake, FILE* in, FILE* out);

char *definition(Tree<Phrase> **path, size_t num);

Tree<Phrase>* seek(Tree<Phrase>* akinator, const char* phrase);

Tree<Phrase>** getPath(Tree<Phrase>* node, size_t* num);

void compare(Tree<Phrase>* akinator, FILE* in, FILE* out);

int main() {
    FILE* log = fopen(TREE_LOG_NAME, "wb"); //clearing log file
    fclose(log);

    char* buffer = nullptr;
    Tree<Phrase>* akinator = parse(&buffer);

    compare(akinator, stdin, stdout);

    delete (akinator);
    free(buffer);
    return 0;
}

Tree<Phrase>* parse(char** buff) {
    size_t length = 0;
    char* buffer = read_file_to_buffer_alloc(DATA_BASE_INPUT, "r", &length);
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

void printDatabase(Tree<Phrase>* node, FILE *stream) {
    fprintf(stream, "%c %c", BEGIN_SEPARATOR, *VALUE_SEPARATOR);
    node->valuePrint(stream);
    fprintf(stream, "%c ", *VALUE_SEPARATOR);
    if (node->getChild(LEFT_CHILD) != (Tree<Phrase>*)node->NIL) {
        printDatabase(node->getChild(LEFT_CHILD), stream);
    }
    if (node->getChild(RIGHT_CHILD) != (Tree<Phrase>*)node->NIL) {
        printDatabase(node->getChild(RIGHT_CHILD), stream);
    }
    fprintf(stream, "%c", END_SEPARATOR);
}

void newAnswer (Tree<Phrase>* mistake, FILE* in, FILE* out) {
    fprintf(out, "A kakov pravilniy otvet?\n");
    char correct[ANSWER_SIZE] = "";
    fscanf(in, "%s", correct);

    if (findCollision(mistake, correct, in, out)) {
        return;
    }

    fprintf(out, "Seriously? Chem %s otlichaetsya ot ", correct);
    mistake->valuePrint(out);
    fprintf(out, "\n");
    char question[ANSWER_SIZE] = "";
    fscanf(in, "%s", question);
    size_t question_len = strlen(question);

    char* fit_correct = (char*)calloc(strlen(correct) + 1, sizeof(fit_correct[0]));
    strcpy(fit_correct, correct);
    char* fit_question = (char*)calloc(question_len + 2, sizeof(fit_question[0]));
    strcpy(fit_question, question);
    fit_question[question_len] = '?';

    mistake->growChild(LEFT_CHILD, {fit_correct, true});
    mistake->growChild(RIGHT_CHILD, {mistake->getValue().phrase, mistake->getValue().allocated});
    mistake->setValue({fit_question, true});
    fprintf(out, "OK, ya zapomnu.");
}

void play (Tree<Phrase>* akinator, FILE* in, FILE* out) {
    //Если ответ найден
    if (akinator->childIsEmpty(LEFT_CHILD) && akinator->childIsEmpty(RIGHT_CHILD)) {
        fprintf(out, "Moy otvet:\n ");
        akinator->valuePrint(out);
        fprintf(out, "\nYa ugadal?\n");
        char answer[ANSWER_SIZE] = " ";
        for (int i = 0; i < TRIES; ++i) {
            fscanf(in, ANSWER_FORMAT, answer);
            if (strcmp(answer, YES) == 0) {
                return;
            }
            if (strcmp(answer, NO) == 0) {
                newAnswer(akinator, in, out);
                return;
            }
            fprintf(out, "%d Ne mogu ponyat otvet. Vvedite eshe raz:\n", i);
        }
        //Блок обработки ошибок
        fprintf(out, "Ne hotite po horoshemu, budet po plohomu...\nInitializing Skynet = new Task->Kill_All_Humans\n");
        abort();
    }
    //Если в узле вопрос
    akinator->valuePrint(out);
    fprintf(out, "\n");
    char answer[ANSWER_SIZE] = " ";
    for (int i = 0; i < TRIES; ++i) {
        fscanf(in, "%s", answer);
        if (strcmp(answer, YES) == 0) {
            play(akinator->getChild(LEFT_CHILD), in, out);
            return;
        }
        if (strcmp(answer, NO) == 0) {
            play(akinator->getChild(RIGHT_CHILD), in, out);
            return;
        }
        fprintf(out, "Ne mogu ponyat otvet. Vvedite eshe raz:\n");
    }
    //Блок обработки ошибок
    fprintf(out, "Ne hotite po horoshemu, budet po plohomu...\nInitializing Skynet = new Task->Kill_All_Humans\n");
    abort();
}

bool findCollision(Tree<Phrase> *akinator, const char *phrase, FILE *in, FILE* out) {
    assert(akinator);
    assert(in);
    assert(out);

    Tree<Phrase>* copy = seek(akinator->getRoot(), phrase);
    if (copy != nullptr) {
        size_t size = 0;
        Tree<Phrase>** path = getPath(copy, &size);
        char* def = definition(path, size);
        free(path);
        fprintf(out, "Ne pravda. Ya uzhe znau %s\n%s.", phrase, def);
        free(def);
        return true;
    }
    return false;
}

char *definition(Tree<Phrase> **path, size_t num) {
    assert(path);

    size_t len = 0;

    for (int i = 0; i < num; ++i) {
        len += strlen(path[i]->getValue().phrase) + 4;
    }
    char* output = (char*) calloc(len + 2, sizeof(output[0]));
    assert(output);
    strcpy(output, path[num - 1]->getValue().phrase);
    strcat(output, " eto ");
    for (int i = 0; i < num - 1; ++i) {
        if (path[i]->getChild(RIGHT_CHILD) == path[i + 1]) {
            strcat(output, "ne ");
        }
        path[i]->getValue().phrase[strlen(path[i]->getValue().phrase) - 1] = ' ';
        strcat(output, path[i]->getValue().phrase);
        path[i]->getValue().phrase[strlen(path[i]->getValue().phrase) - 1] = '?';
        strcat(output, " ");
    }
    return output;
}

Tree<Phrase> *seek(Tree<Phrase> *akinator, const char *phrase) {
    assert(akinator);
    assert(phrase);

    Tree<Phrase>** nodes = akinator->allocTree();
    akinator->inorder(nodes);
    for (int i = 0; i < akinator->getSize(); ++i) {
        if (strcmp(phrase, nodes[i]->getValue().phrase) == 0) {
            Tree<Phrase>* ret_val = nodes[i];
            free(nodes);
            return ret_val;
        }
    }
    free(nodes);
    return nullptr;
}

Tree<Phrase>** getPath(Tree<Phrase>* node, size_t* num) {
    assert(node);

    Tree<Phrase>** rpath = (Tree<Phrase>**)calloc( ceil(log(node->getSize()) / log(2)), sizeof(rpath[0]));
    assert(rpath);
    *num = 0;

    while(node != nullptr) {
        rpath[*num] = node;
        *num += 1;
        node = node->getParent();
    }

    Tree<Phrase>** path = (Tree<Phrase>**)calloc( *num, sizeof(path[0]));
    assert(path);
    for (int i = 0; i < *num; ++i) {
        path[i] = rpath[*num - i - 1];
    }
    free(rpath);

    return path;
}

void compare(Tree<Phrase> *akinator, FILE *in, FILE *out) {
    fprintf(out, "Chto mne sravnit: \n");
    char in_first[ANSWER_SIZE] = "";
    char in_second[ANSWER_SIZE] = "";
    fscanf(in, "%s %s", in_first, in_second);
    Tree<Phrase>* first = seek(akinator, in_first);
    if (first == nullptr) {
        fprintf(out, "Ya ne znau %s", in_first);
        return;
    }
    Tree<Phrase>* second = seek(akinator, in_second);
    if (second == nullptr) {
        fprintf(out, "Ya ne znau %s", in_second);
        return;
    }
    size_t size1 = 0, size2 = 0;
    Tree<Phrase>** path1 = getPath(first, &size1);
    Tree<Phrase>** path2 = getPath(second, &size2);

    //общее поддерево
    int index = 0;
    if (path1[1] == path2[1]) {
        fprintf(out, "%s and %s pohozhi tem, chto oni oba", first->getValue().phrase, second->getValue().phrase);
        for (; path1[index + 1] == path2[index + 1]; ++index) {
            if (path1[index]->getChild(RIGHT_CHILD) == path1[index + 1] &&
                path2[index]->getChild(RIGHT_CHILD) == path2[index + 1])
                fprintf(out, " ne");
            fprintf(out, " %s", path1[index]->getValue().phrase);
        }
        fprintf(out, ", no ");
    }
    //личные поддеревья
    fprintf(out, "%s", first->getValue().phrase);
    for (int i = index; i < size1 - 1; ++i) {
        if (path1[i]->getChild(RIGHT_CHILD) == path1[i + 1])
            fprintf(out, " ne");
        path1[i]->getValue().phrase[strlen(path1[i]->getValue().phrase) - 1] = ' ';
        fprintf(out, " %s", path1[i]->getValue().phrase);
        path1[i]->getValue().phrase[strlen(path1[i]->getValue().phrase) - 1] = '?';
    }
    fprintf(out, ", a %s", second->getValue().phrase);
    for (int i = index; i < size2 - 1; ++i) {
        if (path2[i]->getChild(RIGHT_CHILD) == path2[i + 1])
            fprintf(out, " ne");
        path2[i]->getValue().phrase[strlen(path2[i]->getValue().phrase) - 1] = ' ';
        fprintf(out, " %s", path2[i]->getValue().phrase);
        path2[i]->getValue().phrase[strlen(path2[i]->getValue().phrase) - 1] = '?';
    }
    free(path1);
    free(path2);
}
