#ifndef COI_H
#define COI_H

#include <iostream>
#include <vector>
#include <string>
#include <stdlib.h>

using namespace std;

#ifndef DEBUG
#define DEBUG 0
#endif

#define eps 1e-3

template <typename T>
class COI_3_7
{
public:
    COI_3_7() = default;
    COI_3_7(int n, int m, vector<vector<T>>& D, vector<int>& N);
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
COI_3_7<T>::COI_3_7(int n, int m, vector<vector<T>>& D, vector<int>& N)
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
T COI_3_7<T>::Start()
{
    while(notNull(N_max) && !isUsedRows())
    {
        for(int i = 0; i < num_rows; i++)
        {
            pair<T, int> max_d = {0, 0};

            if(used_rows[i]) continue;

            for(int j = 0; j < num_cols; j++)
            {
                if(used_cols[j]) continue;

                if(D[i][j] > max_d.first)
                {
                    max_d.first = D[i][j];
                    max_d.second = j;
                }
            }

            bool is_optimal = true;

            for(int j = 0; j < num_rows; j++)
            {
                if(j != i && !used_rows[j])
                {
                    if(D[j][max_d.second] > max_d.first)
                    {
                        is_optimal = false;
                        break;
                    }
                    else if(abs(D[j][max_d.second] - max_d.first) < eps)
                    {
                        if(j > i) continue;

                        is_optimal = false;
                        break;
                    }
                }
            }

            if(is_optimal)
            {
                used_rows[i] = true;

                if(--N_max[max_d.second] == 0)
                {
                    used_cols[max_d.second] = true;
                }

                answer += max_d.first;

#if DEBUG
                cout << "row: " << i + 1 << " col: " << max_d.second + 1 << " value: " << max_d.first << endl;
                printMatrix(D);
#endif
            }
        }
    }

    return answer;
}

template<typename T>
T COI_3_7<T>::Update(int n, int m, vector<vector<T> > &D, vector<int> &N)
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
bool COI_3_7<T>::notNull(const vector<F>& v)
{
    int n = v.size();

    for(int i = 0; i < n; i++)
    {
        if(v[i] > 0) return true;
    }
    return false;
}


template<typename T>
bool COI_3_7<T>::isUsedRows()
{
    for(int i = 0; i < num_rows; i++)
    {
        if(!used_rows[i]) return false;
    }
    return true;
}


template<typename T>
bool COI_3_7<T>::isUsedCols()
{
    for(int i = 0; i < num_cols; i++)
    {
        if(!used_cols[i]) return false;
    }
    return true;
}


template<typename T>
void COI_3_7<T>::printMatrix(const vector<vector<T> > &matrix)
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


#endif // COI_H
