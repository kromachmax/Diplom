#ifndef AUCTIONALGO_HPP
#define AUCTIONALGO_HPP

#include <iostream>
#include <vector>
#include <string>
#include <stdlib.h>
#include <set>
#include <map>
#include <limits>


template <typename T>
class AuctionAlgo
{
public:
    AuctionAlgo() noexcept = default;
    explicit AuctionAlgo(std::size_t n, std::size_t m, std::vector<std::vector<T>>& D, std::vector<int>& N);
    void Update(std::size_t n, std::size_t m, std::vector<std::vector<T>>& D, std::vector<int>& N);
    T Start(int n, int m, const vector<vector<T>>& alpha, double epsilon, vector<int>& assignment);

private:
    [[nodiscard]] bool isUsedRows() const noexcept;
    [[nodiscard]] bool isUsedCols() const noexcept;
    template<typename F>
    [[nodiscard]]bool notNull(const std::vector<F>& v) const noexcept;
    std::pair<T, int> find_max(const std::vector<T>&arr);

    void printMatrix(const std::vector<std::vector<T>>& matrix) const;

private:

    const double eps = 1e-5;

    std::size_t num_rows = 0;
    std::size_t num_cols = 0;

    std::vector<std::vector<T>> D;
    std::vector<int> N_max;
    std::vector<bool> used_rows;
    std::vector<bool> used_cols;

    std::map<std::size_t, std::size_t> distribution_plan;

    T answer = T(0);
};


template<typename T>
AuctionAlgo<T>::AuctionAlgo(std::size_t n, std::size_t m, std::vector<std::vector<T>>& D, std::vector<int>& N)
    : num_rows(n), num_cols(m), D(D), N_max(N), used_rows(n, false), used_cols(m, false)
{
    if (D.size() != n || (n > 0 && D[0].size() != m) || N.size() != m)
    {
        throw std::invalid_argument("Invalid dimensions or sizes for D or N");
    }
}

template<typename T>
T AuctionAlgo<T>::Start(int n, int m, const std::vector<std::vector<T>>& alpha, double epsilon, std::vector<int>& assignment)
{
    // Инициализация цен задач
    std::vector<T> prices(m, T(0));

    // Инициализация назначений (робот -> задача)
    assignment.resize(n, -1); // Изначально ни один робот не назначен

    // Вспомогательный вектор для отслеживания, какая задача назначена какому роботу
    std::vector<int> task_to_robot(m, -1); // -1 означает, что задача не назначена

    // Начальное назначение: каждому роботу даем задачу, если задач достаточно
    for (int i = 0; i < n && i < m; i++)
    {
        assignment[i] = i;
        task_to_robot[i] = i;
    }

    // Флаг для проверки, "счастливы" ли все роботы
    bool all_happy = false;

    while (!all_happy)
    {
        all_happy = true;

        for (int i = 0; i < n; i++)
        {
            // Если робот не назначен на задачу и задач больше нет, пропускаем
            if (assignment[i] == -1 && i >= m) continue;

            // Вычисляем прибыль для текущей задачи (если робот назначен)
            T current_profit = (assignment[i] != -1) ? alpha[i][assignment[i]] - prices[assignment[i]] : numeric_limits<T>::lowest();
            // Находим максимальную прибыль по всем задачам
            T max_profit = std::numeric_limits<T>::lowest();

            for (int j = 0; j < m; j++)
            {
                T profit = alpha[i][j] - prices[j];
                max_profit = max(max_profit, profit);
            }

            // Проверяем, "счастлив" ли робот
            if (current_profit < max_profit - epsilon)
            {
                all_happy = false; // Робот несчастлив, продолжаем
                // Находим задачу с максимальной прибылью
                std::vector<T> profits(m);

                for (int j = 0; j < m; j++) {
                    profits[j] = alpha[i][j] - prices[j];
                }

                auto [v_i, t_i] = find_max(profits); // v_i - максимальная прибыль, t_i - новая задача
                // Находим вторую по величине прибыль

                profits[t_i] = numeric_limits<T>::lowest(); // Убираем максимум
                auto [w_i, _] = find_max(profits); // w_i - вторая по величине прибыль

                // Находим робота, который сейчас назначен на задачу t_i
                int current_owner = task_to_robot[t_i];
                if (current_owner != -1) {
                    // Старому владельцу убираем задачу
                    assignment[current_owner] = assignment[i]; // Даем старую задачу (может быть -1)
                    if (assignment[i] != -1) {
                        task_to_robot[assignment[i]] = current_owner;
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
    for (int i = 0; i < n; i++)
    {
        if (assignment[i] != -1) {
            total_utility += alpha[i][assignment[i]];
        }
    }
    return total_utility;
}



template<typename T>
void AuctionAlgo<T>::Update(std::size_t n, std::size_t m, std::vector<std::vector<T> > &D, std::vector<int> &N)
{
    if (D.size() != n || (n > 0 && D[0].size() != m) || N.size() != m)
    {
        throw std::invalid_argument("Invalid dimensions or sizes for D or N in Update");
    }

    num_rows = n;
    num_cols = m;
    this->D = D;
    N_max = N;
    used_rows.assign(n, false);
    used_cols.assign(m, false);
    answer = T(0);
}


template<typename T>
template<typename F>
bool AuctionAlgo<T>::notNull(const std::vector<F>& v) const noexcept
{
    return std::any_of(v.begin(), v.end(), [](const F& val) { return val > 0; });
}


template<typename T>
bool AuctionAlgo<T>::isUsedRows() const noexcept
{
    return std::all_of(used_rows.begin(), used_rows.end(), [](bool used) { return used; });
}


template<typename T>
bool AuctionAlgo<T>::isUsedCols() const noexcept
{
    return std::all_of(used_cols.begin(), used_cols.end(), [](bool used) { return used; });
}

template<typename T>
std::pair<T, int> AuctionAlgo<T>::find_max(const std::vector<T> &arr)
{
    T max_val = -std::numeric_limits<T>::max();
    int max_idx = -1;
    for (size_t i = 0; i < arr.size(); i++) {
        if (arr[i] > max_val) {
            max_val = arr[i];
            max_idx = i;
        }
    }
    return {max_val, max_idx};
}


template<typename T>
void AuctionAlgo<T>::printMatrix(const std::vector<std::vector<T>>& matrix) const
{
    std::cout << '\n';
    for (std::size_t i = 0; i < matrix.size(); ++i)
    {
        for (std::size_t j = 0; j < matrix[0].size(); ++j)
        {
            if (used_cols[j] || used_rows[i])
            {
                std::cout << 0 << ' ';
            } else
            {
                std::cout << matrix[i][j] << ' ';
            }
        }
        std::cout << '\n';
    }
    std::cout << '\n';
}


#endif // AUCTIONALGO_HPP

