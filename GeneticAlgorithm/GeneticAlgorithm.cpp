#include <iostream>
#include <vector>
#include <bitset>
#include <algorithm>
#include <random>
#include <string>

using namespace std;

const int NUM_VERTICES = 8;

class WrongIValueException : exception
{
private:
    int size;
    string str;
public:
    WrongIValueException(string s, int value) { str = s; size = value; }
    void print() const
    {
        cout << "WrongIValue: " << size << "; " << what();
    }
};

class GeneticAlgorithm {
private:
    int populationSize_;
    int generations_;
    vector<bitset<NUM_VERTICES>> population_;
    vector<pair<bitset<NUM_VERTICES>, int>> fitnessScores_;
    vector<bitset<NUM_VERTICES>> parents_;
    double mutatePercent_;
    double crit_;
    vector<vector<int>> adjacencyMatrix_;
public:
    GeneticAlgorithm(const vector<vector<int>>& adjacencyMatrix, int populationSize, int generations, double mutatePercent = 0.05, double crit = 10)
        : adjacencyMatrix_(adjacencyMatrix), populationSize_(populationSize), generations_(generations), mutatePercent_(mutatePercent), crit_(crit) {
        if (populationSize <= 0) {
            throw WrongIValueException("\nWrong value of populationSize in constructor of GA", populationSize);
        }
        if (generations <= 0)
        {
            throw WrongIValueException("\nWrong value of generations in constructor of GA", generations);
        }
    }

    void setMutatePercent(double mt)
    {
        if (mt < 0 || mt > 1)
        {
            throw WrongIValueException("\nWrong value of mutatePercent in setter of GA", mt);
        }
        mutatePercent_ = mt;
    }

    void setCrit(double value)
    {
        crit_ = value;
    }

    int summaryFitness() const
    {
        int res{ 0 };
        for (int i = 0; i < fitnessScores_.size(); ++i)
        {
            res += fitnessScores_[i].second;;
        }
        return res;
    }


    //мутация особи инвертацией битов
    void mutate(bitset<NUM_VERTICES>& clique) const {
        static random_device rd;
        static mt19937 gen(rd());
        static uniform_real_distribution<> dis(0.0, 1.0);

        for (int i = 0; i < NUM_VERTICES; ++i) {
            if (dis(gen) < mutatePercent_) {
                clique.flip(i);
            }
        }
    }

    //одноточечное скрещивание двух особей
    bitset<NUM_VERTICES> crossover(const bitset<NUM_VERTICES>& parent1, const bitset<NUM_VERTICES>& parent2) const {
        bitset<NUM_VERTICES> child;
        static random_device rd;
        static mt19937 gen(rd());
        static uniform_int_distribution<> dis(0, NUM_VERTICES - 1);

        int crossoverPoint = dis(gen);

        for (int i = 0; i < NUM_VERTICES; ++i) {
            if (i <= crossoverPoint) {
                child[i] = parent1[i];
            }
            else {
                child[i] = parent2[i];
            }
        }

        return child;
    }

    int evaluateFitness(const bitset<NUM_VERTICES>& clique) const {
        for (int i = 0; i < NUM_VERTICES; ++i) {
            if (clique[i]) {
                for (int j = i + 1; j < NUM_VERTICES; ++j) {
                    if (clique[j] && adjacencyMatrix_[i][j] == 0) {
                        return 0;  // найдено ребро между двумя вершинами в клике, которых не должно быть
                    }
                }
            }
        }
        return clique.count();  // вернуть размер клики
    }


    //нахождение максимальной клики
    bitset<NUM_VERTICES> maxClique() {

        // Инициализация начальной популяции случайными значениями
        for (int i = 0; i < populationSize_; ++i) {
            bitset<NUM_VERTICES> individual;
            for (int j = 0; j < NUM_VERTICES; ++j) {
                individual[j] = rand() % 2;
            }
            population_.push_back(individual);
        }

        int currK = 0;

        // Основной цикл генетического алгоритма
        for (int gen = 0; gen < generations_; ++gen) {
            // Вычисление приспособленности для каждой особи
            for (const auto& individual : population_) {
                int fitness = evaluateFitness(individual);
                fitnessScores_.push_back(make_pair(individual, fitness));
            }

            // Сортировка по убыванию приспособленности
            sort(fitnessScores_.begin(), fitnessScores_.end(),
                [](const pair<bitset<NUM_VERTICES>, int>& a, const pair<bitset<NUM_VERTICES>, int>& b)
                { return a.second > b.second; });

            int tmp = summaryFitness();
            if (abs(tmp - currK) < crit_) break;
            currK = tmp;

            // Отбор родителей для скрещивания
            for (int i = 0; i < populationSize_ / 2; ++i) {
                parents_.push_back(fitnessScores_[i].first);
            }

            // Генерация новой популяции через скрещивание и мутацию
            population_.clear();
            for (int i = 0; i < parents_.size(); ++i) {
                for (int j = i + 1; j < parents_.size(); ++j) {
                    bitset<NUM_VERTICES> child = crossover(parents_[i], parents_[j]);
                    mutate(child);
                    population_.push_back(child);
                }

            }
            parents_.clear();
            fitnessScores_.clear();
        }

        // Вычисление приспособленности для каждой особи в конечной популяции
        for (const auto& individual : population_) {
            int fitness = evaluateFitness(individual);
            fitnessScores_.push_back(make_pair(individual, fitness));
        }

        // Сортировка по убыванию приспособленности
        sort(fitnessScores_.begin(), fitnessScores_.end(),
            [](const pair<bitset<NUM_VERTICES>, int>& a, const pair<bitset<NUM_VERTICES>, int>& b)
            { return a.second > b.second; });
        return fitnessScores_[0].first;
    }
};

//функция для вывода матрицы на экран
void printMatrix(const vector<vector<int>>& mat)
{
    cout << "\n |";
    for (int i = 0; i < NUM_VERTICES; ++i)
        cout << i + 1 << " ";
    cout << "\n_________________\n";
    for (int i = 0; i < NUM_VERTICES; ++i)
    {
        cout << i + 1 << "|";
        for (int j = 0; j < NUM_VERTICES; ++j)
            cout << mat[i][j] << " ";
        cout << "\n";
    }
}

int main() {
    char ans;
    try
    {
        srand(time(NULL));
        cout << "\nDEMONSTRATION OF GENETIC ALGORYTHM BY EXAMPLE: \nsolving the problem of finding the maximum clique in a graph by the adjacency matrix \n";
        vector<vector<int>> adjacencyMatrix(NUM_VERTICES, vector<int>(NUM_VERTICES));
        do
        {
            for (int i = 0; i < 5; ++i)
            {
                cout << "\n---------------------------------------\n\nEXAMPLE " << i + 1 << "\n";
                for (int j = 0; j < NUM_VERTICES; ++j)
                {

                    for (int k = j; k < NUM_VERTICES; ++k)
                    {
                        adjacencyMatrix[j][k] = adjacencyMatrix[k][j] = rand() % 2;
                    }
                }
                printMatrix(adjacencyMatrix);
                GeneticAlgorithm ga(adjacencyMatrix, 100, 1000);

                bitset<NUM_VERTICES> maxClique = ga.maxClique();

                //Вывод максимальной клики
                cout << "\nMax clique: ";
                for (int i = 0; i < NUM_VERTICES; ++i) {
                    if (maxClique[i]) {
                        cout << i + 1 << " ";
                    }
                }
            }
            cout << "\nDo you want more examples[y/n]?\n";
            do
            {
                cin >> ans;
                if (ans != 'y' && ans != 'n') cout << "\nIncorrect input: type y or n";
            } while (ans != 'y' && ans != 'n');
        } while (ans == 'y');
    }
    catch (WrongIValueException e)
    {
        e.print();
    }
    return 0;
}
