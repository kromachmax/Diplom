#ifndef COI_3_1_HPP
#define COI_3_1_HPP

#include <iostream>
#include <vector>
#include <string>
#include <stdlib.h>
#include <set>
#include <map>
#include <limits>

//15 13 8
//21 19 5
//20 21 14


template <typename T>
class COI_3_1
{
public:
    COI_3_1() noexcept = default;
    explicit COI_3_1(std::size_t n, std::size_t m, std::vector<std::vector<T>>& D, std::vector<int>& N);
    void Update(std::size_t n, std::size_t m, std::vector<std::vector<T>>& D, std::vector<int>& N);
    [[nodiscard]] T Start();

private:
    [[nodiscard]] bool isUsedRows() const noexcept;
    [[nodiscard]] bool isUsedCols() const noexcept;
    template<typename F>
    [[nodiscard]]bool notNull(const std::vector<F>& v) const noexcept;

    void Step_0();
    bool Step_1_2();
    bool Step_3_4();

    void printMatrix(const std::vector<std::vector<T>>& matrix) const;

private:

    const double eps = 1e-5;

    std::size_t num_rows = 0;
    std::size_t num_cols = 0;

    std::vector<std::vector<T>> D;
    std::vector<std::vector<T>> cost;
    std::vector<int> N_max;
    std::vector<bool> used_rows;
    std::vector<bool> used_cols;

    std::map<std::size_t, std::size_t> distribution_plan;

    T answer = T(0);
};


template<typename T>
COI_3_1<T>::COI_3_1(std::size_t n, std::size_t m, std::vector<std::vector<T>>& cost, std::vector<int>& N)
    : num_rows(n), num_cols(m), cost(cost), N_max(N), used_rows(n, false), used_cols(m, false)
{
    if (cost.size() != n || (n > 0 && cost[0].size() != m) || N.size() != m)
    {
        throw std::invalid_argument("Invalid dimensions or sizes for D or N");
    }

    // Find maximum value in matrix
    T max_val = cost[0][0];
    for (std::size_t i = 0; i < num_rows; ++i)
    {
        for (std::size_t j = 0; j < num_cols; ++j)
        {
            if (cost[i][j] > max_val) {
                max_val = cost[i][j];
            }
        }
    }

    // Create cost matrix (max_val - D[i][j])
    D = std::vector<std::vector<T>>(num_rows, std::vector<T>(num_cols));
    for (std::size_t i = 0; i < num_rows; ++i)
    {
        for (std::size_t j = 0; j < num_cols; ++j)
        {
            D[i][j] = max_val - cost[i][j];
        }
    }

#ifdef DEBUG
    std::cout << "Cost: " << std::endl;
    printMatrix(cost);

    std::cout << "D: " << std::endl;
    printMatrix(D);
#endif

}


template<typename T>
T COI_3_1<T>::Start()
{
    Step_0();

    bool step_1_2 = true;
    bool step_3_4 = true;

    while(step_1_2 || step_3_4)
    {
        step_1_2 = false;
        step_3_4 = false;

        if(Step_1_2())
        {
            step_1_2 = true;
            continue;
        }
        else
        {
            while(true)
            {
                if(!Step_3_4()) {
                    break;
                }
                else step_3_4 = true;
            }
        }
    }

    for(auto& elem : distribution_plan)
    {
        answer += cost[elem.first][elem.second];
    }
    return answer;
}


template<typename T>
void COI_3_1<T>::Update(std::size_t n, std::size_t m, std::vector<std::vector<T> > &cost, std::vector<int> &N)
{
    if (cost.size() != n || (n > 0 && cost[0].size() != m) || N.size() != m)
    {
        throw std::invalid_argument("Invalid dimensions or sizes for D or N in Update");
    }

    num_rows = n;
    num_cols = m;
    this->cost = cost;
    N_max = N;
    used_rows.assign(n, false);
    used_cols.assign(m, false);
    answer = T(0);

    T max_val = cost[0][0];
    for (std::size_t i = 0; i < num_rows; ++i)
    {
        for (std::size_t j = 0; j < num_cols; ++j)
        {
            if (cost[i][j] > max_val) {
                max_val = cost[i][j];
            }
        }
    }

    D = std::vector<std::vector<T>>(num_rows, std::vector<T>(num_cols));
    for (std::size_t i = 0; i < num_rows; ++i)
    {
        for (std::size_t j = 0; j < num_cols; ++j)
        {
            D[i][j] = max_val - cost[i][j];
        }
    }

#ifdef DEBUG
    std::cout << "Cost: " << std::endl;
    printMatrix(cost);

    std::cout << "D: " << std::endl;
    printMatrix(D);
#endif

}


template<typename T>
template<typename F>
bool COI_3_1<T>::notNull(const std::vector<F>& v) const noexcept
{
    return std::any_of(v.begin(), v.end(), [](const F& val) { return val > 0; });
}


template<typename T>
bool COI_3_1<T>::isUsedRows() const noexcept
{
    return std::all_of(used_rows.begin(), used_rows.end(), [](bool used) { return used; });
}


template<typename T>
bool COI_3_1<T>::isUsedCols() const noexcept
{
    return std::all_of(used_cols.begin(), used_cols.end(), [](bool used) { return used; });
}


template<typename T>
void COI_3_1<T>::Step_0()
{
    for(std::size_t i = 0; i < num_rows; i++)
    {
        std::pair<T, std::size_t> max = { std::numeric_limits<T>::max(), 0 };

        for(std::size_t j = 0; j < num_cols; j++)
        {
            if(D[i][j] < max.first && !used_cols[j])
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
bool COI_3_1<T>::Step_1_2()
{
#ifdef DEBUG
    std::cout << "Step_1_2: " << "\n\n";

    std::cout << "Matrix: " << std::endl;

    printMatrix(D);

    std::cout << "Plan: " << std::endl;

    for(auto& el : distribution_plan)
    {
        std::cout << "row: " << el.first << " col: " << el.second << std::endl << std::endl;
    }
#endif

    std::vector<std::pair<T, std::pair<std::size_t, std::size_t>>> swap_pairs;
    std::set<std::size_t> prohibited_rows;

    for(auto& first_value : distribution_plan)
    {
        T delta = T(0);
        std::size_t row_fi = first_value.first;
        std::size_t col_fi = first_value.second;

        for(auto& second_value : distribution_plan)
        {
            std::size_t row_se = second_value.first;
            std::size_t col_se = second_value.second;

            if(row_se > row_fi)
            {
                delta = D[row_fi][col_fi] + D[row_se][col_se] - D[row_fi][col_se] - D[row_se][col_fi];

                if(delta > 0)
                    swap_pairs.push_back({delta, {row_fi, row_se}});
            }
        }
    }

    if(!swap_pairs.size())
        return false;

    std::sort(swap_pairs.begin(), swap_pairs.end(),
              [](const auto& a, const auto& b) {
        return a.first > b.first;
    });

#ifdef DEBUG

    for(auto& pair : swap_pairs)
    {
        std::cout << "row1: " << pair.second.first << " col1: " << distribution_plan[pair.second.first] << std::endl;
        std::cout << "row2: " << pair.second.second << " col2: " << distribution_plan[pair.second.second] << std::endl;
        std::cout << "delta: " << pair.first << "\n";
    }
    std::cout << std::endl;
#endif

    for(auto& swap_pair : swap_pairs)
    {
        std::size_t row1 = swap_pair.second.first;
        std::size_t row2 = swap_pair.second.second;

        if(!prohibited_rows.count(row1) && !prohibited_rows.count(row2))
        {
            T col = distribution_plan[row1];

            distribution_plan[row1] = distribution_plan[row2];
            distribution_plan[row2] = col;

            prohibited_rows.insert(row1);
            prohibited_rows.insert(row2);
        }
    }

    return true;
}


template<typename T>
bool COI_3_1<T>::Step_3_4()
{

#ifdef DEBUG
    std::cout << "Step_3_4: " << "\n\n";

    std::cout << "Matrix: " << std::endl;

    printMatrix(D);

    std::cout << "Plan: " << std::endl;

    for(auto& el : distribution_plan)
    {
        std::cout << "row: " << el.first << " col: " << el.second << std::endl << std::endl;
    }
#endif

    std::vector<std::pair<T, std::pair<std::size_t, std::size_t>>> swap_pairs;

    for(auto& first_value : distribution_plan)
    {
        std::size_t row_fi = first_value.first;
        std::size_t col_fi = first_value.second;

        for(auto& second_value : distribution_plan)
        {
            std::size_t row_se = second_value.first;
            std::size_t col_se = second_value.second;

            if(row_se != row_fi)
            {
                T delta = D[row_fi][col_fi] + D[row_se][col_se] - D[row_fi][col_se];
                if(delta > 0)
                    swap_pairs.push_back({delta, {row_fi, row_se}});
            }
        }
    }

    if(!swap_pairs.size())
        return false;

    std::sort(swap_pairs.begin(), swap_pairs.end(),
              [](const auto& a, const auto& b) {
        return a.first > b.first;
    });

#ifdef DEBUG

    for(auto& pair : swap_pairs)
    {
        std::cout << "row1: " << pair.second.first << " col1: " << distribution_plan[pair.second.first] << std::endl;
        std::cout << "row2: " << pair.second.second << " col2: " << distribution_plan[pair.second.second] << std::endl;
        std::cout << "delta: " << pair.first << "\n";
    }
    std::cout << std::endl;
#endif

#ifdef DEBUG
    int k = 1;
#endif

    for(auto& swap_pair : swap_pairs)
    {
        std::map<std::size_t, std::size_t> new_distribution_plan = distribution_plan;
        std::set<std::size_t> prohibited_robots;

        T Y = 0.0;

        std::size_t start_robot_row = swap_pair.second.first;
        std::size_t cur_robot_row = swap_pair.second.second;
        std::size_t cur_robot_col = distribution_plan[cur_robot_row];

        Y += D[cur_robot_row][cur_robot_col] - D[start_robot_row][cur_robot_col];
        new_distribution_plan[start_robot_row] = distribution_plan[cur_robot_row];
        prohibited_robots.insert(cur_robot_row);

#ifdef DEBUG
        std::cout << "Iteration: " << k << "\n\n";

        std::cout << "First elements: " << std::endl;
        std::cout << "row: " << cur_robot_row << " col: " << cur_robot_col << std::endl;
#endif

        do {
            std::pair<T, std::size_t> next_robot = {-20000, 0};

            for(std::size_t i = 0; i < num_rows; ++i)
            {
                if(prohibited_robots.count(i) || i == cur_robot_row)
                    continue;

                std::size_t next_row = i;
                std::size_t next_col = distribution_plan[i];

                T delta = D[cur_robot_row][cur_robot_col] + D[next_row][next_col] - D[cur_robot_row][next_col];
                if(delta > next_robot.first)
                {
                    next_robot = {delta, next_row};
                }
            }


            new_distribution_plan[cur_robot_row] = distribution_plan[next_robot.second];
            Y += next_robot.first - D[cur_robot_row][cur_robot_col];

            cur_robot_row = next_robot.second;
            cur_robot_col = distribution_plan[next_robot.second];

#ifdef DEBUG
            std::cout << "Delta: " << next_robot.first << std::endl;
            std::cout << "New elements: " << std::endl;
            std::cout << "row1: " << cur_robot_row << " row2: " << cur_robot_col << std::endl;
#endif

            prohibited_robots.insert(cur_robot_row);
        }
        while(start_robot_row != cur_robot_row);

#ifdef DEBUG
        std::cout << "Y: " << Y << "\n\n";
#endif

        if(Y > 0)
        {
            distribution_plan = new_distribution_plan;
            return true;
        }

#ifdef DEBUG
        k++;
#endif

    }

    return false;
}


template<typename T>
void COI_3_1<T>::printMatrix(const std::vector<std::vector<T>>& matrix) const
{
    for (std::size_t i = 0; i < matrix.size(); ++i)
    {
        for (std::size_t j = 0; j < matrix[0].size(); ++j)
        {
            std::cout << matrix[i][j] << ' ';
        }
        std::cout << '\n';
    }
    std::cout << '\n';
}


#endif // COI_3_1_HPP
