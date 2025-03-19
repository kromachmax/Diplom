#ifndef COI_3_9_HPP
#define COI_3_9_HPP

#include <iostream>
#include <vector>
#include <string>
#include <stdlib.h>
#include <array>
#include <tuple>

using namespace std;

#ifndef DEBUG
#define DEBUG 0
#endif

#define eps 1e-3

template <typename T>
class COI_3_9
{
public:
    COI_3_9() = default;
    COI_3_9(int n, int m, vector<vector<T>>& D, vector<int>& N);
    T Start();
    T Update(int n, int m, vector<vector<T>>& D, vector<int>& N);

private:
    bool isUsedRows();
    bool isUsedCols();
    template<typename F>
    bool notNull(const vector<F>& v);

    void printMatrix(const vector<vector<T>>& matrix);

private:
    int num_rows;
    int num_cols;

    vector<vector<T>> D;
    vector<int> N_max;
    vector<bool> used_rows;
    vector<bool> used_cols;

    T answer;
};


template<typename T>
COI_3_9<T>::COI_3_9(int n, int m, vector<vector<T>>& D, vector<int>& N)
{
    num_rows = n;
    num_cols = m;

    this->D = D;
    this->N_max = N;

    answer = T(0);

    used_rows = vector<bool>(n, false);
    used_cols = vector<bool>(m, false);
}


template<typename T>
T COI_3_9<T>::Start()
{
    while(notNull(N_max) && !isUsedRows())
    {
        vector<pair<T, int>> first_max(num_rows, {T(0), 0});
        vector<pair<T, int>> second_max(num_rows, {T(0), 0});

        for(int i = 0; i < num_rows; i++)
        {
            if(used_rows[i]) continue;

            for(int j = 0; j < num_cols; j++)
            {
                if(D[i][j] > first_max[i].first && !used_cols[j])
                {
                    first_max[i].first = D[i][j];
                    first_max[i].second = j;
                }
            }

            for(int j = 0; j < num_cols; j++)
            {
                if(D[i][j] > second_max[i].first && !used_cols[j] && j != first_max[i].second)
                {
                    second_max[i].first = D[i][j];
                    second_max[i].second = j;
                }
            }
        }

        vector<T> result(num_rows, 0);
        vector<T> delta(num_rows, 0);
        pair<T, int> res = {T(0), 0};

        for(int i = 0; i < num_rows; i++)
        {
            result[i] = 2 * first_max[i].first - second_max[i].first;
            delta[i] = first_max[i].first - second_max[i].first;
        }

        for(int i = 0; i < num_rows; i++)
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

#if DEBUG
        cout << "row: " << res.second + 1 << " col: " << first_max[res.second].second + 1 << " value: " << first_max[res.second].first << endl;
        printMatrix(D);
#endif

    }

    return answer;
}

template<typename T>
T COI_3_9<T>::Update(int n, int m, vector<vector<T> > &D, vector<int> &N)
{
    num_rows = n;
    num_cols = m;

    this->D = D;
    this->N_max = N;

    answer = T(0);

    used_rows = vector<bool>(n, false);
    used_cols = vector<bool>(m, false);
}


template<typename T>
template<typename F>
bool COI_3_9<T>::notNull(const vector<F>& v)
{
    int n = v.size();

    for(int i = 0; i < n; i++)
    {
        if(v[i] > 0) return true;
    }
    return false;
}


template<typename T>
bool COI_3_9<T>::isUsedRows()
{
    for(int i = 0; i < num_rows; i++)
    {
        if(!used_rows[i]) return false;
    }
    return true;
}


template<typename T>
bool COI_3_9<T>::isUsedCols()
{
    for(int i = 0; i < num_cols; i++)
    {
        if(!used_cols[i]) return false;
    }
    return true;
}


template<typename T>
void COI_3_9<T>::printMatrix(const vector<vector<T> > &matrix)
{
    int n = matrix.size();
    int m = matrix[0].size();

    cout << endl;

    for(int i = 0; i < n; i++)
    {
        for(int j = 0; j < m; j++)
        {
            if(used_cols[j] || used_rows[i])
                cout << 0 << ' ';
            else
                cout << matrix[i][j] << ' ';
        }

        cout << endl;
    }

    cout << endl;
}

#endif // COI_3_9_HPP
