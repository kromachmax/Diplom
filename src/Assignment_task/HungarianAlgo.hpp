#ifndef HUNGARIANALGO_HPP
#define HUNGARIANALGO_HPP

#include <iostream>
#include <vector>
#include <stdlib.h>
#include <map>
#include <limits>
#include <algorithm>

template <typename T>
class HungarianAlgo
{
public:
    HungarianAlgo() noexcept = default;
    explicit HungarianAlgo(std::size_t n, std::size_t m, std::vector<std::vector<T>>& D, std::vector<int>& N);
    void Update(std::size_t n, std::size_t m, std::vector<std::vector<T>>& D, std::vector<int>& N);
    [[nodiscard]] T Start(std::vector<int>&);

private:
    [[nodiscard]] bool isUsedRows() const noexcept;
    [[nodiscard]] bool isUsedCols() const noexcept;
    template<typename F>
    [[nodiscard]]bool notNull(const std::vector<F>& v) const noexcept;

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
HungarianAlgo<T>::HungarianAlgo(std::size_t n, std::size_t m, std::vector<std::vector<T>>& D, std::vector<int>& N)
    : num_rows(n), num_cols(m), D(D), N_max(N), used_rows(n, false), used_cols(m, false)
{
    if (D.size() != n || (n > 0 && D[0].size() != m) || N.size() != m)
    {
        throw std::invalid_argument("Invalid dimensions or sizes for D or N");
    }
}

template<typename T>
T HungarianAlgo<T>::Start(std::vector<int>& assignment)
{
    if (num_rows == 0 || num_cols == 0) {
        return T(0);
    }

    // Find maximum value in matrix
    T max_val = D[0][0];
    for (std::size_t i = 0; i < num_rows; ++i) {
        for (std::size_t j = 0; j < num_cols; ++j) {
            if (D[i][j] > max_val) {
                max_val = D[i][j];
            }
        }
    }

    // Create cost matrix (max_val - D[i][j])
    std::vector<std::vector<T>> cost(num_rows, std::vector<T>(num_cols));
    for (std::size_t i = 0; i < num_rows; ++i) {
        for (std::size_t j = 0; j < num_cols; ++j) {
            cost[i][j] = max_val - D[i][j];
        }
    }

    // Hungarian algorithm implementation
    const T INF = std::numeric_limits<T>::max();
    std::vector<T> u(num_rows + 1, 0);
    std::vector<T> v(num_cols + 1, 0);
    std::vector<std::size_t> p(num_cols + 1, 0);
    std::vector<std::size_t> way(num_cols + 1, 0);

    for (std::size_t i = 1; i <= num_rows; ++i) {
        p[0] = i;
        std::size_t j0 = 0;
        std::vector<T> minv(num_cols + 1, INF);
        std::vector<bool> used(num_cols + 1, false);

        do {
            used[j0] = true;
            std::size_t i0 = p[j0];
            T delta = INF;
            std::size_t j1 = 0;

            for (std::size_t j = 1; j <= num_cols; ++j) {
                if (!used[j]) {
                    T cur = cost[i0-1][j-1] - u[i0] - v[j];
                    if (cur < minv[j]) {
                        minv[j] = cur;
                        way[j] = j0;
                    }
                    if (minv[j] < delta) {
                        delta = minv[j];
                        j1 = j;
                    }
                }
            }

            for (std::size_t j = 0; j <= num_cols; ++j) {
                if (used[j]) {
                    u[p[j]] += delta;
                    v[j] -= delta;
                } else {
                    minv[j] -= delta;
                }
            }
            j0 = j1;
        } while (p[j0] != 0);

        do {
            std::size_t j1 = way[j0];
            p[j0] = p[j1];
            j0 = j1;
        } while (j0 != 0);
    }

    // Calculate total profit
    T total_profit = 0;
    assignment.resize(num_rows, -1);

    for (std::size_t j = 1; j <= num_cols; ++j) {
        if (p[j] > 0) {
            total_profit += D[p[j]-1][j-1];
            assignment[p[j]-1] = j - 1;
        }
    }

    // Вывод результатов
    std::cout << "Назначения (робот -> задача): ";
    for (int i = 0; i < num_rows; ++i) {
        std::cout << assignment[i] << " ";
    }

    answer = total_profit;
    return answer;
}

template<typename T>
void HungarianAlgo<T>::Update(std::size_t n, std::size_t m, std::vector<std::vector<T> > &D, std::vector<int> &N)
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
bool HungarianAlgo<T>::notNull(const std::vector<F>& v) const noexcept
{
    return std::any_of(v.begin(), v.end(), [](const F& val) { return val > 0; });
}


template<typename T>
bool HungarianAlgo<T>::isUsedRows() const noexcept
{
    return std::all_of(used_rows.begin(), used_rows.end(), [](bool used) { return used; });
}


template<typename T>
bool HungarianAlgo<T>::isUsedCols() const noexcept
{
    return std::all_of(used_cols.begin(), used_cols.end(), [](bool used) { return used; });
}


template<typename T>
void HungarianAlgo<T>::printMatrix(const std::vector<std::vector<T>>& matrix) const
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


#endif // HUNGARIANALGO_HPP

