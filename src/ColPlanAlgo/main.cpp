#include <QtWidgets/QApplication>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>
#include <QMainWindow>
#include <iostream>
#include <vector>
#include <random>
#include <chrono>
#include "COI_3_7.hpp"
#include "COI_3_9.hpp"

QT_CHARTS_USE_NAMESPACE
using namespace std;
using namespace std::chrono;

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    std::random_device rd;
    std::mt19937 gen(rd());

    std::uniform_real_distribution<> valueDist(0.0, 23.0);
    std::uniform_int_distribution<> nDist(1, 10);

    const int NUM_ITERATIONS = 70;

    std::vector<int> matrixSizes; // Размеры матриц (n * m)
    std::vector<double> answers_3_7;
    std::vector<double> answers_3_9;
    std::vector<long long> times_3_7; // Время выполнения COI_3_7
    std::vector<long long> times_3_9; // Время выполнения COI_3_9
    std::vector<double> answer_diffs; // Разница решений (answer_3_9 - answer_3_7)

    for(int iter = 0; iter < NUM_ITERATIONS; iter++)
    {
        int n = iter + 1; // Размер от 1 до 10
        int m = n;        // Квадратная матрица: m = n

        std::vector<std::vector<double>> D(n, std::vector<double>(m));
        std::vector<int> N(m);

        for (int i = 0; i < n; i++)
        {
            for (int j = 0; j < m; j++)
            {
                D[i][j] = valueDist(gen);
            }
        }

        for (int i = 0; i < m; i++)
        {
            N[i] = nDist(gen);
        }

        auto start_3_7 = high_resolution_clock::now();

        COI_3_7<double> coi_3_7;
        coi_3_7.Update(n, m, D, N);
        double answer_3_7 = coi_3_7.Start();

        auto end_3_7 = high_resolution_clock::now();
        auto duration_3_7 = duration_cast<microseconds>(end_3_7 - start_3_7);

        auto start_3_9 = high_resolution_clock::now();

        COI_3_9<double> coi_3_9;
        coi_3_9.Update(n, m, D, N);
        double answer_3_9 = coi_3_9.Start();

        auto end_3_9 = high_resolution_clock::now();
        auto duration_3_9 = duration_cast<microseconds>(end_3_9 - start_3_9);

        matrixSizes.push_back(n);
        answers_3_7.push_back(answer_3_7);
        answers_3_9.push_back(answer_3_9);
        times_3_7.push_back(duration_3_7.count());
        times_3_9.push_back(duration_3_9.count());
        answer_diffs.push_back(answer_3_9 - answer_3_7);

        std::cout << "\nIteration " << iter + 1 << ":" << std::endl;
        std::cout << "Matrix size: " << n << " x " << m << std::endl;
        std::cout << "COI_3_7 Answer: " << answer_3_7 << ", Time: " << duration_3_7.count() << " microseconds" << std::endl;
        std::cout << "COI_3_9 Answer: " << answer_3_9 << ", Time: " << duration_3_9.count() << " microseconds" << std::endl;
    }

    // Первый график: зависимость времени от размера матрицы
    QLineSeries *series_time_3_7 = new QLineSeries();
    series_time_3_7->setName("COI_3_7 Time");
    QLineSeries *series_time_3_9 = new QLineSeries();
    series_time_3_9->setName("COI_3_9 Time");

    for (size_t i = 0; i < matrixSizes.size(); ++i)
    {
        series_time_3_7->append(matrixSizes[i], times_3_7[i]);
        series_time_3_9->append(matrixSizes[i], times_3_9[i]);
    }

    QChart *chart_time = new QChart();
    chart_time->addSeries(series_time_3_7);
    chart_time->addSeries(series_time_3_9);
    chart_time->setTitle("Execution Time vs Matrix Size");
    chart_time->legend()->setAlignment(Qt::AlignBottom);

    QValueAxis *axisX_time = new QValueAxis();
    axisX_time->setTitleText("Matrix Size (n * m)");
    axisX_time->setRange(0, 100); // Максимум 10 * 10 = 100
    axisX_time->setLabelFormat("%.0f");

    QValueAxis *axisY_time = new QValueAxis();
    axisY_time->setTitleText("Time (microseconds)");
    axisY_time->setRange(0, 50000); // Примерный диапазон, можно настроить
    axisY_time->setLabelFormat("%.0f");

    chart_time->addAxis(axisX_time, Qt::AlignBottom);
    chart_time->addAxis(axisY_time, Qt::AlignLeft);

    series_time_3_7->attachAxis(axisX_time);
    series_time_3_7->attachAxis(axisY_time);
    series_time_3_9->attachAxis(axisX_time);
    series_time_3_9->attachAxis(axisY_time);

    QChartView *chartView_time = new QChartView(chart_time);
    chartView_time->setRenderHint(QPainter::Antialiasing);

    QMainWindow window_time;
    window_time.setCentralWidget(chartView_time);
    window_time.resize(800, 600);
    window_time.setWindowTitle("Execution Time vs Matrix Size");
    window_time.show();

    // Второй график: зависимость разницы решений от размера матрицы
    QLineSeries *series_diff = new QLineSeries();
    series_diff->setName("Answer Difference (COI_3_9 - COI_3_7)");

    for (size_t i = 0; i < matrixSizes.size(); ++i)
    {
        series_diff->append(matrixSizes[i], answer_diffs[i]);
    }

    QChart *chart_diff = new QChart();
    chart_diff->addSeries(series_diff);
    chart_diff->setTitle("Answer Difference vs Matrix Size");
    chart_diff->legend()->setAlignment(Qt::AlignBottom);

    QValueAxis *axisX_diff = new QValueAxis();
    axisX_diff->setTitleText("Matrix Size (n * m)");
    axisX_diff->setRange(0, 100); // Максимум 10 * 10 = 100
    axisX_diff->setLabelFormat("%.0f");

    QValueAxis *axisY_diff = new QValueAxis();
    axisY_diff->setTitleText("Answer Difference");
    axisY_diff->setRange(-100, 100); // Примерный диапазон, можно настроить
    axisY_diff->setLabelFormat("%.0f");

    chart_diff->addAxis(axisX_diff, Qt::AlignBottom);
    chart_diff->addAxis(axisY_diff, Qt::AlignLeft);

    series_diff->attachAxis(axisX_diff);
    series_diff->attachAxis(axisY_diff);

    QChartView *chartView_diff = new QChartView(chart_diff);
    chartView_diff->setRenderHint(QPainter::Antialiasing);

    QMainWindow window_diff;
    window_diff.setCentralWidget(chartView_diff);
    window_diff.resize(800, 600);
    window_diff.setWindowTitle("Answer Difference vs Matrix Size");
    window_diff.show();

    return app.exec();
}
