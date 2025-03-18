#include <iostream>
#include <vector>
#include <string>

using namespace std;

#define INF 1e9

int main(void)
{
    int n, m;
    cin >> n >> m;
    vector<int> u(n+1, 0), v(m+1, 0), p(m+1, 0), way(m+1, 0);
    vector<vector<int>> a(n + 1, vector<int>(m + 1));

    for(int i = 1; i < n + 1; i++)
    {
        for(int j = 1; j < m + 1; j++)
        {
            cin >> a[i][j];
        }
    }

    for (int i=1; i<=n; ++i) {
        p[0] = i;
        int j0 = 0;
        vector<int> minv (m+1, INF);
        vector<char> used (m+1, false);
        do {
            used[j0] = true;
            int i0 = p[j0],  delta = INF,  j1;
            for (int j=1; j<=m; ++j)
                if (!used[j]) {
                    int cur = a[i0][j]-u[i0]-v[j];
                    if (cur < minv[j])
                        minv[j] = cur,  way[j] = j0;
                    if (minv[j] < delta)
                        delta = minv[j],  j1 = j;
                }
            for (int j=0; j<=m; ++j)
                if (used[j])
                    u[p[j]] += delta,  v[j] -= delta;
                else
                    minv[j] -= delta;
            j0 = j1;
        } while (p[j0] != 0);
        do {
            int j1 = way[j0];
            p[j0] = p[j1];
            j0 = j1;
        } while (j0);
    }

    {
        vector<int> ans (n+1);
        for (int j=1; j<=m; ++j)
            ans[p[j]] = j;

        int row = 0;
        int sum = 0;

        cout << "answer:" << endl;
        for(auto& el : ans)
        {
            if(!row)
            {
                row++;
                continue;
            }
            cout << "row: " << row << ", column: " << el << endl;
            sum += a[row][el];
            row++;
        }

        cout << "sum: " << sum << endl;
    }
}
