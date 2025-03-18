#include <QApplication>

#include <COI_3_7.hpp>
#include <COI_3_9.hpp>

using namespace std;


//0.10 0.09 0.04 0.08 0.05
//0.20 0.17 0.07 0.30 0.25
//0.15 0.30 0.40 0.35 0.20
//0.12 0.50 0.45 0.10 0.30
//0.08 0.60 0.30 0.20 0.75

//0.20 0.08 0.25 0.10 0.12
//0.12 0.23 0.28 0.16 0.08
//0.30 0.13 0.07 0.30 0.20
//0.25 0.28 0.14 0.14 0.20
//0.16 0.35 0.17 0.40 0.30
//0.23 0.27 0.17 0.42 0.45
//0.27 0.14 0.18 0.27 0.24
//0.14 0.15 0.17 0.19 0.20
//0.17 0.18 0.20 0.24 0.18
//0.24 0.20 0.24 0.24 0.18


int main(int argc, char *argv[])
{
    int n, m;

    cout << "Enter number of rows: " << endl;

    cin >> n;

    cout << "Enter number of columns: " << endl;

    cin >> m;

    vector<vector<float>> D(n, vector<float>(m));
    vector<int> N(m);

    cout << "Enter the matrix of efficiency: " << endl;

    for(int i = 0; i < n; i++)
    {
        for(int j = 0; j < m; j++)
        {
            cin >> D[i][j];
        }
    }

    cout << "Enter the N_max vector: " << endl;

    for(int i = 0; i < m; i++)
    {
        cin >> N[i];
    }

    cout << "Starting compute..." << endl;

    //COI_3_7<float> coi(n, m, D, N);
    COI_3_9<float> coi(n, m, D, N);

    float answer = coi.Start();

    cout << "Answer: " << answer << endl;

    return 0;
}
