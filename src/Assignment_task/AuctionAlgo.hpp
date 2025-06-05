#ifndef AUCTION_ALGO
#define AUCTION_ALGO

#include <vector>
#include <limits>
#include <algorithm>
#include <queue>
#include <cmath>
#include <thread>
#include <mutex>
#include <optional>

namespace PARAMETRS
{
static double min_utility;
static double max_utility;
static double visibility_radius;
const  double DISTANCE_OFFSET = 1e-2;
const  double epsilon = 1e-2;
}

// Структура для хранения координат
struct Point
{
    double x, y;
};

// Функция для вычисления евклидова расстояния
double calculate_distance(const Point& p1, const Point& p2)
{
    return std::sqrt(std::pow(p2.x - p1.x, 2) + std::pow(p2.y - p1.y, 2));
}

template<typename T>
class AuctionAlgo
{
private:
    // Вспомогательная функция для поиска максимума и индекса
    std::pair<T, int> FindMax(const std::vector<T>& values)
    {
        T max_val = std::numeric_limits<T>::lowest();
        int max_idx = -1;

        for (size_t i = 0; i < values.size(); ++i)
        {
            if (values[i] > max_val)
            {
                max_val = values[i];
                max_idx = i;
            }
        }
        return {max_val, max_idx};
    }

    std::vector<std::vector<int>> FindConnectedComponents(const std::vector<std::vector<int>>& visibility)
    {
        int n = visibility.size();
        std::vector<bool> visited(n, false);
        std::vector<std::vector<int>> components;

        for (int i = 0; i < n; ++i)
        {
            if (!visited[i])
            {
                std::vector<int> component;
                std::queue<int> q;
                q.push(i);
                visited[i] = true;

                while (!q.empty())
                {
                    int u = q.front();
                    q.pop();
                    component.push_back(u);

                    for (int v = 0; v < n; ++v)
                    {
                        if (visibility[u][v] && !visited[v])
                        {
                            visited[v] = true;
                            q.push(v);
                        }
                    }
                }
                components.push_back(component);
            }
        }
        return components;
    }

    T RunningForComponent(int n, int m,
                          const std::vector<std::vector<T>>& alpha,
                          double epsilon,
                          std::vector<int>& assignment,
                          std::optional<std::reference_wrapper<std::vector<double>>> value_per_iteration = std::nullopt,
                          std::optional<std::reference_wrapper<int>> op_count = std::nullopt,
                          std::optional<std::reference_wrapper<int>> logic_op_count = std::nullopt,
                          std::optional<std::reference_wrapper<int>> change_op_count = std::nullopt)
    {

        // Инициализация цен задач
        std::vector<T> prices(m, T(0));

        // Инициализация назначений (робот -> задача)
        assignment.resize(n, -1); // Изначально ни один робот не назначен

        // Вспомогательный вектор для отслеживания, какая задача назначена какому роботу
        std::vector<int> task_to_robot(m, -1); // -1 означает, что задача не назначена

        int operation_count = 0;
        int logic_operation_count = 0;
        int change_operation_count = 0;

        // Начальное назначение: назначаем задачи уникально
        for (int i = 0; i < n; ++i)
        {
            for (int task = 0; task < m; ++task)
            {
                logic_operation_count += 1;

                if (task_to_robot[task] == -1)
                {
                    assignment[i] = task;
                    task_to_robot[task] = i;
                    operation_count += 2;
                    change_operation_count += n;
                    break;
                }
            }
        }

        // Флаг для проверки, "счастливы" ли все роботы
        bool all_happy = false;

        while (!all_happy)
        {
            all_happy = true;
            operation_count += 1;

            for (int i = 0; i < n; ++i)
            {
                // Вычисляем прибыль для текущей задачи (если робот назначен)
                T current_profit = (assignment[i] != -1)
                                       ? alpha[i][assignment[i]] - prices[assignment[i]]
                                       : T(0);

                operation_count += 2;
                logic_operation_count += 1;

                // Находим максимальную прибыль по доступным задачам
                T max_profit = T(0);
                operation_count += 1;

                for (int j = 0; j < m; ++j)
                {
                    T profit = alpha[i][j] - prices[j];
                    max_profit = std::max(max_profit, profit);
                }

                operation_count += 3 * std::log2(m);

                logic_operation_count += 1;

                // Проверяем, "счастлив" ли робот
                if (current_profit < max_profit - epsilon)
                {
                    all_happy = false; // Робот несчастлив, продолжаем
                    operation_count += 1;

                    // Находим задачу с максимальной прибылью, включая фиктивную
                    std::vector<T> profits(m, std::numeric_limits<T>::lowest());

                    for (int j = 0; j < m; ++j)
                    {
                        profits[j] = alpha[i][j] - prices[j];
                    }
                    profits.push_back(T(0)); // Прибыль для фиктивной задачи (неназначение)

                    auto [v_i, t_i] = FindMax(profits); // v_i - максимальная прибыль, t_i - индекс задачи

                    if (t_i == static_cast<int>(profits.size()) - 1)
                    {
                        t_i = -1; // Робот выбирает фиктивную задачу (неназначение)
                    }

                    // Находим вторую по величине прибыль
                    profits[t_i == -1 ? profits.size() - 1 : t_i] = std::numeric_limits<T>::lowest();

                    auto [w_i, _] = FindMax(profits); // w_i - вторая по величине прибыль

                    logic_operation_count += 1;

                    // Если робот выбирает фиктивную задачу, не меняем цены
                    if (t_i == -1)
                    {
                        logic_operation_count += 1;
                        if (assignment[i] != -1)
                        {
                            task_to_robot[assignment[i]] = -1; // Освобождаем старую задачу
                            operation_count += 1;
                        }
                        assignment[i] = -1;
                        operation_count += 1;
                        continue;
                    }

                    // Находим робота, который сейчас назначен на задачу t_i
                    int current_owner = task_to_robot[t_i];
                    operation_count += 1;


                    logic_operation_count += 1;
                    if (current_owner != -1)
                    {
                        // Старому владельцу даём неназначение (фиктивную задачу)
                        assignment[current_owner] = -1;
                        operation_count += 1;
                    }

                    logic_operation_count += 1;
                    if(assignment[i] != -1)
                    {
                        // Освобождаем старую задачу
                        task_to_robot[assignment[i]] = -1;
                        operation_count += 1;
                    }

                    // Новый робот получает новую задачу
                    assignment[i] = t_i;
                    task_to_robot[t_i] = i;
                    operation_count += 2;


                    // Обновляем цену задачи t_i
                    prices[t_i] += (v_i - w_i + epsilon);
                    operation_count += 3;

                    change_operation_count += n;
                }
            }

            if(value_per_iteration.has_value())
            {
                T current_utility = 0;
                for (int i = 0; i < n; ++i)
                {
                    if (assignment[i] != -1) {
                        current_utility += alpha[i][assignment[i]];
                    }
                }
                value_per_iteration->get().push_back(current_utility);
            }
        }

        if(op_count.has_value())
        {
            op_count->get() = operation_count;
            logic_op_count->get() = logic_operation_count;
            change_op_count->get() = change_operation_count;
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

public:
    T Start(int n, int m,
            std::vector<std::vector<T>>& alpha,
            const std::vector<std::vector<int>>& visibility_robots,
            double epsilon,
            std::vector<int>& assignment,
            std::optional<std::reference_wrapper<std::vector<double>>> value_per_iteration = std::nullopt,
            std::optional<std::reference_wrapper<int>> operation_count = std::nullopt,
            std::optional<std::reference_wrapper<int>> logic_op_count = std::nullopt,
            std::optional<std::reference_wrapper<int>> change_count = std::nullopt)
    {
        // Находим компоненты связности
        auto components = FindConnectedComponents(visibility_robots);

        // Инициализация
        assignment.resize(n, -1);
        T total_utility = 0;

        // Для хранения максимальной полезности по каждой задаче
        std::vector<T> max_task_utility(m, std::numeric_limits<T>::lowest());

        std::mutex mtx;
        std::vector<std::thread> threads;

        // 1. Выполняем аукцион для всех компонент
        for (const auto& component : components)
        {
            std::size_t n = component.size();
            std::vector<std::vector<T>> component_alpha;
            for (int robot : component) {
                component_alpha.push_back(alpha[robot]);
            }

            threads.emplace_back([&, component_alpha, n]() {

                std::vector<int> component_assignment;

                if(value_per_iteration.has_value())
                {
                    RunningForComponent(n, m, component_alpha, epsilon, component_assignment, value_per_iteration);
                }
                else if(operation_count.has_value())
                {
                    RunningForComponent(n, m, component_alpha, epsilon, component_assignment, std::nullopt, operation_count, logic_op_count, change_count);
                }
                else
                {
                    RunningForComponent(n, m, component_alpha, epsilon, component_assignment);
                }

                std::lock_guard<std::mutex> lock(mtx);

                // Обновляем назначения и максимальные полезности
                for (size_t i = 0; i < component.size(); ++i)
                {
                    int robot = component[i];
                    int task = component_assignment[i];
                    assignment[robot] = task;

                    // Обновляем максимальную полезность для задачи
                    if (task != -1) {
                        max_task_utility[task] = std::max(max_task_utility[task],
                                                          alpha[robot][task]);
                    }
                }
            });
        }

        for(auto& thread : threads)
        {
            thread.join();
        }

        // 2. Суммируем только максимальные полезности
        for (int task = 0; task < m; ++task) {
            if (max_task_utility[task] > std::numeric_limits<T>::lowest()) {
                total_utility += max_task_utility[task];
            }
        }

        return total_utility;
    }
};

#endif
