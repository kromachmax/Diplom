#include <iostream>
#include <vector>
#include <random>
#include <algorithm>
#include "AuctionAlgo.hpp"
#include "HungarianAlgo.hpp"

// Функция для генерации случайной матрицы alpha и available_tasks
void generate_random_instance(int n, int m, double min_utility, double max_utility,
                              std::vector<std::vector<double>>& alpha,
                              std::vector<std::vector<int>>& available_tasks) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> util_dist(min_utility, max_utility);
    std::uniform_int_distribution<> task_count_dist(1, m); // Количество доступных задач для робота

    alpha.assign(n, std::vector<double>(m, -std::numeric_limits<double>::infinity()));
    available_tasks.assign(n, std::vector<int>());

    // Шаг 1: Гарантируем, что каждый робот имеет хотя бы одну задачу
    // Создаём начальное паросочетание, чтобы обеспечить возможность назначения
    std::vector<int> tasks(m);
    for (int i = 0; i < m; ++i) tasks[i] = i;
    std::shuffle(tasks.begin(), tasks.end(), gen); // Перемешиваем задачи

    for (int i = 0; i < n && i < m; ++i) {
        available_tasks[i].push_back(tasks[i]);
        alpha[i][tasks[i]] = util_dist(gen);
    }

    // Шаг 2: Добавляем дополнительные случайные задачи
    for (int i = 0; i < n; ++i) {
        int num_tasks = task_count_dist(gen); // Сколько задач доступно роботу
        num_tasks = std::min(num_tasks, m - static_cast<int>(available_tasks[i].size())); // Ограничиваем
        std::vector<int> remaining_tasks;
        for (int j = 0; j < m; ++j) {
            if (std::find(available_tasks[i].begin(), available_tasks[i].end(), j) == available_tasks[i].end()) {
                remaining_tasks.push_back(j);
            }
        }
        std::shuffle(remaining_tasks.begin(), remaining_tasks.end(), gen);
        for (int k = 0; k < num_tasks && !remaining_tasks.empty(); ++k) {
            int task = remaining_tasks.back();
            remaining_tasks.pop_back();
            available_tasks[i].push_back(task);
            alpha[i][task] = util_dist(gen);
        }
    }

    // Шаг 3: Проверяем, что каждый робот имеет хотя бы одну задачу
    for (int i = 0; i < n; ++i) {
        if (available_tasks[i].empty()) {
            // Добавляем случайную задачу
            int task = std::uniform_int_distribution<>(0, m-1)(gen);
            available_tasks[i].push_back(task);
            alpha[i][task] = util_dist(gen);
        }
    }
}

int main() {

    AuctionAlgo<double> algo;
    double epsilon = 0.5;
    double min_utility = 1.0; // Минимальная полезность
    double max_utility = 100.0; // Максимальная полезность
    int num_iterations = 15; // Количество итераций
    int iter = 0;

    for(int n = 1; n < num_iterations; ++n)
    {
        for(int m = n; m < num_iterations; ++m)
        {

#ifdef DEBUG
            std::cout << "\nИтерация " << iter + 1 << ":\n";
#endif

            // Генерация случайной матрицы и available_tasks
            std::vector<std::vector<double>> alpha;
            std::vector<std::vector<int>> available_tasks;
            generate_random_instance(n, m, min_utility, max_utility, alpha, available_tasks);

#ifdef DEBUG
            // Вывод матрицы alpha
            std::cout << "Матрица полезностей (alpha):\n";
            for (int i = 0; i < n; ++i) {
                for (int j = 0; j < m; ++j) {
                    if (alpha[i][j] == -std::numeric_limits<double>::infinity()) {
                        std::cout << "-inf ";
                    } else {
                        std::cout << alpha[i][j] << " ";
                    }
                }
                std::cout << "\n";
            }

            // Вывод available_tasks
            std::cout << "Доступные задачи (available_tasks):\n";
            for (int i = 0; i < n; ++i) {
                std::cout << "Робот " << i << ": ";
                for (int task : available_tasks[i]) {
                    std::cout << task << " ";
                }
                std::cout << "\n";
            }
#endif

            std::cout << "Итерация " << iter + 1 << ": \n";
            std::cout << "Матрица: " << n << "x" << m << std::endl;
            // Запуск аукционного алгоритма
            std::vector<int> assignment;
            double total_utility = algo.Start(n, m, alpha, available_tasks, epsilon, assignment);

            // Вывод результатов
            std::cout << "Назначения (робот -> задача): ";
            for (int i = 0; i < n; ++i) {
                std::cout << assignment[i] << " ";
            }
            std::cout << "\nОбщая полезность: " << total_utility << "\n";


            HungarionAlgo<double> hunAlgo;
            std::vector<int> N(m, 1);
            hunAlgo.Update(n, m, alpha, N);
            total_utility = hunAlgo.Start();
            std::cout << "\nОбщая полезность: " << total_utility << "\n\n";

            iter++;
        }
    }

    return 0;
}
