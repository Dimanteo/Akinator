#include <windows.h>

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

void comparision(Phrase first, Phrase second);

void newAnswer(Tree<Phrase>* mistake, FILE* in, FILE* out);


int main() {
    FILE* log = fopen(TREE_LOG_NAME, "wb"); //clearing log file
    fclose(log);

    char* buffer = nullptr;
    Tree<Phrase>* akinator = parse(&buffer);
    size_t inputSize = akinator->getSize();
    Tree<Phrase>** seq = akinator->allocTree();
    akinator->inorder(seq);
    akinator->graphDump("Graph.png", seq);

    play(akinator, stdin, stdout);
    printf("\n");

    akinator->inorder(seq);
    akinator->graphDump("Graph.png", seq);
    printDatabase(akinator, stdout);
    FILE* data_base = fopen(DATA_BASE_OUTPUT, "wb");
    printDatabase(akinator, data_base);
    fclose(data_base);

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