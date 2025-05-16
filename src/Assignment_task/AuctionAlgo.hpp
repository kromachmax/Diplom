#ifndef AUCTION_ALGO
#define AUCTION_ALGO

#include <vector>
#include <limits>
#include <algorithm>
#include <queue>
#include <iostream>
#include <cmath>

namespace PARAMETRS
{
    static double min_utility;
    static double max_utility;
    static double visibility_radius;
    const  double DISTANCE_OFFSET = 0.1;
    const  double epsilon = 1e-5;
}

// Структура для хранения координат
struct Point {
    double x, y;
};

// Функция для вычисления евклидова расстояния
double calculate_distance(const Point& p1, const Point& p2) {
    return std::sqrt(std::pow(p2.x - p1.x, 2) + std::pow(p2.y - p1.y, 2));
}

template<typename T>
class AuctionAlgo
{
private:
    // Вспомогательная функция для поиска максимума и индекса
    std::pair<T, int> find_max(const std::vector<T>& values) {
        T max_val = std::numeric_limits<T>::lowest();
        int max_idx = -1;
        for (size_t i = 0; i < values.size(); ++i) {
            if (values[i] > max_val) {
                max_val = values[i];
                max_idx = i;
            }
        }
        return {max_val, max_idx};
    }

    void update_available_tasks(
        int n, int m,
        std::vector<std::vector<T>>& alpha,
        const std::vector<std::vector<int>>& visibility_robots,
        const std::vector<std::vector<int>>& visibility_tasks,
        const std::vector<Point>& robot_coords,
        const std::vector<Point>& task_coords,
        std::vector<std::vector<int>>& available_tasks)
    {
        const double DISTANCE_OFFSET = 0.1;

        // Используем BFS для распространения информации о задачах
        for (int start_robot = 0; start_robot < n; ++start_robot)
        {
            std::vector<bool> visited(n, false);
            std::queue<int> q;
            q.push(start_robot);
            visited[start_robot] = true;

            while (!q.empty())
            {
                int current_robot = q.front();
                q.pop();

                // Добавляем все задачи, видимые текущим роботом
                for (int task = 0; task < m; ++task)
                {
                    if (visibility_tasks[current_robot][task])
                    {
                        if (std::find(available_tasks[start_robot].begin(),
                                      available_tasks[start_robot].end(), task) ==
                            available_tasks[start_robot].end())
                        {
                            available_tasks[start_robot].push_back(task);
                            // Обновляем alpha
                            if (alpha[start_robot][task] == -std::numeric_limits<double>::infinity())
                            {
                                double distance = calculate_distance(robot_coords[start_robot], task_coords[task]);
                                alpha[start_robot][task] = PARAMETRS::max_utility / (distance + DISTANCE_OFFSET);
                                std::cout << "Updated alpha[" << start_robot << "][" << task << "] = " << alpha[start_robot][task] << "\n";
                            }
                        }
                    }
                }

                // Посещаем соседних роботов
                for (int next_robot = 0; next_robot < n; ++next_robot)
                {
                    if (visibility_robots[current_robot][next_robot] && !visited[next_robot])
                    {
                        visited[next_robot] = true;
                        q.push(next_robot);
                    }
                }
            }
        }

        std::cout << "Available tasks:\n";
        for (const auto& robot : available_tasks)
        {
            for (const auto& available_task : robot)
            {
                std::cout << available_task << ' ';
            }
            std::cout << '\n';
        }

        std::cout << "Updated alpha matrix:\n";
        for (int i = 0; i < n; ++i)
        {
            for (int j = 0; j < m; ++j)
            {
                if (alpha[i][j] == -std::numeric_limits<double>::infinity())
                    std::cout << "-inf ";
                else
                    std::cout << alpha[i][j] << ' ';
            }
            std::cout << '\n';
        }
    }

public:
    T Start(int n, int m,
            std::vector<std::vector<T>>& alpha,
            const std::vector<std::vector<int>>& visibility_tasks,
            const std::vector<std::vector<int>>& visibility_robots,
            const std::vector<Point>& robot_coords,
            const std::vector<Point>& task_coords,
            double epsilon,
            std::vector<int>& assignment)
    {
        // Инициализация доступных задач на основе начальной видимости
        std::vector<std::vector<int>> available_tasks(n);
        for (int i = 0; i < n; ++i)
        {
            for (int j = 0; j < m; ++j)
            {
                if (visibility_tasks[i][j]) {
                    available_tasks[i].push_back(j);
                }
            }
        }

        // Обновляем доступные задачи с учётом обмена информацией
        update_available_tasks(n, m, alpha, visibility_robots, visibility_tasks, robot_coords, task_coords, available_tasks);

        // Инициализация цен задач
        std::vector<T> prices(m, T(0));

        // Инициализация назначений (робот -> задача)
        assignment.resize(n, -1); // Изначально ни один робот не назначен

        // Вспомогательный вектор для отслеживания, какая задача назначена какому роботу
        std::vector<int> task_to_robot(m, -1); // -1 означает, что задача не назначена

        // Начальное назначение: назначаем задачи уникально
        for (int i = 0; i < n; ++i) {
            if (!available_tasks[i].empty()) {
                // Ищем незанятую задачу среди доступных
                for (int task : available_tasks[i]) {
                    if (task_to_robot[task] == -1) { // Задача свободна
                        assignment[i] = task;
                        task_to_robot[task] = i;
                        break;
                    }
                }
            }
        }

        // Флаг для проверки, "счастливы" ли все роботы
        bool all_happy = false;

        while (!all_happy)
        {
            all_happy = true;

            for (int i = 0; i < n; ++i) {
                // Если у робота нет доступных задач, пропускаем
                if (available_tasks[i].empty())
                {
                    assignment[i] = -1;
                    continue;
                }

                // Вычисляем прибыль для текущей задачи (если робот назначен)
                T current_profit = (assignment[i] != -1)
                                       ? alpha[i][assignment[i]] - prices[assignment[i]]
                                       : T(0); // Неназначенный робот имеет прибыль 0

                // Находим максимальную прибыль по доступным задачам
                T max_profit = std::numeric_limits<T>::lowest();
                for (int j : available_tasks[i])
                {
                    T profit = alpha[i][j] - prices[j];
                    max_profit = std::max(max_profit, profit);
                }

                // Если робот неназначен, сравниваем с 0
                if (assignment[i] == -1) {
                    max_profit = std::max(max_profit, T(0));
                }

                // Проверяем, "счастлив" ли робот
                if (current_profit < max_profit - epsilon)
                {
                    all_happy = false; // Робот несчастлив, продолжаем

                    // Находим задачу с максимальной прибылью, включая фиктивную
                    std::vector<T> profits(m, std::numeric_limits<T>::lowest());
                    for (int j : available_tasks[i]) {
                        profits[j] = alpha[i][j] - prices[j];
                    }
                    profits.push_back(T(0)); // Прибыль для фиктивной задачи (неназначение)

                    auto [v_i, t_i] = find_max(profits); // v_i - максимальная прибыль, t_i - индекс задачи
                    if (t_i == static_cast<int>(profits.size()) - 1) {
                        t_i = -1; // Робот выбирает фиктивную задачу (неназначение)
                    }

                    // Находим вторую по величине прибыль
                    profits[t_i == -1 ? profits.size() - 1 : t_i] = std::numeric_limits<T>::lowest();
                    auto [w_i, _] = find_max(profits); // w_i - вторая по величине прибыль

                    // Если робот выбирает фиктивную задачу, не меняем цены
                    if (t_i == -1)
                    {
                        if (assignment[i] != -1) {
                            task_to_robot[assignment[i]] = -1; // Освобождаем старую задачу
                        }
                        assignment[i] = -1;
                        continue;
                    }

                    // Находим робота, который сейчас назначен на задачу t_i
                    int current_owner = task_to_robot[t_i];
                    if (current_owner != -1)
                    {
                        // Старому владельцу даём неназначение (фиктивную задачу)
                        assignment[current_owner] = -1;
                    }

                    // Новый робот получает новую задачу
                    assignment[i] = t_i;
                    task_to_robot[t_i] = i;

                    // Обновляем цену задачи t_i
                    prices[t_i] += (v_i - w_i + epsilon);
                }
            }
        }

        // Вычисление общей полезности (только для реальных задач)
        T total_utility = 0;
        for (int i = 0; i < n; ++i)
        {
            if (assignment[i] != -1) {
                total_utility += alpha[i][assignment[i]];
            }
        }
        return total_utility;
    }
};

#endif
