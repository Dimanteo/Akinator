#include <cmath>
#include "Tree_t/Tree.cpp"
#include "txt_files.h"



//Переменные формата
#define MIPT_FORMAT " \"%[^\"]\" "
const char BEGIN_SEPARATOR = '{';
const char END_SEPARATOR = '}';
const char VALUE_SEPARATOR[] = "\"";
const char YES[] = "Y";
const char NO[] = "N";
const char COMMANDS_LIST[] = "-play : отгадывать\n"
                           "-diff <name1> <name2> : сравнить name1 с name2\n"
                           "-what <name> : дать определение name\n"
                           "-graph <file> : вывести граф дерева в файл\n"
                           "-show : вывести дерево в консоль\n"
                           "-save : сохранить дерево\n"
                           "-q : выход\n"
                           "-h : помощь\n";

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

//Имена файлов с базой данных
const char DATA_BASE_INPUT[] = "DBase.txt";


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


//Методы акинатора
char* readNode(char* ptr, Tree<Phrase>* node);

Tree<Phrase>* parse(char** buff);

void printDatabase(Tree<Phrase>* node, FILE* stream);

bool play (Tree<Phrase>* akinator, FILE* in, FILE* out);

bool findCollision(Tree<Phrase> *akinator, const char *phrase, FILE* out);

bool newAnswer(Tree<Phrase>* mistake, FILE* in, FILE* out);

char *definition(Tree<Phrase> **path, size_t num);

Tree<Phrase>* seek(Tree<Phrase>* akinator, const char* phrase);

Tree<Phrase>** getPath(Tree<Phrase>* node, size_t* num);

void compare(Tree<Phrase>* akinator, FILE* in, FILE* out);

bool handlerYesNo(FILE* in, FILE* out);

void start(Tree<Phrase>* akinator, FILE* in, FILE* out);

void save(Tree<Phrase>* akinator);

int main() {
    char* buffer = nullptr;
    Tree<Phrase>* akinator = parse(&buffer);

    start(akinator, stdin, stdout);

    delete (akinator);
    free(buffer);
    return 0;
}

bool handlerYesNo(FILE* in, FILE* out) {
    fprintf(out, "Y - да, N - нет\n");
    char answer[ANSWER_SIZE] = " ";
    for (int i = 0; i < TRIES; ++i) {
        fscanf(in, "%s", answer);
        if (strcmp(answer, YES) == 0) {
            return true;
        }
        if (strcmp(answer, NO) == 0) {
            return false;
        }
        fprintf(out, "Не понимаю ваш ответ. Введите еще раз:\n");
    }
    //Блок обработки ошибок
    fprintf(out, "Не хотите по хорошему, будет по плохому...\nInitializing Skynet = new Task->Kill_All_Humans\n");
    abort();
}

void start(Tree<Phrase>* akinator, FILE* in, FILE* out) {
    fprintf(out, "Привет, я Акинатор.\n %s", COMMANDS_LIST);
    bool rak_na_gore_ne_svistnet = true, DBchanged = false;
    while (rak_na_gore_ne_svistnet) {
        char token[3] = "";
        int read = fscanf(in, "%s", token);
        if (read == 0) {
            fprintf(out, "Неправильная команда. Введите -h чтобы увидеть список команд\n");
            continue;
        }
        if (strcmp(token, "-h") == 0) {
            fprintf(out, "%s", COMMANDS_LIST);
            continue;
        }
        if (strcmp(token, "-play") == 0) {
            DBchanged = play(akinator, in, out);
            continue;
        }
        if (strcmp(token, "-q") == 0) {
            if (DBchanged) {
                fprintf(out, "База данных изменилась. Сохранить новые ответы?\n");
                if (handlerYesNo(in, out)){
                    save(akinator);
                }
            }
            break;
        }
        if (strcmp(token, "-graph") == 0) {
            char arg[ANSWER_SIZE] = "";
            fscanf(in, "%s", arg);
            Tree<Phrase>** seq = akinator->allocTree();
            akinator->inorder(seq);
            akinator->graphDump(arg, seq);
            free(seq);
            continue;
        }
        if (strcmp(token, "-what") == 0) {
            char arg[ANSWER_SIZE] = "";
            fscanf(in, "%s", arg);
            Tree<Phrase>* node = seek(akinator, arg);
            if (node != nullptr) {
                size_t size = 0;
                Tree<Phrase>** path = getPath(node, &size);
                char* answer = definition(path, size);
                fprintf(out, "%s\n", answer);
                free(answer);
                continue;
            } else {
                fprintf(out, "Я не знаю %s\n", arg);
                continue;
            }
        }
        if (strcmp(token, "-show") == 0) {
            printDatabase(akinator, stdout);
            printf("\n");
            continue;
        }
        if (strcmp(token, "-diff") == 0) {
            compare(akinator, in, out);
            fprintf(out, "\n");
            continue;
        }
        if (strcmp(token, "-save") == 0) {
            save(akinator);
            continue;
        }
        fprintf(out, "Неправильная команда. Введите -h чтобы увидеть список команд\n");
    }
}

Tree<Phrase>* parse(char** buff) {
    size_t length = 0;
    char* buffer = read_file_to_buffer_alloc(DATA_BASE_INPUT, "r", &length);
    Tree<Phrase>* node = new Tree<Phrase>({nullptr});
    readNode(buffer, node);
    *buff = buffer;
    return node;
}

char *readNode(char *ptr, Tree<Phrase> *node) { //format { "quest?" { "YES" }{ "NO" }}
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

bool newAnswer (Tree<Phrase>* mistake, FILE* in, FILE* out) {
    fprintf(out, "А каков правильный ответ?\n");
    char correct[ANSWER_SIZE] = "";
    fflush(in);
    fscanf(in, " ");
    fgets(correct, ANSWER_SIZE, in);
    size_t correct_len = strlen(correct);
    correct[correct_len - 1] = '\0';

    if (findCollision(mistake, correct, out)) {
        return false;
    }

    fprintf(out, "Что правда? Чем %s отличается от ", correct);
    mistake->valuePrint(out);
    fprintf(out, "\n");
    char question[ANSWER_SIZE] = "";
    fflush(in);
    fscanf(in, " ");
    fgets(question, ANSWER_SIZE, in);
    size_t question_len = strlen(question);

    char* fit_correct = (char*)calloc(correct_len, sizeof(fit_correct[0]));
    strcpy(fit_correct, correct);
    char* fit_question = (char*)calloc(question_len + 1, sizeof(fit_question[0]));
    strcpy(fit_question, question);
    fit_question[question_len - 1] = '?';

    mistake->growChild(LEFT_CHILD, {fit_correct, true});
    mistake->growChild(RIGHT_CHILD, {mistake->getValue().phrase, mistake->getValue().allocated});
    mistake->setValue({fit_question, true});
    fprintf(out, "OK, я запомню.\n");
    return true;
}

bool play (Tree<Phrase>* akinator, FILE* in, FILE* out) {
    //Если ответ найден
    if (akinator->childIsEmpty(LEFT_CHILD) && akinator->childIsEmpty(RIGHT_CHILD)) {
        fprintf(out, "Мой ответ:\n ");
        akinator->valuePrint(out);
        fprintf(out, "\nЯ угадал?\n");
        if (handlerYesNo(in, out)) {
            return false;
        } else {
            return newAnswer(akinator, in, out);
        }
    }
    //Если в узле вопрос
    akinator->valuePrint(out);
    fprintf(out, "\n");
    if (handlerYesNo(in, out)) {
        return play(akinator->getChild(LEFT_CHILD), in, out);
    } else {
        return play(akinator->getChild(RIGHT_CHILD), in, out);
    }
}

bool findCollision(Tree<Phrase> *akinator, const char *phrase, FILE* out) {
    assert(akinator);
    assert(out);

    Tree<Phrase>* copy = seek(akinator->getRoot(), phrase);
    if (copy != nullptr) {
        size_t size = 0;
        Tree<Phrase>** path = getPath(copy, &size);
        char* def = definition(path, size);
        free(path);
        fprintf(out, "Не правда. Я уже знаю %s\n%s.", phrase, def);
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
    strcat(output, " это ");
    for (int i = 0; i < num - 1; ++i) {
        if (path[i]->getChild(RIGHT_CHILD) == path[i + 1]) {
            strcat(output, "не ");
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
    char in_first[ANSWER_SIZE] = "";
    char in_second[ANSWER_SIZE] = "";
    fscanf(in, "%s %s", in_first, in_second);
    Tree<Phrase>* first = seek(akinator, in_first);
    if (first == nullptr) {
        fprintf(out, "Я не знаю %s", in_first);
        return;
    }
    Tree<Phrase>* second = seek(akinator, in_second);
    if (second == nullptr) {
        fprintf(out, "Я не знаю %s", in_second);
        return;
    }
    size_t size1 = 0, size2 = 0;
    Tree<Phrase>** path1 = getPath(first, &size1);
    Tree<Phrase>** path2 = getPath(second, &size2);

    //общее поддерево
    int index = 0;
    if (path1[1] == path2[1]) {
        fprintf(out, "%s и %s похожи тем, что они оба", first->getValue().phrase, second->getValue().phrase);
        for (; path1[index + 1] == path2[index + 1]; ++index) {
            if (path1[index]->getChild(RIGHT_CHILD) == path1[index + 1] &&
                path2[index]->getChild(RIGHT_CHILD) == path2[index + 1])
                fprintf(out, " не");
            fprintf(out, " %s", path1[index]->getValue().phrase);
        }
        fprintf(out, ", но ");
    }
    //личные поддеревья
    fprintf(out, "%s", first->getValue().phrase);
    for (int i = index; i < size1 - 1; ++i) {
        if (path1[i]->getChild(RIGHT_CHILD) == path1[i + 1])
            fprintf(out, " не");
        path1[i]->getValue().phrase[strlen(path1[i]->getValue().phrase) - 1] = ' ';
        fprintf(out, " %s", path1[i]->getValue().phrase);
        path1[i]->getValue().phrase[strlen(path1[i]->getValue().phrase) - 1] = '?';
    }
    fprintf(out, ", а %s", second->getValue().phrase);
    for (int i = index; i < size2 - 1; ++i) {
        if (path2[i]->getChild(RIGHT_CHILD) == path2[i + 1])
            fprintf(out, " не");
        path2[i]->getValue().phrase[strlen(path2[i]->getValue().phrase) - 1] = ' ';
        fprintf(out, " %s", path2[i]->getValue().phrase);
        path2[i]->getValue().phrase[strlen(path2[i]->getValue().phrase) - 1] = '?';
    }
    free(path1);
    free(path2);
}

void save(Tree<Phrase> *akinator) {
    FILE* file = fopen(DATA_BASE_INPUT, "wb");
    printDatabase(akinator, file);
    fclose(file);
}
