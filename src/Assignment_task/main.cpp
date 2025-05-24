#include <iostream>
#include <vector>
#include <cmath>
#include <random>
#include <chrono>
#include <QApplication>
#include <QtCharts>

#ifdef _WIN32
#include <winsock2.h>
#include <windows.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#endif

#include "AuctionAlgo.hpp"
#include "HungarianAlgo.hpp"

using namespace std;


void generate_random_instance(
    int n, int m,
    vector<vector<double>>& alpha_auction,
    vector<vector<int>>& visibility_robots,
    mt19937& gen)
{
    uniform_real_distribution<double> coord_dist(0.0, 100.0);
    alpha_auction.assign(n, vector<double>(m, -numeric_limits<double>::infinity()));
    visibility_robots.assign(n, vector<int>(n, 0));

    vector<Point> robot_coords(n);
    for (int i = 0; i < n; ++i) {
        robot_coords[i] = {coord_dist(gen), coord_dist(gen)};
    }

    vector<Point> task_coords(m);
    for (int j = 0; j < m; ++j) {
        task_coords[j] = {coord_dist(gen), coord_dist(gen)};
    }

    for (int i = 0; i < n; ++i)
    {
        for (int j = 0; j < n; ++j)
        {
            if (i != j && calculate_distance(robot_coords[i], robot_coords[j]) <= PARAMETRS::visibility_radius) {
                visibility_robots[i][j] = 1;
            }
        }
    }

    for (int i = 0; i < n; ++i)
    {
        for (int j = 0; j < m; ++j)
        {
            double distance = calculate_distance(robot_coords[i], task_coords[j]);
            alpha_auction[i][j] = PARAMETRS::max_utility / (distance + PARAMETRS::DISTANCE_OFFSET);
        }
    }
}

QChart* create_chart(const vector<int>& sizes,
                     const vector<double>& auction_times,
                     const vector<double>& hungarian_times)
{
    QChart *chart = new QChart();
    chart->setTitle("Сравнение времени выполнения алгоритмов");
    chart->setAnimationOptions(QChart::AllAnimations);

    QLineSeries *auction_series = new QLineSeries();
    auction_series->setName("Аукционный алгоритм");

    QLineSeries *hungarian_series = new QLineSeries();
    hungarian_series->setName("Венгерский алгоритм");

    for (size_t i = 0; i < sizes.size(); ++i)
    {
        auction_series->append(sizes[i], auction_times[i]);
        hungarian_series->append(sizes[i], hungarian_times[i]);
    }

    chart->addSeries(auction_series);
    chart->addSeries(hungarian_series);

    QValueAxis *axisX = new QValueAxis();
    axisX->setTitleText("Размер задачи (n)");
    axisX->setLabelFormat("%d");
    axisX->setTickCount(sizes.size());
    chart->addAxis(axisX, Qt::AlignBottom);
    auction_series->attachAxis(axisX);
    hungarian_series->attachAxis(axisX);

    QValueAxis *axisY = new QValueAxis();
    axisY->setTitleText("Время выполнения (мс)");
    axisY->setLabelFormat("%.2f");
    chart->addAxis(axisY, Qt::AlignLeft);
    auction_series->attachAxis(axisY);
    hungarian_series->attachAxis(axisY);

    return chart;
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#else
    setlocale(LC_ALL, "en_US.UTF-8");
#endif

    const int min_size = 5;
    const int max_size = 100;
    const int step = 1;
    const int num_repeats = 100;

    random_device rd;
    mt19937 gen(rd());

    vector<int> sizes;
    vector<double> auction_times_avg;
    vector<double> hungarian_times_avg;

    PARAMETRS::min_utility = 1.0;
    PARAMETRS::max_utility = 30.0;
    PARAMETRS::visibility_radius = 40.0;

    for (int n = min_size; n <= max_size; n += step)
    {
        int m = n;
        sizes.push_back(n);

        double auction_total = 0.0;
        double hungarian_total = 0.0;

        for (int repeat = 0; repeat < num_repeats; ++repeat)
        {
            vector<vector<double>> alpha;
            vector<vector<int>> visibility_robots;
            generate_random_instance(n, m, alpha, visibility_robots, gen);

            AuctionAlgo<double> algo;
            vector<int> auction_assignment;

            auto start = chrono::high_resolution_clock::now();
            algo.Start(n, m, alpha, visibility_robots, PARAMETRS::epsilon, auction_assignment);
            auto end = chrono::high_resolution_clock::now();
            auction_total += chrono::duration_cast<chrono::microseconds>(end - start).count() / 1000.0;

            vector<int> hungarian_assignment;
            vector<int> N_max(m, 1);
            HungarianAlgo<double> hungarian_algo(n, m, alpha, N_max);

            start = chrono::high_resolution_clock::now();
            hungarian_algo.Start(hungarian_assignment);
            end = chrono::high_resolution_clock::now();
            hungarian_total += chrono::duration_cast<chrono::microseconds>(end - start).count() / 1000.0;
        }

        auction_times_avg.push_back(auction_total / num_repeats);
        hungarian_times_avg.push_back(hungarian_total / num_repeats);

        cout << "Размер: " << n << "x" << m
             << " | Аукционный: " << auction_times_avg.back() << " мс"
             << " | Венгерский: " << hungarian_times_avg.back() << " мс" << endl;
    }

    QChartView *chartView = new QChartView(create_chart(sizes, auction_times_avg, hungarian_times_avg));
    chartView->setRenderHint(QPainter::Antialiasing);
    chartView->resize(800, 600);
    chartView->show();

    return a.exec();
}
