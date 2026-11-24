#include <chrono>
#include <iomanip>
#include <iostream>
#include <limits>
#include <queue>
#include <random>
#include <stdexcept>
#include <vector>

using namespace std;

using Matrix = vector<vector<int>>;
using AdjacencyList = vector<vector<int>>;

class ListQueue {
private:
    struct Node {
        int value;
        Node* next;
    };

    Node* head = nullptr;
    Node* tail = nullptr;

public:
    // Конструктор очереди на списке
    ListQueue() = default;

    // Деструктор очереди на списке
    ~ListQueue() {
        while (!empty()) {
            pop();
        }
    }

    // Проверка очереди на пустоту
    bool empty() const {
        return head == nullptr;
    }

    // Добавление элемента в очередь
    void push(int value) {
        Node* node = new Node{value, nullptr};
        if (tail == nullptr) {
            head = tail = node;
        } else {
            tail->next = node;
            tail = node;
        }
    }

    // Извлечение элемента из очереди
    int pop() {
        if (empty()) {
            throw out_of_range("Очередь пуста");
        }
        Node* node = head;
        int value = node->value;
        head = head->next;
        if (head == nullptr) {
            tail = nullptr;
        }
        delete node;
        return value;
    }
};

// Генератор матрицы смежности
Matrix generateMatrix(int size, double probability, mt19937& generator) {
    Matrix graph(size, vector<int>(size, 0));
    bernoulli_distribution edge(probability);
    for (int i = 0; i < size; ++i) {
        for (int j = i + 1; j < size; ++j) {
            if (edge(generator)) {
                graph[i][j] = graph[j][i] = 1;
            }
        }
    }
    return graph;
}

// Вывод матрицы смежности
void printMatrix(const Matrix& graph) {
    for (const auto& row : graph) {
        for (int value : row) {
            cout << value << ' ';
        }
        cout << '\n';
    }
}

// Преобразование матрицы в списки смежности
AdjacencyList makeAdjacencyList(const Matrix& graph) {
    AdjacencyList list(graph.size());
    for (int i = 0; i < static_cast<int>(graph.size()); ++i) {
        for (int j = 0; j < static_cast<int>(graph.size()); ++j) {
            if (graph[i][j] != 0) {
                list[i].push_back(j);
            }
        }
    }
    return list;
}

// Обход матрицы в ширину стандартной очередью
vector<int> bfsMatrix(const Matrix& graph) {
    const int size = static_cast<int>(graph.size());
    vector<bool> visited(size, false);
    vector<int> order;
    queue<int> queue;

    for (int start = 0; start < size; ++start) {
        if (visited[start]) {
            continue;
        }
        visited[start] = true;
        queue.push(start);
        while (!queue.empty()) {
            int vertex = queue.front();
            queue.pop();
            order.push_back(vertex);
            for (int neighbor = 0; neighbor < size; ++neighbor) {
                if (graph[vertex][neighbor] != 0 && !visited[neighbor]) {
                    visited[neighbor] = true;
                    queue.push(neighbor);
                }
            }
        }
    }
    return order;
}

// Обход списков смежности в ширину
vector<int> bfsAdjacencyList(const AdjacencyList& graph) {
    const int size = static_cast<int>(graph.size());
    vector<bool> visited(size, false);
    vector<int> order;
    queue<int> queue;

    for (int start = 0; start < size; ++start) {
        if (visited[start]) {
            continue;
        }
        visited[start] = true;
        queue.push(start);
        while (!queue.empty()) {
            int vertex = queue.front();
            queue.pop();
            order.push_back(vertex);
            for (int neighbor : graph[vertex]) {
                if (!visited[neighbor]) {
                    visited[neighbor] = true;
                    queue.push(neighbor);
                }
            }
        }
    }
    return order;
}

// Обход матрицы очередью на связном списке
vector<int> bfsMatrixListQueue(const Matrix& graph) {
    const int size = static_cast<int>(graph.size());
    vector<bool> visited(size, false);
    vector<int> order;
    ListQueue queue;

    for (int start = 0; start < size; ++start) {
        if (visited[start]) {
            continue;
        }
        visited[start] = true;
        queue.push(start);
        while (!queue.empty()) {
            int vertex = queue.pop();
            order.push_back(vertex);
            for (int neighbor = 0; neighbor < size; ++neighbor) {
                if (graph[vertex][neighbor] != 0 && !visited[neighbor]) {
                    visited[neighbor] = true;
                    queue.push(neighbor);
                }
            }
        }
    }
    return order;
}

// Измерение среднего времени обхода матрицы
double measureTime(const Matrix& graph, bool useListQueue, int repetitions) {
    volatile size_t result = 0;
    const auto begin = chrono::steady_clock::now();
    for (int i = 0; i < repetitions; ++i) {
        if (useListQueue) {
            result += bfsMatrixListQueue(graph).size();
        } else {
            result += bfsMatrix(graph).size();
        }
    }
    const auto end = chrono::steady_clock::now();
    (void)result;
    return chrono::duration<double, milli>(end - begin).count() / repetitions;
}

// Сравнение времени работы очередей
void benchmark(mt19937& generator) {
    const vector<int> sizes{100, 300, 600};
    cout << "\nСравнение обходов, среднее время в миллисекундах\n";
    cout << "Вершины | queue | очередь-список\n";
    for (int size : sizes) {
        Matrix graph = generateMatrix(size, 0.03, generator);
        int repetitions = size <= 100 ? 100 : (size <= 300 ? 30 : 10);
        double standard = measureTime(graph, false, repetitions);
        double custom = measureTime(graph, true, repetitions);
        cout << setw(7) << size << " | "
                  << fixed << setprecision(4) << setw(10) << standard
                  << " | " << setw(14) << custom << '\n';
    }
}

// Запуск программы
int main() {
    mt19937 generator(random_device{}());
    int size;
    double probability;
    int start;

    while (true) {
        cout << "Введите количество вершин: ";
        if (cin >> size && size > 0) {
            break;
        }
        cout << "Количество вершин должно быть положительным целым числом.\n";
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }
    while (true) {
        cout << "Введите вероятность ребра от 0 до 1: ";
        if (cin >> probability && probability >= 0.0 && probability <= 1.0) {
            break;
        }
        cout << "Вероятность должна быть числом в диапазоне [0, 1].\n";
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }
    while (true) {
        cout << "Введите начальную вершину от 1 до " << size << ": ";
        if (cin >> start && start >= 1 && start <= size) {
            break;
        }
        cout << "Некорректный номер вершины.\n";
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }

    Matrix graph = generateMatrix(size, probability, generator);
    cout << "\nМатрица смежности:\n";
    printMatrix(graph);

    vector<int> matrixOrder = bfsMatrix(graph);
    cout << "\nОбход матрицы (все компоненты): ";
    for (int vertex : matrixOrder) {
        cout << vertex + 1 << ' ';
    }
    cout << "\nОбход от вершины " << start << ": ";
    vector<int> fromStart;
    vector<bool> visited(size, false);
    queue<int> queue;
    int source = start - 1;
    visited[source] = true;
    queue.push(source);
    while (!queue.empty()) {
        int vertex = queue.front();
        queue.pop();
        fromStart.push_back(vertex);
        for (int neighbor = 0; neighbor < size; ++neighbor) {
            if (graph[vertex][neighbor] != 0 && !visited[neighbor]) {
                visited[neighbor] = true;
                queue.push(neighbor);
            }
        }
    }
    for (int vertex : fromStart) {
        cout << vertex + 1 << ' ';
    }
    cout << "\nОбход списков смежности: ";
    for (int vertex : bfsAdjacencyList(makeAdjacencyList(graph))) {
        cout << vertex + 1 << ' ';
    }
    cout << '\n';

    benchmark(generator);
    return 0;
}
