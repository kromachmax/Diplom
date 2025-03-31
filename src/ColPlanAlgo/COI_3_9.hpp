#ifndef COI_3_9_HPP
#define COI_3_9_HPP

#include <iostream>
#include <vector>
#include <string>
#include <stdlib.h>
#include <array>
#include <tuple>
#include <cstddef>


template <typename T>
class COI_3_9
{
public:
    COI_3_9() noexcept = default;
    explicit COI_3_9(std::size_t n, std::size_t m, std::vector<std::vector<T>>& D, std::vector<int>& N);
    void Update(std::size_t n, std::size_t m, std::vector<std::vector<T>>& D, std::vector<int>& N);
    [[nodiscard]] T Start();

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

    T answer = T{};
};


template<typename T>
COI_3_9<T>::COI_3_9(std::size_t n, std::size_t m, std::vector<std::vector<T>>& D, std::vector<int>& N)
    : num_rows(n), num_cols(m), D(D), N_max(N), used_rows(n, false), used_cols(m, false)
{
    if (D.size() != n || (n > 0 && D[0].size() != m) || N.size() != m)
    {
        throw std::invalid_argument("Invalid dimensions or sizes for D or N");
    }
}


template<typename T>
T COI_3_9<T>::Start()
{
    for(std::size_t i = 0; i < N_max.size(); i++)
    {
        if(!N_max[i])
            used_cols[i] = true;
    }

    while(notNull(N_max) && !isUsedRows())
    {
        std::vector<std::pair<T, std::size_t>> first_max(num_rows, {T(0), 0});
        std::vector<std::pair<T, std::size_t>> second_max(num_rows, {T(0), 0});

        for(std::size_t i = 0; i < num_rows; i++)
        {
            if(used_rows[i]) continue;

            for(std::size_t j = 0; j < num_cols; j++)
            {
                if(D[i][j] > first_max[i].first && !used_cols[j])
                {
                    first_max[i].first = D[i][j];
                    first_max[i].second = j;
                }
            }

            for(std::size_t j = 0; j < num_cols; j++)
            {
                if(D[i][j] > second_max[i].first && !used_cols[j] && j != first_max[i].second)
                {
                    second_max[i].first = D[i][j];
                    second_max[i].second = j;
                }
            }
        }

        std::vector<T> result(num_rows, 0);
        std::vector<T> delta(num_rows, 0);
        std::pair<T, std::size_t> res = {T(0), 0};

        for(std::size_t i = 0; i < num_rows; i++)
        {
            result[i] = 2 * first_max[i].first - second_max[i].first;
            delta[i] = first_max[i].first - second_max[i].first;
        }

        for(std::size_t i = 0; i < num_rows; i++)
        {
            if(res.first < result[i])
            {
                res.first = result[i];
                res.second = i;
            }
            else if(abs(res.first - result[i]) < eps)
            {
                if(delta[res.second] < delta[i])
                {
                    res.first = result[i];
                    res.second = i;
                }
            }
        }

        used_rows[res.second] = true;

        if(--N_max[first_max[res.second].second] == 0)
        {
            used_cols[first_max[res.second].second] = true;
        }

        answer += first_max[res.second].first;
    }

    return answer;
}

template<typename T>
void COI_3_9<T>::Update(std::size_t n, std::size_t m, std::vector<std::vector<T> > &D, std::vector<int> &N)
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
bool COI_3_9<T>::notNull(const std::vector<F>& v) const noexcept
{
    return std::any_of(v.begin(), v.end(), [](const F& val) { return val > 0; });
}


template<typename T>
bool COI_3_9<T>::isUsedRows() const noexcept
{
    return std::all_of(used_rows.begin(), used_rows.end(), [](bool used) { return used; });
}


template<typename T>
bool COI_3_9<T>::isUsedCols() const noexcept
{
    return std::all_of(used_cols.begin(), used_cols.end(), [](bool used) { return used; });
}


template<typename T>
void COI_3_9<T>::printMatrix(const std::vector<std::vector<T>>& matrix) const
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



#endif // COI_3_9_HPP
