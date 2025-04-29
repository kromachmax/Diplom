#include <iostream>
#include <vector>
#include <random>
#include <algorithm>
#include <cmath>
#include <fstream>
#include <cstdlib>
#include <thread>
#include <chrono>
#include <filesystem>
#include "AuctionAlgo.hpp"
#include "HungarianAlgo.hpp"

// Структура для хранения координат
struct Point {
    double x, y;
};

// Функция для вычисления евклидова расстояния
double calculate_distance(const Point& p1, const Point& p2) {
    return std::sqrt(std::pow(p2.x - p1.x, 2) + std::pow(p2.y - p1.y, 2));
}

// Функция для генерации матриц alpha и alpha_hungarian
void generate_random_instance(int n, int m, double min_utility, double max_utility,
                             std::vector<std::vector<double>>& alpha,
                             std::vector<std::vector<int>>& available_tasks,
                             std::vector<std::vector<double>>& alpha_hungarian,
                             std::vector<Point>& robot_coords,
                             std::vector<Point>& task_coords)
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> coord_dist(0.0, 100.0);
    std::uniform_int_distribution<> task_count_dist(1, m);
    const double DISTANCE_OFFSET = 0.2;

    // Инициализация матриц
    alpha.assign(n, std::vector<double>(m, -std::numeric_limits<double>::infinity()));
    alpha_hungarian.assign(n, std::vector<double>(m, 0.0));
    available_tasks.assign(n, std::vector<int>());

    // Генерация координат
    robot_coords.resize(n);
    task_coords.resize(m);

    for (int i = 0; i < n; ++i) {
        robot_coords[i] = {coord_dist(gen), coord_dist(gen)};
    }
    for (int j = 0; j < m; ++j) {
        task_coords[j] = {coord_dist(gen), coord_dist(gen)};
    }

    // Заполнение alpha_hungarian (все задачи доступны)
    for (int i = 0; i < n; ++i)
    {
        for (int j = 0; j < m; ++j)
        {
            double distance = calculate_distance(robot_coords[i], task_coords[j]);
            alpha_hungarian[i][j] = max_utility / (distance + DISTANCE_OFFSET);
        }
    }

    // Заполнение alpha и available_tasks (с ограничениями видимости)
    std::vector<int> tasks(m);
    for (int i = 0; i < m; ++i) tasks[i] = i;
    std::shuffle(tasks.begin(), tasks.end(), gen);

    for (int i = 0; i < n && i < m; ++i)
    {
        int task = tasks[i];
        available_tasks[i].push_back(task);
        double distance = calculate_distance(robot_coords[i], task_coords[task]);
        alpha[i][task] = max_utility / (distance + DISTANCE_OFFSET);
    }

    for (int i = 0; i < n; ++i)
    {
        int num_tasks = task_count_dist(gen);
        num_tasks = std::min(num_tasks, m - static_cast<int>(available_tasks[i].size()));
        std::vector<int> remaining_tasks;

        for (int j = 0; j < m; ++j)
        {
            if (std::find(available_tasks[i].begin(), available_tasks[i].end(), j) == available_tasks[i].end()) {
                remaining_tasks.push_back(j);
            }
        }

        std::shuffle(remaining_tasks.begin(), remaining_tasks.end(), gen);
        for (int k = 0; k < num_tasks && !remaining_tasks.empty(); ++k)
        {
            int task = remaining_tasks.back();
            remaining_tasks.pop_back();
            available_tasks[i].push_back(task);
            double distance = calculate_distance(robot_coords[i], task_coords[task]);
            alpha[i][task] = max_utility / (distance + DISTANCE_OFFSET);
        }
    }

    for (int i = 0; i < n; ++i)
    {
        if (available_tasks[i].empty())
        {
            int task = std::uniform_int_distribution<>(0, m-1)(gen);
            available_tasks[i].push_back(task);
            double distance = calculate_distance(robot_coords[i], task_coords[task]);
            alpha[i][task] = max_utility / (distance + DISTANCE_OFFSET);
        }
    }
}

int main()
{
    AuctionAlgo<double> algo;
    double epsilon = 0.5;
    double min_utility = 1.0;
    double max_utility = 100.0;
    int num_iterations = 15;
    int iter = 0;

    std::ofstream auction_json("auction_assignments.json");
    std::ofstream hungarian_json("hungarian_assignments.json");
    if (!auction_json.is_open() || !hungarian_json.is_open()) {
        std::cerr << "Ошибка создания JSON-файлов\n";
        return 1;
    }
    auction_json << "[\n";
    hungarian_json << "[\n";

    for (int n = 1; n < num_iterations; ++n)
    {
        for (int m = n; m < num_iterations; ++m)
        {
#ifdef DEBUG
            std::cout << "\nИтерация " << iter + 1 << ":\n";
#endif

            std::vector<std::vector<double>> alpha;
            std::vector<std::vector<int>> available_tasks;
            std::vector<std::vector<double>> alpha_hungarian;
            std::vector<Point> robot_coords;
            std::vector<Point> task_coords;
            generate_random_instance(n, m, min_utility, max_utility, alpha, available_tasks, alpha_hungarian, robot_coords, task_coords);

#ifdef DEBUG
            std::cout << "Координаты роботов:\n";
            for (int i = 0; i < n; ++i) {
                std::cout << "Робот " << i << ": (" << robot_coords[i].x << ", " << robot_coords[i].y << ")\n";
            }

            std::cout << "Координаты задач:\n";
            for (int j = 0; j < m; ++j) {
                std::cout << "Задача " << j << ": (" << task_coords[j].x << ", " << task_coords[j].y << ")\n";
            }

            std::cout << "Матрица полезностей (alpha, аукционный):\n";
            for (int i = 0; i < n; ++i)
            {
                for (int j = 0; j < m; ++j)
                {
                    if (alpha[i][j] == -std::numeric_limits<double>::infinity()) {
                        std::cout << "-inf ";
                    } else {
                        std::cout << alpha[i][j] << " ";
                    }
                }
                std::cout << "\n";
            }

            std::cout << "Матрица полезностей (alpha_hungarian, венгерский):\n";
            for (int i = 0; i < n; ++i)
            {
                for (int j = 0; j < m; ++j)
                {
                    std::cout << alpha_hungarian[i][j] << " ";
                }
                std::cout << "\n";
            }

            std::cout << "Доступные задачи (available_tasks):\n";
            for (int i = 0; i < n; ++i)
            {
                std::cout << "Робот " << i << ": ";
                for (int task : available_tasks[i]) {
                    std::cout << task << " ";
                }
                std::cout << "\n";
            }
#endif

            std::cout << "Итерация " << iter + 1 << ": \n";
            std::cout << "Матрица: " << n << "x" << m << std::endl;

            // Аукционный алгоритм
            std::vector<int> auction_assignment;
            double auction_utility = algo.Start(n, m, alpha, available_tasks, epsilon, auction_assignment);

            std::cout << "Назначения (аукционный, робот -> задача): ";
            for (int i = 0; i < n; ++i) {
                std::cout << auction_assignment[i] << " ";
            }
            std::cout << "\nОбщая полезность (аукционный): " << auction_utility << "\n";

            // Венгерский алгоритм
            HungarianAlgo<double> hunAlgo;
            std::vector<int> N(m, 1);
            std::vector<int> hungarian_assignment;
            hunAlgo.Update(n, m, alpha_hungarian, N);
            double hungarian_utility = hunAlgo.Start(hungarian_assignment);
            std::cout << "Назначения (венгерский, робот -> задача): ";
            for (int i = 0; i < n; ++i) {
                std::cout << hungarian_assignment[i] << " ";
            }
            std::cout << "\nОбщая полезность (венгерский): " << hungarian_utility << "\n\n";

            // Запись в auction_assignments.json
            if (iter > 0) auction_json << ",\n";
            auction_json << "  {\"n\":" << n << ",\"m\":" << m << ",\"assignment\":[";
            for (size_t i = 0; i < auction_assignment.size(); ++i) {
                auction_json << auction_assignment[i];
                if (i < auction_assignment.size() - 1) auction_json << ",";
            }
            auction_json << "],\"robot_coords\":[";
            for (size_t i = 0; i < robot_coords.size(); ++i) {
                auction_json << "[" << robot_coords[i].x << "," << robot_coords[i].y << "]";
                if (i < robot_coords.size() - 1) auction_json << ",";
            }
            auction_json << "],\"task_coords\":[";
            for (size_t j = 0; j < task_coords.size(); ++j) {
                auction_json << "[" << task_coords[j].x << "," << task_coords[j].y << "]";
                if (j < task_coords.size() - 1) auction_json << ",";
            }
            auction_json << "]}";

            // Запись в hungarian_assignments.json
            if (iter > 0) hungarian_json << ",\n";
            hungarian_json << "  {\"n\":" << n << ",\"m\":" << m << ",\"assignment\":[";
            for (size_t i = 0; i < hungarian_assignment.size(); ++i) {
                hungarian_json << hungarian_assignment[i];
                if (i < hungarian_assignment.size() - 1) hungarian_json << ",";
            }
            hungarian_json << "],\"robot_coords\":[";
            for (size_t i = 0; i < robot_coords.size(); ++i) {
                hungarian_json << "[" << robot_coords[i].x << "," << robot_coords[i].y << "]";
                if (i < robot_coords.size() - 1) hungarian_json << ",";
            }
            hungarian_json << "],\"task_coords\":[";
            for (size_t j = 0; j < task_coords.size(); ++j) {
                hungarian_json << "[" << task_coords[j].x << "," << task_coords[j].y << "]";
                if (j < task_coords.size() - 1) hungarian_json << ",";
            }
            hungarian_json << "]}";

            iter++;
        }
    }

    auction_json << "\n]";
    hungarian_json << "\n]";
    auction_json.close();
    hungarian_json.close();

    // Проверка наличия index.html
    if (!std::filesystem::exists("index.html")) {
        std::cerr << "Ошибка: index.html не найден в папке сборки\n";
        return 1;
    }

    // Завершение процессов, использующих порт 8000 (Linux)
#ifndef _WIN32
    system("fuser -k 8000/tcp > /dev/null 2>&1");
#endif

    // Запуск веб-сервера и открытие браузера
#ifdef _WIN32
    system("start python -m http.server 8000");
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    system("start http://localhost:8000");
#else
    system("python3 -m http.server 8000 &");
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    system("xdg-open http://localhost:8000");
#endif

    return 0;
}
