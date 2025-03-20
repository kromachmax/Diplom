#ifndef COI_3_6_HPP
#define COI_3_6_HPP

#include <iostream>
#include <vector>
#include <string>
#include <stdlib.h>

#ifndef DEBUG
#define DEBUG 0
#endif

#define eps 1e-3

template <typename T>
class COI_3_6
{
public:
    COI_3_6() noexcept = default;
    explicit COI_3_6(std::size_t n, std::size_t m, std::vector<std::vector<T>>& D, std::vector<int>& N);
    void Update(std::size_t n, std::size_t m, std::vector<std::vector<T>>& D, std::vector<int>& N);
    [[nodiscard]] T Start();

private:
    [[nodiscard]] bool isUsedRows() const noexcept;
    [[nodiscard]] bool isUsedCols() const noexcept;
    template<typename F>
    [[nodiscard]]bool notNull(const std::vector<F>& v) const noexcept;

    void printMatrix(const std::vector<std::vector<T>>& matrix) const;

private:
    std::size_t num_rows = 0;
    std::size_t num_cols = 0;

    std::vector<std::vector<T>> D;
    std::vector<int> N_max;
    std::vector<bool> used_rows;
    std::vector<bool> used_cols;

    T answer = T(0);
};


template<typename T>
COI_3_6<T>::COI_3_6(std::size_t n, std::size_t m, std::vector<std::vector<T>>& D, std::vector<int>& N)
    : num_rows(n), num_cols(m), D(D), N_max(N), used_rows(n, false), used_cols(m, false)
{
    if (D.size() != n || (n > 0 && D[0].size() != m) || N.size() != m)
    {
        throw std::invalid_argument("Invalid dimensions or sizes for D or N");
    }
}


template<typename T>
T COI_3_6<T>::Start()
{

    std::vector<int> initial_plan;

    for(std::size_t i = 0; i < std::min(num_rows, num_cols); i++)
    {
    }


    return answer;
}


template<typename T>
void COI_3_6<T>::Update(std::size_t n, std::size_t m, std::vector<std::vector<T> > &D, std::vector<int> &N)
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
bool COI_3_6<T>::notNull(const std::vector<F>& v) const noexcept
{
    return std::any_of(v.begin(), v.end(), [](const F& val) { return val > 0; });
}


template<typename T>
bool COI_3_6<T>::isUsedRows() const noexcept
{
    return std::all_of(used_rows.begin(), used_rows.end(), [](bool used) { return used; });
}


template<typename T>
bool COI_3_6<T>::isUsedCols() const noexcept
{
    return std::all_of(used_cols.begin(), used_cols.end(), [](bool used) { return used; });
}


template<typename T>
void COI_3_6<T>::printMatrix(const std::vector<std::vector<T>>& matrix) const
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

#endif // COI_3_6_HPP
