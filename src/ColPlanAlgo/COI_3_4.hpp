#ifndef COI_3_4_HPP
#define COI_3_4_HPP

#include <iostream>
#include <vector>
#include <string>
#include <stdlib.h>
#include <set>
#include <map>
#include <unordered_map>
#include <unordered_set>

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

    void Step_0();
    void Step_1_2();
    [[nodiscard]] bool Step_3_4();
    [[nodiscard]] bool Step_5_6();

    void printMatrix(const std::vector<std::vector<T>>& matrix) const;

private:
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
    Step_0();

    while(true)
    {
        Step_1_2();

        if(Step_3_4())
            continue;

        if(Step_5_6())
            break;
    }

    for(auto& value : distribution_plan)
    {
        std::size_t row = value.first;
        std::size_t col = value.second;

        answer += D[row][col];
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
void COI_3_6<T>::Step_0()
{
    for(std::size_t i = 0; i < num_cols; i++)
    {
        std::pair<T, std::size_t> max = { T(0), 0 };

        for(std::size_t j = 0; j < num_cols; j++)
        {
            if(D[i][j] > max && !used_cols[j])
            {
                max.first = D[i][j];
                max.second = j;
            }
        }

        used_cols[max.second] = true;
        used_rows[i] = true;
        distribution_plan[i] = max.second;
    }
}

template<typename T>
void COI_3_6<T>::Step_1_2()
{

    std::multimap<T, std::pair<std::size_t, std::size_t>> swap_pairs;
    std::set<std::size_t> prohibited_robots;

    for(std::size_t i = 0; i < num_rows; i++)
    {
        if(used_rows[i])
            continue;

        for(auto& plan_value : distribution_plan)
        {
            std::size_t row = plan_value.first;
            std::size_t col = plan_value.second;

            T delta = D[row][col] - D[i][col];

            if(delta < 0)
                swap_pairs.insert(delta, {row, i});
        }
    }

    for(auto& swap_pair : swap_pairs)
    {
        std::size_t row1 = swap_pair.second.first;
        std::size_t row2 = swap_pair.second.second;

        if(!prohibited_robots.count(row1) && !prohibited_robots.count(row2))
        {
            std::swap(used_rows[row1], used_rows[row2]);
            distribution_plan[row2] = distribution_plan[row1];

            distribution_plan.erase(row1);
            prohibited_robots.insert(row1);
            prohibited_robots.insert(row2);
        }
    }
}

template<typename T>
bool COI_3_6<T>::Step_3_4()
{
    std::multimap<T, std::pair<std::size_t, std::size_t>> swap_pairs;
    std::set<std::size_t> prohibited_rows;

    for(auto& first_value : distribution_plan)
    {
        T delta = T(0);
        std::size_t row_fi = first_value.first;
        std::size_t col_fi = first_value.second;

        for(auto& second_value : distribution_plan)
        {
            std::size_t row_se = seconf_value.first;
            std::size_t col_se = second_value.second;

            if(row_se != row_fi)
            {
                delta = D[row_fi][col_fi] + D[row_se][col_se] - D[row_fi][col_se] - D[row_se][col_fi];

                if(delta < 0)
                    swap_pairs.insert(delta, {row_fi, row_se});
            }
        }
    }

    if(!swap_pairs.size())
        return false;

    for(auto& swap_pair : swap_pairs)
    {
        std::size_t row1 = swap_pair.second.first;
        std::size_t row2 = swap_pair.second.second;

        if(!prohibited_rows.count(row1) && !prohibited_rows.count(row2))
        {
            T col = distribution_plan[row1];

            distribution_plan[row1] = distribution_plan[row2];
            distribution_plan[row2] = col;
        }
    }

    return true;
}

template<typename T>
bool COI_3_6<T>::Step_5_6()
{
    while(true)
    {
        std::map<std::size_t, std::set<std::pair<T, std::size_t>>> swap_pairs;

        for(auto& first_value : distribution_plan)
        {
            T delta = T(0);
            std::size_t row_fi = first_value.first;
            std::size_t col_fi = first_value.second;

            for(auto& second_value : distribution_plan)
            {
                std::size_t row_se = seconf_value.first;
                std::size_t col_se = second_value.second;

                if(row_se != row_fi)
                {
                    delta = D[row_fi][col_fi] + D[row_se][col_se] - D[row_fi][col_se];

                    if(delta < 0)
                        swap_pairs[row_fi].insert({delta, row_se});
                }
            }
        }

        if(!swap_pairs.size())
            return true;


        T Y = T(0);
        std::size_t start_robot = swap_pairs.begin()->first;
        std::size_t current_robot = start_robot;
        std::size_t end_robot = -1;

        std::map<std::size_t, std::size_t> new_distribution_plan = distribution_plan;

        while(start_robot != end_robot)
        {
            std::size_t old_row  = current_robot;
            std::size_t old_col  = distribution_planp[old_row];
            std::size_t new_row  = swap_pairs[old_row].begin()->first;
            std::size_t new_col  = distribution_plan[new_row];

            Y += D[old_row][old_col] - T[new_row][new_col];
            new_distribution_plan[old_row] = new_row;

            current_robot = new_row;
        }

        if(-Y > 0)
        {
            distribution_plan = new_distribution_plan;
            continue;
        }
    }
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


#endif // COI_3_4_HPP
