#include <iostream>
#include <vector>
#include <random>
#include <cmath>
#include <fstream>
#include <cstdlib>
#include <thread>
#include <chrono>
#include <windows.h>
#include <filesystem>
#include "AuctionAlgo.hpp"

// Функция для генерации матриц alpha, visibility_robots и visibility_tasks
void generate_random_instance(
    int n, int m,
    std::vector<std::vector<double>>& alpha_auction,
    std::vector<std::vector<int>>& visibility_robots,
    std::vector<std::vector<int>>& visibility_tasks,
    std::vector<Point>& robot_coords,
    std::vector<Point>& task_coords
    )
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> coord_dist(PARAMETRS::min_utility, PARAMETRS::max_utility);

    // Инициализация матриц
    alpha_auction.assign(n, std::vector<double>(m, -std::numeric_limits<double>::infinity()));
    visibility_robots.assign(n, std::vector<int>(n, 0));
    visibility_tasks.assign(n, std::vector<int>(m, 0));

    // Генерация координат
    robot_coords.resize(n);
    task_coords.resize(m);

    for (int i = 0; i < n; ++i) {
        robot_coords[i] = {coord_dist(gen), coord_dist(gen)};
    }
    for (int j = 0; j < m; ++j) {
        task_coords[j] = {coord_dist(gen), coord_dist(gen)};
    }

    // Заполнение visibility_robots и visibility_tasks на основе радиуса видимости
    for (int i = 0; i < n; ++i) {
        // Видимость других роботов
        for (int j = 0; j < n; ++j) {
            if (i != j && calculate_distance(robot_coords[i], robot_coords[j]) <= PARAMETRS::visibility_radius) {
                visibility_robots[i][j] = 1;
            }
        }
        // Видимость задач
        for (int j = 0; j < m; ++j) {
            if (calculate_distance(robot_coords[i], task_coords[j]) <= PARAMETRS::visibility_radius) {
                visibility_tasks[i][j] = 1;
            }
        }
    }

    // Заполнение alpha на основе visibility_tasks
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < m; ++j) {
            if (visibility_tasks[i][j]) {
                double distance = calculate_distance(robot_coords[i], task_coords[j]);
                alpha_auction[i][j] = PARAMETRS::max_utility / (distance + PARAMETRS::DISTANCE_OFFSET);
            }
        }
    }
}

int main()
{
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    AuctionAlgo<double> algo;
    PARAMETRS::min_utility = 1.0;
    PARAMETRS::max_utility = 30.0;
    PARAMETRS::visibility_radius = 15.0; // Радиус видимости
    int num_iterations = 6;
    int iter = 0;

    std::ofstream auction_json("auction_assignments.json");
    if (!auction_json.is_open()) {
        std::cerr << "Ошибка создания JSON-файла\n";
        return 1;
    }
    auction_json << "[\n";

    for (int n = 1; n < num_iterations; ++n) {
        for (int m = 1; m < num_iterations; ++m) {
#ifdef DEBUG
            std::cout << "\nИтерация " << iter + 1 << ":\n";
#endif

            std::vector<std::vector<double>> alpha;
            std::vector<std::vector<int>> visibility_robots;
            std::vector<std::vector<int>> visibility_tasks;
            std::vector<Point> robot_coords;
            std::vector<Point> task_coords;

            // Генерация экземпляра с учётом радиуса видимости
            generate_random_instance(n, m, alpha, visibility_robots, visibility_tasks,
                                     robot_coords, task_coords);

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

            std::cout << "Видимость роботов (visibility_robots):\n";
            for (int i = 0; i < n; ++i) {
                for (int j = 0; j < n; ++j) {
                    std::cout << visibility_robots[i][j] << " ";
                }
                std::cout << "\n";
            }

            std::cout << "Видимость задач (visibility_tasks):\n";
            for (int i = 0; i < n; ++i) {
                for (int j = 0; j < m; ++j) {
                    std::cout << visibility_tasks[i][j] << " ";
                }
                std::cout << "\n";
            }
#endif

            std::cout << "Итерация " << iter + 1 << ": \n";
            std::cout << "Матрица: " << n << "x" << m << std::endl;

            // Аукционный алгоритм с учётом visibility_robots и visibility_tasks
            std::vector<int> auction_assignment;
            double auction_utility = algo.Start(n, m, alpha, visibility_tasks, visibility_robots,
                                                robot_coords, task_coords, PARAMETRS::epsilon, auction_assignment);

            std::cout << "Назначения (аукционный, робот -> задача): ";
            for (int i = 0; i < n; ++i) {
                std::cout << auction_assignment[i] << " ";
            }
            std::cout << "\nОбщая полезность (аукционный): " << auction_utility << "\n\n";

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

            iter++;
        }
    }

    auction_json << "\n]";
    auction_json.close();

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
