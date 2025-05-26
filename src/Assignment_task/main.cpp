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


void generate_random_instance(
    int n, int m,
    std::vector<std::vector<double>>& alpha_auction,
    std::vector<std::vector<int>>& visibility_robots,
    double visibility_radius,
    std::mt19937& gen)
{
    std::uniform_real_distribution<double> coord_dist(PARAMETRS::min_utility, PARAMETRS::max_utility);
    alpha_auction.assign(n, std::vector<double>(m, -std::numeric_limits<double>::infinity()));
    visibility_robots.assign(n, std::vector<int>(n, 0));

    std::vector<Point> robot_coords(n);
    for (int i = 0; i < n; ++i) {
        robot_coords[i] = {coord_dist(gen), coord_dist(gen)};
    }

    std::vector<Point> task_coords(m);
    for (int j = 0; j < m; ++j) {
        task_coords[j] = {coord_dist(gen), coord_dist(gen)};
    }

    for (int i = 0; i < n; ++i)
    {
        for (int j = 0; j < n; ++j)
        {
            if (i != j && calculate_distance(robot_coords[i], robot_coords[j]) <= visibility_radius) {
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

QChart* create_time_chart(const std::vector<int>& sizes,
                     const std::vector<double>& auction_times,
                     const std::vector<double>& hungarian_times)
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

    QLogValueAxis *axisY = new QLogValueAxis();
    axisY->setTitleText("Время выполнения (мс)");
    axisY->setLabelFormat("%.2f");
    axisY->setBase(10.0);
    double min_time = *std::min_element(auction_times.begin(), auction_times.end());
    double max_time = *std::max_element(hungarian_times.begin(), hungarian_times.end());
    axisY->setRange(std::max(0.1, min_time * 0.5), max_time * 2.0);
    chart->addAxis(axisY, Qt::AlignLeft);
    auction_series->attachAxis(axisY);
    hungarian_series->attachAxis(axisY);

    return chart;
}


QChart* create_accuracy_chart(const std::vector<double>& radii,
                              const std::vector<double>& accuracy_avg)
{
    QChart *chart = new QChart();
    chart->setTitle("Зависимость точности от радиуса видимости");
    chart->setAnimationOptions(QChart::AllAnimations);

    QLineSeries *accuracy_series = new QLineSeries();
    accuracy_series->setName("Относительная точность аукционного алгоритма");

    for (size_t i = 0; i < radii.size(); ++i) {
        accuracy_series->append(radii[i], accuracy_avg[i]);
    }

    chart->addSeries(accuracy_series);

    QValueAxis *axisX = new QValueAxis();
    axisX->setTitleText("Радиус видимости");
    axisX->setLabelFormat("%.1f");
    axisX->setTickCount(radii.size());
    chart->addAxis(axisX, Qt::AlignBottom);
    accuracy_series->attachAxis(axisX);

    QValueAxis *axisY = new QValueAxis();
    axisY->setTitleText("Относительная точность (%)");
    axisY->setLabelFormat("%.2f");
    double min_accuracy = *std::min_element(accuracy_avg.begin(), accuracy_avg.end());
    double max_accuracy = *std::max_element(accuracy_avg.begin(), accuracy_avg.end());
    axisY->setRange(std::max(0.0, min_accuracy * 0.9), max_accuracy * 1.1);
    chart->addAxis(axisY, Qt::AlignLeft);
    accuracy_series->attachAxis(axisY);

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
    const int max_size = 50;
    const int step = 1;
    const int num_repeats = 10;

    std::random_device rd;
    std::mt19937 gen(rd());

    std::vector<int> sizes;
    std::vector<double> auction_times_avg;
    std::vector<double> hungarian_times_avg;

    PARAMETRS::min_utility = 1.0;
    PARAMETRS::max_utility = 100.0;

    double fixed_visibility_radius = 15.0;
    double fixed_size = 2;
    double min_radius = 1;
    double max_radius = 300;

    std::vector<double> radii;
    std::vector<double> accuracy_avg;

    for (int n = min_size; n <= max_size; n += step)
    {
        int m = n;
        sizes.push_back(n);

        double auction_total = 0.0;
        double hungarian_total = 0.0;

        for (int repeat = 0; repeat < num_repeats; ++repeat)
        {
            std::vector<std::vector<double>> alpha;
            std::vector<std::vector<int>> visibility_robots;
            generate_random_instance(n, m, alpha, visibility_robots, fixed_visibility_radius, gen);

            AuctionAlgo<double> algo;
            std::vector<int> auction_assignment;

            auto start = std::chrono::high_resolution_clock::now();
            algo.Start(n, m, alpha, visibility_robots, 1.0 / n, auction_assignment);
            auto end = std::chrono::high_resolution_clock::now();
            auction_total += std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() / 1000.0;

            std::vector<int> hungarian_assignment;
            std::vector<int> N_max(m, 1);
            HungarianAlgo<double> hungarian_algo(n, m, alpha, N_max);

            start = std::chrono::high_resolution_clock::now();
            hungarian_algo.Start(hungarian_assignment);
            end = std::chrono::high_resolution_clock::now();
            hungarian_total += std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() / 1000.0;
        }

        auction_times_avg.push_back(auction_total / num_repeats);
        hungarian_times_avg.push_back(hungarian_total / num_repeats);

        std::cout << "Размер: " << n << "x" << m
             << " | Аукционный: " << auction_times_avg.back() << " мс"
             << " | Венгерский: " << hungarian_times_avg.back() << " мс" << std::endl;
    }


    for (double r = min_radius; r <= max_radius; r += step)
    {
        radii.push_back(r);
        double accuracy_total = 0.0;

        for (int repeat = 0; repeat < num_repeats; ++repeat)
        {
            std::vector<std::vector<double>> alpha;
            std::vector<std::vector<int>> visibility_robots;
            generate_random_instance(fixed_size, fixed_size, alpha, visibility_robots, r, gen);

            AuctionAlgo<double> algo;
            std::vector<int> auction_assignment;
            double auction_benefit = algo.Start(fixed_size, fixed_size, alpha, visibility_robots, 1e-2, auction_assignment);

            std::vector<int> hungarian_assignment;
            std::vector<int> N_max(fixed_size, 1);
            HungarianAlgo<double> hungarian_algo(fixed_size, fixed_size, alpha, N_max);
            double hungarian_benefit = hungarian_algo.Start(hungarian_assignment);

            if (hungarian_benefit > 0) {
                accuracy_total += auction_benefit / hungarian_benefit * 100;
            } else {
                accuracy_total += 0.0;
            }
        }

        accuracy_avg.push_back(accuracy_total / num_repeats);
        std::cout << "Радиус: " << r
                  << " | Точность: " << accuracy_avg.back() << " %" << std::endl;
    }

    QChartView *chartView = new QChartView(create_time_chart(sizes, auction_times_avg, hungarian_times_avg));
    chartView->setRenderHint(QPainter::Antialiasing);
    chartView->resize(800, 600);
    chartView->show();

    QChartView *accuracyChartView = new QChartView(create_accuracy_chart(radii, accuracy_avg));
    accuracyChartView->setRenderHint(QPainter::Antialiasing);
    accuracyChartView->resize(800, 600);
    accuracyChartView->show();

    return a.exec();
}
