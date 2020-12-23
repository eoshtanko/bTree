#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>

using namespace std;

struct keyValuePairs {
public:
    keyValuePairs() = default;
    keyValuePairs(int key, int value) {
        this->key = key;
        this->value = value;
    }
    // Аксессоры показались мне здесь неуместными, так как мы считываем/устанавливаем
    // ключ и значение без всякой проверки
    int key;
    int value;
};

class Node {
    int n;// кол-во ключей
    bool isLeaf; // является ли листом
    keyValuePairs *items; // хранимые элементы(ключ-значение)
    int t;// минимальная степень ветвления
    Node **C;// дети

public:
    Node(int t, bool isLeaf) {
        // кол-во ключей
        n = 0;
        this->t = t;
        this->isLeaf = isLeaf;
        // макс - 2t - 1 ключей
        items = new keyValuePairs[2 * t - 1];
        // макс - 2t детей
        C = new Node *[2 * t];
    }

    /// Поиск элемента
    std::string search(int k) {
        int i = 0;
        // находим первый ключ, который больше или равен k
        while (i < n && k > items[i].key) {
            i++;
        }
        if (items[i].key == k && i < n) {
            return to_string(items[i].value);
        }
        if (isLeaf) {
            return "null";
        }
        return C[i]->search(k);
    }

    /// Вспоспомогательный метод вставки
    void insertNonFull(keyValuePairs k) {
        int i = n - 1;
        if (isLeaf) {
            // поиск места для вставки и сдвиг элементов
            while (i >= 0 && items[i].key > k.key) {
                items[i + 1].key = items[i].key;
                items[i + 1].value = items[i].value;
                i--;
            }
            // вставка элемента
            items[i + 1].key = k.key;
            items[i + 1].value = k.value;
            n = n + 1;
        } else {
            while (i >= 0 && items[i].key > k.key) {
                i--;
            }
            // 2 * t - 1 - кол-во элементов в узле, при котором мы считаем его "заполненным"
            if (C[i + 1]->n == 2 * t - 1) {
                splitChild(i + 1, C[i + 1]);
                if (k.key > items[i + 1].key) {
                    i++;
                }
            }
            C[i + 1]->insertNonFull(k);
        }
    }

    /// Разбиение узла-ребенка nodeToSplit с индексом i в массиве детей
    void splitChild(int i, Node *nodeToSplit) {
        // Создание нового узла с t - 1 элементами
        Node *newNode = new Node(nodeToSplit->t, nodeToSplit->isLeaf);
        newNode->n = t - 1;
        // Помещаем в новый узел последние t-1 элементов переданного
        for (int j = 0; j < t - 1; j++) {
            newNode->items[j].key = nodeToSplit->items[j + t].key;
            newNode->items[j].value = nodeToSplit->items[j + t].value;
        }
        // Помещаем в новый узел t последних "детей" переданного
        if (!nodeToSplit->isLeaf) {
            for (int j = 0; j < t; j++) {
                newNode->C[j] = nodeToSplit->C[j + t];
            }
        }
        nodeToSplit->n = t - 1;
        for (int j = n; j >= i + 1; j--) {
            C[j + 1] = C[j];
        }
        C[i + 1] = newNode;
        for (int j = n - 1; j >= i; j--) {
            items[j + 1].key = items[j].key;
            items[j + 1].value = items[j].value;
        }
        items[i].key = nodeToSplit->items[t - 1].key;
        items[i].value = nodeToSplit->items[t - 1].value;
        n = n + 1;
    }

    /// Удаление элемента
    void del(keyValuePairs k) {
        int index = findKeyIndex(k);
        // Если ключ находится в данном узле
        if (index < n && items[index].key == k.key) {
            if (isLeaf) {
                for (int j = index + 1; j < n; ++j) {
                    items[j - 1].value = items[j].value;
                    items[j - 1].key = items[j].key;
                }
                n--;
            } else {
                removeFromNonLeaf(index);
            }
        } else {
            if (isLeaf) {
                return;
            }
            bool isInTheSubtree = index == n;
            // Если в ребенке, где мы ищем элемент, меншьше t элементов - заполняем его
            if (C[index]->n < t) {
                fillNode(index);
            }
            if (isInTheSubtree && index > n) {
                C[index - 1]->del(k);
            } else {
                C[index]->del(k);
            }
        }
    }

    /// Возвращает индекс первого ключа большего или равного k
    int findKeyIndex(keyValuePairs k) {
        int i = 0;
        while (i < n && k.key > items[i].key) {
            i++;
        }
        return i;
    }

    /// Вспоспомогательный метод удаления
    // находим предшественника k в поддереве и замещаем им k. Удоляем предшественника.
    void replaceByPred(int index){
        Node *node = C[index];
        while (!node->isLeaf) {
            node = node->C[node->n];
        }
        items[index].key = node->items[node->n - 1].key;
        items[index].value = node->items[node->n - 1].value;
        // удаляем предшественника
        C[index]->del(node->items[node->n - 1]);
    }

    /// Вспоспомогательный метод удаления
    // находим преемника k в поддереве и замещаем им k. Удоляем преемника.
    void replaceBySuc(int index){
        Node *node = C[index + 1];
        while (!node->isLeaf) {
            node = node->C[0];
        }
        items[index].value = (node->items[0]).value;
        items[index].key = (node->items[0]).key;
        C[index + 1]->del(node->items[0]);
    }

    /// Удаление элемента в случае если узел не является "листом"(у него есть дети)
    void removeFromNonLeaf(int index) {
        keyValuePairs el = items[index];
        // находим предшественника k в поддереве и замещаем им k. Удоляем предшественника.
        if (C[index]->n >= t) {
            replaceByPred(index);
        } else if (C[index + 1]->n >= t) {
            // находим преемника k в поддереве и замещаем им k. Удоляем преемника.
            replaceBySuc(index);
        } else {
            merge(index);
            C[index]->del(el);
        }
    }

    /// Сдвигает элементы массива на одно место назад
    static void moveBackwards(Node *node){
        for (int j = 1; j < node->n; ++j) {
            node->items[j - 1] = node->items[j];
        }
        if (!node->isLeaf) {
            for (int j = 1; j <= node->n; ++j) {
                node->C[j - 1] = node->C[j];
            }
        }
    }

    /// Сдвигает элементы массива на одно место вперед
    static void moveForward(Node *node){
        for (int j = node->n - 1; j >= 0; --j) {
            node->items[j + 1] = node->items[j];
        }
        if (!node->isLeaf) {
            for (int j = node->n; j >= 0; --j) {
                node->C[j + 1] = node->C[j];
            }
        }
    }

    /// Заполняет узел-ребенок
    void fillNode(int i) {
        // Если у ребенка после C[i] более t - 1 элементов - забираем элемент у него!
        if (i != n && C[i + 1]->n >= t) {
            Node *child1 = C[i];
            Node *child2 = C[i + 1];

            child1->items[(child1->n)] = items[i];
            // первый ребенок child2 -> последним ребенком child1
            if (!(child1->isLeaf)) {
                child1->C[(child1->n) + 1] = child2->C[0];
            }
            items[i] = child2->items[0];
            // сдвигаем назад все элементы в child2
            moveBackwards(child2);
            child1->n += 1;
            child2->n -= 1;
        }
            // Если у ребенка до C[i] более t - 1 элементов - забираем элемент у него!
        else if (i != 0 && t <= C[i - 1]->n) {
            Node *child1 = C[i];
            Node *child2 = C[i - 1];
            // сдвигаем элементы в child1 на один вперед
            moveForward(child1);
            child1->items[0] = items[i - 1];
            // делаем первым ребенком child1 последнего child2
            if (!child1->isLeaf) {
                child1->C[0] = child2->C[child2->n];
            }
            items[i - 1] = child2->items[child2->n - 1];
            child1->n += 1;
            child2->n -= 1;
        } else {
            if (i == n)
                merge(i - 1);
            else
                merge(i);
        }
    }

    /// Копирует элементы и указатели на детей из одного узла в другой
    void copingItems(Node *nodeTo, Node *nodeFrom) const{
        for (int j = 0; j < nodeFrom->n; ++j) {
            nodeTo->items[j + t] = nodeFrom->items[j];
        }
        if (!nodeTo->isLeaf) {
            for (int j = 0; j <= nodeFrom->n; ++j)
                nodeTo->C[j + t] = nodeFrom->C[j];
        }
    }

    /// "Сливает" детей с индексом i и i + 1 и удаляет i + 1
    void merge(int i) {
        Node *child1 = C[i];
        Node *child2 = C[i + 1];
        child1->items[t - 1] = items[i];
        copingItems(child1, child2);
        for (int j = i + 1; j < n; ++j) {
            items[j - 1].value = items[j].value;
            items[j - 1].key = items[j].key;
        }
        for (int j = i + 2; j <= n; ++j) {
            C[j - 1] = C[j];
        }
        child1->n += child2->n + 1;
        n--;
        delete (child2);
    }

    friend class BTree;
};

class BTree {
    int t;
    Node *root;
public:
    explicit BTree(int t) {
        this->t = t;
        root = nullptr;
    }

    /// Поиск ключа в данном дереве
    std::string search(int k) {
        if (root == nullptr) {
            return "null";
        }
        return root->search(k);
    }

    /// Вставка в B-tree
    std::string insertToBTree(int key, int value) {
        keyValuePairs el = keyValuePairs(key, value);
        if (root == nullptr) {
            root = new Node(t, true);
            root->items[0].key = key;
            root->items[0].value = value;
            root->n = 1;
        } else {
            if (search(el.key) != "null") {
                return "false";
            }
            // 2 * t - 1 - кол-во элементов в узле, при котором мы считаем его "заполненным"
            // Когда узел заполнен - разбиваем его на два узла с t - 1 ключами
            if (root->n == 2 * t - 1) {
                Node *node = new Node(t, false);
                // помещаем заполненный узел в "детей" нового узла
                node->C[0] = root;
                node->splitChild(0, root);
                // Выбор, в какого "ребенка" поместить элемент
                int i = 0;
                if (node->items[0].key < el.key) {
                    i++;
                }
                node->C[i]->insertNonFull(el);
                root = node;
            } else {
                root->insertNonFull(el);
            }
        }
        return "true";
    }

    /// Удаление из B-tree
    void deleteFromBTree(int key) {
        if (!root) {
            return;
        }
        keyValuePairs el = keyValuePairs(key, 0);
        root->del(el);
        if (root->n == 0) {
            if (root->isLeaf) {
                root = nullptr;
            } else {
                root = root->C[0];
            }
        }
    }
};

/// Проверяет, существует ли файл
static bool fileExist(const string &name) {
    std::ifstream f(name.c_str());
    return f.good();
}

/// Позволяет работать как с относительными, так и с абсолютными путями
static string path(string pathStr) {
    if (fileExist(pathStr))
        return pathStr;
    if (fileExist("input/" + pathStr))
        return "input/" + pathStr;
    if (fileExist("output/" + pathStr))
        return "output/" + pathStr;
    throw invalid_argument("Такого файла не существуеть!");
}

/// Вспомогательный метод к методу split
static bool space(char c) {
    return isspace(c);
}

/// Вспомогательный метод к методу split
static bool notSpace(char c) {
    return !isspace(c);
}

/// Метод, иммитирующий split в c#. Разделяет строку на слова и помещает их в вектор.
static vector<string> split(const string &s) {
    typedef string::const_iterator iter;
    vector<string> ret;
    iter i = s.begin();

    while (i != s.end()) {

        i = find_if(i, s.end(), notSpace);
        iter j = find_if(i, s.end(), space);

        if (i != s.end()) {
            ret.emplace_back(i, j);
            i = j;
        }

    }
    return ret;
}

/// Обработка команд
static void commandProcessing(BTree &tree, vector<std::string> &output, std::string pathStr) {
    std::ifstream fileInput(path(std::move(pathStr)));
    // Строка для чтения строк из файла
    std::string s;
    while (getline(fileInput, s)) {
        // Строка команды, например "insert 1 2",
        // представленная в виде коллекции слов
        std::vector<std::string> input = split(s);
        if (input[0] == "insert") {
            output.push_back(tree.insertToBTree(std::atoi(input[1].c_str()), std::atoi(input[2].c_str())));
        } else if (input[0] == "delete") {
            string exist = tree.search(std::atoi(input[1].c_str()));
            if (exist == "null") {
                output.push_back("null");
            } else {
                output.push_back(exist);
                tree.deleteFromBTree(std::atoi(input[1].c_str()));
            }
        } else if (input[0] == "find") {
            output.push_back(tree.search(std::atoi(input[1].c_str())));
        }
    }
    fileInput.close();
}

/// Вывод выходных данных в файл
static void outputToFile(const std::vector<std::string> &output, std::string pathStr) {
    std::ofstream fileOutput;
    fileOutput.open(path(std::move(pathStr)));
    if (fileOutput.is_open()) {
        for (int i = 0; i < output.size(); ++i) {
            if (i != output.size() - 1) {
                fileOutput << output[i] << std::endl;
            } else {
                fileOutput << output[i];
            }
        }
    }
}

int main(int argc, char *argv[]) {
    std::vector<std::string> output;
    BTree tree(std::atoi(argv[1]));
    commandProcessing(tree, output, argv[2]);
    outputToFile(output, argv[3]);
    return 0;
}