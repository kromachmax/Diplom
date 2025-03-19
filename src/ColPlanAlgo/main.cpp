
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


#include <QtWidgets/QApplication>
#include <QtWidgets/QMainWindow>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>

#include <iostream>
#include <vector>
#include <string>
#include <chrono>
#include <random>

#include <COI_3_7.hpp>
#include <COI_3_9.hpp>

using namespace std::chrono;

QT_CHARTS_USE_NAMESPACE

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    std::random_device rd;
    std::mt19937 gen(rd());

    std::uniform_int_distribution<> sizeDist(1, 50);
    std::uniform_real_distribution<> valueDist(0.0, 23.0);
    std::uniform_int_distribution<> nDist(1, 10);

    const int NUM_ITERATIONS = 100;

    for(int iter = 0; iter < NUM_ITERATIONS; iter++)
    {
        int n = sizeDist(gen);
        int m = sizeDist(gen);

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

        std::cout << "\nIteration " << iter + 1 << ":" << std::endl;
        std::cout << "Matrix size: " << n << " x " << m << std::endl;
        std::cout << "COI_3_7 Answer: " << answer_3_7 << ", Time: " << duration_3_7.count() << " microseconds" << std::endl;
        std::cout << "COI_3_9 Answer: " << answer_3_9 << ", Time: " << duration_3_9.count() << " microseconds" << std::endl;
    }


//    // Создание серий данных
//    QLineSeries *series1 = new QLineSeries();
//    series1->setName("Dataset 1");
//    series1->append(0, 1);
//    series1->append(1, 2);
//    series1->append(2, 3);
//    series1->append(3, 2);
//    series1->append(4, 5);

//    QLineSeries *series2 = new QLineSeries();
//    series2->setName("Dataset 2");
//    series2->append(0, 0);
//    series2->append(1, 1);
//    series2->append(2, 2);
//    series2->append(3, 4);
//    series2->append(4, 3);

//    // Создание объекта графика
//    QChart *chart = new QChart();
//    chart->addSeries(series1); // Добавление первой серии
//    chart->addSeries(series2); // Добавление второй серии
//    chart->setTitle("Example Line Chart");
//    chart->legend()->setAlignment(Qt::AlignBottom);

//    // Настройка осей
//    QValueAxis *axisX = new QValueAxis();
//    axisX->setTitleText("X Axis");
//    axisX->setRange(0, 4);
//    axisX->setLabelFormat("%.0f");

//    QValueAxis *axisY = new QValueAxis();
//    axisY->setTitleText("Y Axis");
//    axisY->setRange(0, 6);
//    axisY->setLabelFormat("%.0f");

//    // Добавление осей к графику
//    chart->addAxis(axisX, Qt::AlignBottom); // Ось X внизу
//    chart->addAxis(axisY, Qt::AlignLeft);   // Ось Y слева

//    // Привязка осей к сериям
//    series1->attachAxis(axisX);
//    series1->attachAxis(axisY);
//    series2->attachAxis(axisX);
//    series2->attachAxis(axisY);

//    // Создание представления графика
//    QChartView *chartView = new QChartView(chart);
//    chartView->setRenderHint(QPainter::Antialiasing);

//    // Настройка окна
//    QMainWindow window;
//    window.setCentralWidget(chartView);
//    window.resize(800, 600);
//    window.setWindowTitle("Qt Chart Example");
//    window.show();

    return app.exec();
}
