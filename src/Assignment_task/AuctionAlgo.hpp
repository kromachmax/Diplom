#include <vector>
#include <limits>
#include <algorithm>

template<typename T>
class AuctionAlgo {
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

public:
    T Start(int n, int m, const std::vector<std::vector<T>>& alpha,
            const std::vector<std::vector<int>>& available_tasks,
            double epsilon, std::vector<int>& assignment) {
        // Инициализация цен задач
        std::vector<T> prices(m, T(0));

        // Инициализация назначений (робот -> задача)
        assignment.resize(n, -1); // Изначально ни один робот не назначен

        // Вспомогательный вектор для отслеживания, какая задача назначена какому роботу
        std::vector<int> task_to_robot(m, -1); // -1 означает, что задача не назначена

        // Начальное назначение: каждому роботу даём первую доступную задачу, если есть
        for (int i = 0; i < n; ++i) {
            if (!available_tasks[i].empty() && static_cast<size_t>(i) < m) {
                int task = available_tasks[i][0]; // Берём первую доступную задачу
                assignment[i] = task;
                task_to_robot[task] = i;
            }
        }

        // Флаг для проверки, "счастливы" ли все роботы
        bool all_happy = false;

        while (!all_happy) {
            all_happy = true;

            for (int i = 0; i < n; ++i) {
                // Если у робота нет доступных задач, пропускаем
                if (available_tasks[i].empty()) {
                    assignment[i] = -1;
                    continue;
                }

                // Вычисляем прибыль для текущей задачи (если робот назначен)
                T current_profit = (assignment[i] != -1)
                    ? alpha[i][assignment[i]] - prices[assignment[i]]
                    : T(0); // Неназначенный робот имеет прибыль 0

                // Находим максимальную прибыль по доступным задачам
                T max_profit = std::numeric_limits<T>::lowest();
                for (int j : available_tasks[i]) {
                    T profit = alpha[i][j] - prices[j];
                    max_profit = std::max(max_profit, profit);
                }

                // Если робот неназначен, сравниваем с 0
                if (assignment[i] == -1) {
                    max_profit = std::max(max_profit, T(0));
                }

                // Проверяем, "счастлив" ли робот
                if (current_profit < max_profit - epsilon) {
                    all_happy = false; // Робот несчастлив, продолжаем

                    // Находим задачу с максимальной прибылью
                    std::vector<T> profits(m, std::numeric_limits<T>::lowest());
                    for (int j : available_tasks[i]) {
                        profits[j] = alpha[i][j] - prices[j];
                    }
                    // Учитываем возможность неназначения
                    if (assignment[i] == -1 || max_profit > 0) {
                        profits.push_back(T(0)); // Прибыль для неназначения
                    }

                    auto [v_i, t_i] = find_max(profits); // v_i - максимальная прибыль, t_i - новая задача
                    if (t_i == static_cast<int>(profits.size()) - 1 && profits.size() > m) {
                        t_i = -1; // Робот выбирает неназначение
                    }

                    // Находим вторую по величине прибыль
                    profits[t_i == -1 ? profits.size() - 1 : t_i] = std::numeric_limits<T>::lowest();
                    auto [w_i, _] = find_max(profits); // w_i - вторая по величине прибыль

                    // Если робот выбирает неназначение, не меняем цены
                    if (t_i == -1) {
                        if (assignment[i] != -1) {
                            task_to_robot[assignment[i]] = -1; // Освобождаем старую задачу
                        }
                        assignment[i] = -1;
                        continue;
                    }

                    // Находим робота, который сейчас назначен на задачу t_i
                    int current_owner = task_to_robot[t_i];
                    if (current_owner != -1) {
                        // Старому владельцу даём старую задачу робота i (или неназначение)
                        assignment[current_owner] = assignment[i];
                        if (assignment[i] != -1) {
                            task_to_robot[assignment[i]] = current_owner;
                        } else {
                            task_to_robot[assignment[i]] = -1;
                        }
                    }

                    // Новый робот получает новую задачу
                    assignment[i] = t_i;
                    task_to_robot[t_i] = i;

                    // Обновляем цену задачи t_i
                    prices[t_i] += (v_i - w_i + epsilon);
                }
            }
        }

        // Вычисление общей полезности
        T total_utility = 0;
        for (int i = 0; i < n; ++i) {
            if (assignment[i] != -1) {
                total_utility += alpha[i][assignment[i]];
            }
        }
        return total_utility;
    }
};
