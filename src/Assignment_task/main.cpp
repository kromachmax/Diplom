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
                          const std::vector<std::vector<double>>& auction_times,
                          const std::vector<std::vector<double>>& hungarian_times,
                          const std::vector<double>& fixed_visibility_radii)
{
    QChart *chart = new QChart();
    chart->setTitle("Сравнение времени выполнения алгоритмов для разных радиусов");
    chart->setAnimationOptions(QChart::AllAnimations);
    chart->legend()->setVisible(true);
    chart->legend()->setAlignment(Qt::AlignBottom);

    if (auction_times.size() != fixed_visibility_radii.size() ||
            hungarian_times.size() != fixed_visibility_radii.size()) {
        qWarning() << "Несоответствие размеров входных данных";
        return chart;
    }

    for (size_t radius_idx = 0; radius_idx < fixed_visibility_radii.size(); ++radius_idx) {
        QLineSeries *auction_series = new QLineSeries();
        auction_series->setName(QString("Аукционный (R=%1)").arg(fixed_visibility_radii[radius_idx]));

        for (size_t i = 0; i < sizes.size(); ++i) {
            auction_series->append(sizes[i], auction_times[radius_idx][i]);
        }
        chart->addSeries(auction_series);
    }

    QLineSeries *hungarian_series = new QLineSeries();
    hungarian_series->setName(QString("Венгерский").arg(fixed_visibility_radii[0]));
    hungarian_series->setPen(QPen(Qt::red, 2, Qt::DashLine)); // Стиль для венгерского

    for (size_t i = 0; i < sizes.size(); ++i) {
        hungarian_series->append(sizes[i], hungarian_times[0][i]);
    }
    chart->addSeries(hungarian_series);


    QValueAxis *axisX = new QValueAxis();
    axisX->setTitleText("Размер задачи (n)");
    axisX->setLabelFormat("%d");
    axisX->setTickCount(20);
    chart->addAxis(axisX, Qt::AlignBottom);

    QLogValueAxis *axisY = new QLogValueAxis();
    axisY->setTitleText("Время выполнения (мс)");
    axisY->setLabelFormat("%.2f");
    axisY->setBase(10.0);

    double min_time = std::numeric_limits<double>::max();
    double max_time = std::numeric_limits<double>::min();

    for (const auto& vec : auction_times) {
        auto current_min = *std::min_element(vec.begin(), vec.end());
        auto current_max = *std::max_element(vec.begin(), vec.end());
        if (current_min < min_time) min_time = current_min;
        if (current_max > max_time) max_time = current_max;
    }

    for (const auto& vec : hungarian_times) {
        auto current_min = *std::min_element(vec.begin(), vec.end());
        auto current_max = *std::max_element(vec.begin(), vec.end());
        if (current_min < min_time) min_time = current_min;
        if (current_max > max_time) max_time = current_max;
    }

    axisY->setRange(min_time * 0.9, max_time * 1.1);
    chart->addAxis(axisY, Qt::AlignLeft);

    for (auto series : chart->series()) {
        series->attachAxis(axisX);
        series->attachAxis(axisY);
    }

    return chart;
}


QChart* create_accuracy_chart(const std::vector<int>& radii,
                              const std::vector<std::vector<double>>& all_accuracy_avg,
                              const std::vector<int>& fixed_sizes)
{
    QChart *chart = new QChart();
    chart->setTitle("Зависимость точности от радиуса видимости для разных размеров");
    chart->setAnimationOptions(QChart::AllAnimations);

    if (all_accuracy_avg.size() != fixed_sizes.size())
    {
        qWarning() << "Несоответствие размеров all_accuracy_avg и fixed_sizes";
        return chart;
    }

    for (size_t size_idx = 0; size_idx < fixed_sizes.size(); ++size_idx)
    {
        QLineSeries *series = new QLineSeries();
        series->setName(QString("Размер %1").arg(fixed_sizes[size_idx]));

        if (all_accuracy_avg[size_idx].size() != radii.size())
        {
            qWarning() << "Несоответствие размеров all_accuracy_avg[" << size_idx << "] и radii";
            std::cout << all_accuracy_avg[size_idx].size() << std::endl;
            std::cout << radii.size() << std::endl;
            continue;
        }

        for (size_t i = 0; i < radii.size(); ++i) {
            series->append(radii[i], all_accuracy_avg[size_idx][i]);
        }

        chart->addSeries(series);
    }

    QValueAxis *axisX = new QValueAxis();
    axisX->setTitleText("Радиус видимости");
    axisX->setLabelFormat("%d");
    axisX->setTickCount(20);
    chart->addAxis(axisX, Qt::AlignBottom);

    QValueAxis *axisY = new QValueAxis();
    axisY->setTitleText("Относительная точность (%)");
    axisY->setLabelFormat("%.1f");
    axisY->setRange(50.0, 110.0);
    axisY->setTickCount(7);
    axisY->setTickInterval(10.0);
    chart->addAxis(axisY, Qt::AlignLeft);

    for (auto series : chart->series())
    {
        series->attachAxis(axisX);
        series->attachAxis(axisY);
    }

    chart->legend()->setVisible(true);
    chart->legend()->setAlignment(Qt::AlignBottom);

    return chart;
}

QChart* create_epsilon_time_chart(const std::vector<double>& epsilons,
                                  const std::vector<std::vector<double>>& auction_times,
                                  const std::vector<double>& fixed_visibility_radii)
{
    QChart *chart = new QChart();
    chart->setTitle("Зависимость времени выполнения от ε для разных радиусов");
    chart->setAnimationOptions(QChart::AllAnimations);
    chart->legend()->setVisible(true);
    chart->legend()->setAlignment(Qt::AlignBottom);

    if (auction_times.size() != fixed_visibility_radii.size()) {
        qWarning() << "Несоответствие размеров auction_times и fixed_visibility_radii";
        return chart;
    }

    QList<QColor> colors = {Qt::blue, Qt::red, Qt::green, Qt::magenta, Qt::cyan, Qt::yellow};

    for (size_t radius_idx = 0; radius_idx < fixed_visibility_radii.size(); ++radius_idx)
    {
        QLineSeries *series = new QLineSeries();
        series->setName(QString("R=%1").arg(fixed_visibility_radii[radius_idx]));

        QPen pen(colors[radius_idx % colors.size()]);
        pen.setWidth(2);
        series->setPen(pen);

        for (size_t i = 0; i < epsilons.size(); ++i) {
            series->append(epsilons[i], auction_times[radius_idx][i]);
        }

        chart->addSeries(series);
    }

    QLogValueAxis *axisX = new QLogValueAxis();
    axisX->setTitleText("Значение ε");
    axisX->setLabelFormat("%.2e");
    axisX->setBase(10.0);
    axisX->setRange(*std::min_element(epsilons.begin(), epsilons.end()),
                    *std::max_element(epsilons.begin(), epsilons.end()));
    chart->addAxis(axisX, Qt::AlignBottom);

    QValueAxis *axisY = new QValueAxis();
    axisY->setTitleText("Время выполнения (мс)");
    axisY->setLabelFormat("%.1f");

    double min_time = std::numeric_limits<double>::max();
    double max_time = std::numeric_limits<double>::min();

    for (const auto& vec : auction_times)
    {
        auto current_min = *std::min_element(vec.begin(), vec.end());
        auto current_max = *std::max_element(vec.begin(), vec.end());
        if (current_min < min_time) min_time = current_min;
        if (current_max > max_time) max_time = current_max;
    }

    double rounded_min = std::floor(std::max(0.1, min_time * 0.9) / 10.0) * 10.0;
    double rounded_max = std::ceil((max_time * 1.1) / 10.0) * 10.0;
    axisY->setRange(rounded_min, rounded_max);

    double range = rounded_max - rounded_min;
    double tick_interval;
    if (range < 50.0) {
        tick_interval = 5.0;
    }
    else if (range < 500.0) {
        tick_interval = 10.0;
    }
    else if (range < 1000.0) {
        tick_interval = 50.0;
    }
    else {
        tick_interval = 100.0;
    }
    axisY->setTickInterval(tick_interval);

    int tick_count = static_cast<int>((rounded_max - rounded_min) / tick_interval) + 1;
    if (tick_count > 10)
    {
        tick_interval = (rounded_max - rounded_min) / (tick_count - 1);
        axisY->setTickInterval(tick_interval);
    }

    axisY->setTickCount(tick_count);
    axisY->setMinorTickCount(4);
    chart->addAxis(axisY, Qt::AlignLeft);

    for (auto series : chart->series())
    {
        series->attachAxis(axisX);
        series->attachAxis(axisY);
    }

    return chart;
}

QChart* create_epsilon_accuracy_chart(const std::vector<double>& epsilons,
                                      const std::vector<std::vector<double>>& all_accuracy_avg,
                                      const std::vector<double>& fixed_visibility_radii)
{
    QChart *chart = new QChart();
    chart->setTitle("Зависимость точности от ε для разных радиусов");
    chart->setAnimationOptions(QChart::AllAnimations);
    chart->legend()->setVisible(true);
    chart->legend()->setAlignment(Qt::AlignBottom);

    QList<QColor> colors = {Qt::blue, Qt::red, Qt::green, Qt::magenta, Qt::cyan, Qt::yellow};

    for (size_t radius_idx = 0; radius_idx < fixed_visibility_radii.size(); ++radius_idx)
    {
        QLineSeries *series = new QLineSeries();
        series->setName(QString("R=%1").arg(fixed_visibility_radii[radius_idx]));

        QPen pen(colors[radius_idx % colors.size()]);
        pen.setWidth(2);
        series->setPen(pen);

        for (size_t i = 0; i < epsilons.size(); ++i) {
            series->append(epsilons[i], all_accuracy_avg[radius_idx][i]);
        }

        chart->addSeries(series);
    }

    QLogValueAxis *axisX = new QLogValueAxis();
    axisX->setTitleText("Значение ε");
    axisX->setLabelFormat("%.2e");
    axisX->setBase(10.0);
    axisX->setRange(*std::min_element(epsilons.begin(), epsilons.end()),
                    *std::max_element(epsilons.begin(), epsilons.end()));
    chart->addAxis(axisX, Qt::AlignBottom);

    QValueAxis *axisY = new QValueAxis();
    axisY->setTitleText("Относительная точность (%)");
    axisY->setLabelFormat("%.1f");

    double rounded_min = 50.0;
    double rounded_max = 110.0;
    axisY->setRange(rounded_min, rounded_max);

    double tick_interval = 10.0;
    axisY->setTickInterval(tick_interval);
    chart->addAxis(axisY, Qt::AlignLeft);

    for (auto series : chart->series())
    {
        series->attachAxis(axisX);
        series->attachAxis(axisY);
    }

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

    std::random_device rd;
    std::mt19937 gen(rd());

    const std::vector<double> fixed_visibility_radii = {10, 20, 30};
    std::vector<int> sizes;
    std::vector<std::vector<double>> all_auction_times_avg(fixed_visibility_radii.size());
    std::vector<std::vector<double>> all_hungarian_times_avg(fixed_visibility_radii.size());

    std::vector<int> radii;
    const std::vector<int> fixed_sizes = {10, 30, 50};
    std::vector<std::vector<double>> all_accuracy_avg(fixed_sizes.size());

    std::vector<double> epsilons;
    std::vector<std::vector<double>> all_epsilon_times_avg(fixed_visibility_radii.size());
    std::vector<std::vector<double>> all_epsilon_accuracy_avg(fixed_visibility_radii.size());

    PARAMETRS::min_utility = 1.0;
    PARAMETRS::max_utility = 100.0;

    const double fixed_size_eps = 50;
    const double min_radius = 5;
    const double max_radius = 100;
    const double min_epsilon = 1e-6;
    const double max_epsilon = 1.0;

    {
        for(int i = min_size; i <= max_size; i += step) {
            sizes.push_back(i);
        }

        for(int i = 0; i < fixed_visibility_radii.size(); ++i)
        {
            auto& auction_times_avg = all_auction_times_avg[i];
            auto& hungarian_times_avg = all_hungarian_times_avg[i];
            double fixed_visibility_radius = fixed_visibility_radii[i];

            for(auto& n : sizes)
            {
                int m = n;

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

                std::cout << "Радиус: " << fixed_visibility_radius << " | Размер: " << n << "x" << m
                          << " | Аукционный: " << auction_times_avg.back() << " мс"
                          << " | Венгерский: " << hungarian_times_avg.back() << " мс" << std::endl;
            }
        }
    }

    {
        for (int i = min_radius; i <= max_radius; ++i) {
            radii.push_back(i);
        }

        for(int i = 0; i < fixed_sizes.size(); ++i)
        {
            std::vector<double>& accuracy_avg = all_accuracy_avg[i];
            int fixed_size = fixed_sizes[i];

            for (auto& r : radii)
            {
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
                std::cout << "Размер: " << fixed_size << " | Радиус: " << r
                          << " | Точность: " << accuracy_avg.back() << " %" << std::endl;
            }
        }
    }

    {
        for(double e = min_epsilon; e <= max_epsilon; e *= 10.0)
        {
            epsilons.push_back(e);
        }

        for(int i = 0; i < fixed_visibility_radii.size(); ++i)
        {
            double fixed_visibility_radius = fixed_visibility_radii[i];
            auto& epsilon_times_avg = all_epsilon_times_avg[i];
            auto& epsilon_accuracy_avg = all_epsilon_accuracy_avg[i];

            for (auto& e : epsilons)
            {
                double auction_time_total = 0.0;
                double accuracy_total = 0.0;

                for (int repeat = 0; repeat < num_repeats; ++repeat)
                {
                    std::vector<std::vector<double>> alpha;
                    std::vector<std::vector<int>> visibility_robots;
                    generate_random_instance(fixed_size_eps, fixed_size_eps, alpha, visibility_robots, fixed_visibility_radius, gen);

                    AuctionAlgo<double> algo;
                    std::vector<int> auction_assignment;

                    auto start = std::chrono::high_resolution_clock::now();
                    double auction_benefit = algo.Start(fixed_size_eps, fixed_size_eps, alpha, visibility_robots, e, auction_assignment);
                    auto end = std::chrono::high_resolution_clock::now();
                    auction_time_total += std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() / 1000.0;

                    std::vector<int> hungarian_assignment;
                    std::vector<int> N_max(fixed_size_eps, 1);
                    HungarianAlgo<double> hungarian_algo(fixed_size_eps, fixed_size_eps, alpha, N_max);
                    double hungarian_benefit = hungarian_algo.Start(hungarian_assignment);

                    if (hungarian_benefit > 0) {
                        accuracy_total += (auction_benefit / hungarian_benefit) * 100;
                    } else {
                        accuracy_total += 0.0;
                    }
                }

                epsilon_times_avg.push_back(auction_time_total / num_repeats);
                epsilon_accuracy_avg.push_back(accuracy_total / num_repeats);

                std::cout << "Радиус: "  << fixed_visibility_radius << " | ε: " << e
                          << " | Время аукционного: " << epsilon_times_avg.back() << " мс"
                          << " | Точность: " << epsilon_accuracy_avg.back() << " %" << std::endl;
            }
        }
    }

    QChartView *chartView = new QChartView(create_time_chart(sizes, all_auction_times_avg, all_hungarian_times_avg, fixed_visibility_radii));
    chartView->setRenderHint(QPainter::Antialiasing);
    chartView->resize(800, 600);
    chartView->show();

    QChartView *accuracyChartView = new QChartView(create_accuracy_chart(radii, all_accuracy_avg, fixed_sizes));
    accuracyChartView->setRenderHint(QPainter::Antialiasing);
    accuracyChartView->resize(800, 600);
    accuracyChartView->show();

    QChartView *epsilonTimeChartView = new QChartView(create_epsilon_time_chart(epsilons, all_epsilon_times_avg, fixed_visibility_radii));
    epsilonTimeChartView->setRenderHint(QPainter::Antialiasing);
    epsilonTimeChartView->resize(800, 600);
    epsilonTimeChartView->show();

    QChartView *epsilonAccuracyChartView = new QChartView(create_epsilon_accuracy_chart(epsilons, all_epsilon_accuracy_avg, fixed_visibility_radii));
    epsilonAccuracyChartView->setRenderHint(QPainter::Antialiasing);
    epsilonAccuracyChartView->resize(800, 600);
    epsilonAccuracyChartView->show();

    return a.exec();
}
