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
            alpha_auction[i][j] = calculate_distance(robot_coords[i], task_coords[j]) / PARAMETRS::speed;
        }
    }
}


void inverse_matrix(std::vector<std::vector<double>>& matrix)
{
    double max_val = matrix[0][0];
    int n = matrix.size();
    int m = matrix[0].size();

    for (std::size_t i = 0; i < n; ++i)
    {
        for (std::size_t j = 0; j < m; ++j)
        {
            if (matrix[i][j] > max_val) {
                max_val = matrix[i][j];
            }
        }
    }

    for (std::size_t i = 0; i < n; ++i)
    {
        for (std::size_t j = 0; j < m; ++j)
        {
            matrix[i][j] = max_val - matrix[i][j];
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

    for (auto& series : chart->series()) {
        series->attachAxis(axisX);
        series->attachAxis(axisY);
    }

    return chart;
}


QChart* create_operation_chart(
    const std::vector<int>& sizes,
    const std::vector<double>& operation_count_auction_avg,
    const std::vector<double>& logic_operation_count_auction_avg,
    const std::vector<double>& change_operation_count_auction_avg,
    const std::vector<double>& operation_count_hungarian_avg,
    const std::vector<double>& logic_operation_count_hungarian_avg,
    const std::vector<double>& change_operation_count_hungarian_avg,
    const int& r)
{
    QChart* chart = new QChart();
    chart->setTitle(QString("Сравнение количества операций (R=%1)").arg(r));
    chart->setAnimationOptions(QChart::AllAnimations);
    chart->legend()->setVisible(true);
    chart->legend()->setAlignment(Qt::AlignBottom);

    // Серия для аукционного алгоритма: общее количество операций
    QLineSeries* auction_total_series = new QLineSeries();
    auction_total_series->setName(QString("A(R=%1): Арифметические").arg(r));
    auction_total_series->setPen(QPen(Qt::blue, 2, Qt::SolidLine));
    for (size_t i = 0; i < sizes.size(); ++i)
    {
        auction_total_series->append(sizes[i], operation_count_auction_avg[i]);
    }
    chart->addSeries(auction_total_series);

    // Серия для аукционного алгоритма: логические операции
    QLineSeries* auction_logic_series = new QLineSeries();
    auction_logic_series->setName(QString("А(R=%1): Логические").arg(r));
    auction_logic_series->setPen(QPen(Qt::green, 2, Qt::SolidLine));
    for (size_t i = 0; i < sizes.size(); ++i)
    {
        auction_logic_series->append(sizes[i], logic_operation_count_auction_avg[i]);
    }
    chart->addSeries(auction_logic_series);

    // Серия для аукционного алгоритма: обмены
    QLineSeries* auction_change_series = new QLineSeries();
    auction_change_series->setName(QString("А(R=%1): Обмены").arg(r));
    auction_change_series->setPen(QPen(Qt::magenta, 2, Qt::SolidLine));
    for (size_t i = 0; i < sizes.size(); ++i)
    {
        auction_change_series->append(sizes[i], change_operation_count_auction_avg[i]);
    }
    chart->addSeries(auction_change_series);

    // Серия для венгерского алгоритма: общее количество операций
    QLineSeries* hungarian_total_series = new QLineSeries();
    hungarian_total_series->setName(QString("В: Арифметические"));
    hungarian_total_series->setPen(QPen(Qt::red, 2, Qt::DashLine));
    for (size_t i = 0; i < sizes.size(); ++i)
    {
        hungarian_total_series->append(sizes[i], operation_count_hungarian_avg[i]);
    }
    chart->addSeries(hungarian_total_series);

    // Серия для венгерского алгоритма: логические операции
    QLineSeries* hungarian_logic_series = new QLineSeries();
    hungarian_logic_series->setName(QString("В: Логические"));
    hungarian_logic_series->setPen(QPen(Qt::darkYellow, 2, Qt::DashLine));
    for (size_t i = 0; i < sizes.size(); ++i)
    {
        hungarian_logic_series->append(sizes[i], logic_operation_count_hungarian_avg[i]);
    }
    chart->addSeries(hungarian_logic_series);

    // Серия для венгерского алгоритма: обмены
    QLineSeries* hungarian_change_series = new QLineSeries();
    hungarian_change_series->setName(QString("В: Обмены"));
    hungarian_change_series->setPen(QPen(Qt::darkCyan, 2, Qt::DashLine));
    for (size_t i = 0; i < sizes.size(); ++i)
    {
        hungarian_change_series->append(sizes[i], change_operation_count_hungarian_avg[i]);
    }
    chart->addSeries(hungarian_change_series);

    // Ось X (размер задачи)
    QLogValueAxis* axisX = new QLogValueAxis();
    axisX->setTitleText("Размер задачи (n)");
    axisX->setLabelFormat("%.2f");
    axisX->setBase(10.0);
    chart->addAxis(axisX, Qt::AlignBottom);

    // Ось Y (логарифмическая, число операций)
    QLogValueAxis* axisY = new QLogValueAxis();
    axisY->setTitleText("Число операций");
    axisY->setLabelFormat("%.2f");
    axisY->setBase(10.0);

    // Определение диапазона для оси Y
    double min_ops = std::numeric_limits<double>::max();
    double max_ops = std::numeric_limits<double>::min();

    // Поиск минимума и максимума для всех серий
    for (const auto* data : {
             &operation_count_auction_avg,
             &logic_operation_count_auction_avg,
             &change_operation_count_auction_avg,
             &operation_count_hungarian_avg,
             &logic_operation_count_hungarian_avg,
             &change_operation_count_hungarian_avg
         })
    {
        auto current_min = *std::min_element(data->begin(), data->end());
        auto current_max = *std::max_element(data->begin(), data->end());
        min_ops = std::min(min_ops, current_min);
        max_ops = std::max(max_ops, current_max);
    }

    axisY->setRange(min_ops * 0.9, max_ops * 1.1);
    chart->addAxis(axisY, Qt::AlignLeft);

    for (auto& series : chart->series()) {
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
        series->setName(QString("N=%1").arg(fixed_sizes[size_idx]));

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

    QLineSeries* asymptote = new QLineSeries();
    asymptote->setName("Асимптота 100%");
    QPen asymptotePen(Qt::gray);
    asymptotePen.setWidth(2);
    asymptotePen.setStyle(Qt::DashLine);
    asymptote->setPen(asymptotePen);

    for (size_t i = 0; i < radii.size(); ++i)
    {
        asymptote->append(radii[i], 100.0);
    }

    chart->addSeries(asymptote);

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

    for (auto& series : chart->series())
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

    if (auction_times.size() != fixed_visibility_radii.size())
    {
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

        for (size_t i = 0; i < epsilons.size(); ++i)
        {
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

    double rounded_min = std::floor(std::max(0.0, min_time * 0.9) / 5.0) * 5.0;
    double rounded_max = std::ceil(max_time * 1.1 / 5.0) * 5.0;
    double range = rounded_max - rounded_min;

    double tick_interval;
    if (range <= 10.0) {
        tick_interval = 1.0;
    }
    else if (range <= 50.0) {
        tick_interval = 5.0;
    }
    else {
        tick_interval = 10.0;
    }

    int num_ticks = static_cast<int>(std::ceil(range / tick_interval)) + 1;
    rounded_max = rounded_min + (num_ticks - 1) * tick_interval;

    axisY->setRange(rounded_min, rounded_max);
    axisY->setTickInterval(tick_interval);
    axisY->setTickCount(num_ticks);

    chart->addAxis(axisY, Qt::AlignLeft);

    for (auto& series : chart->series())
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

    QLineSeries* asymptote = new QLineSeries();
    asymptote->setName("Асимптота 100%");
    QPen asymptotePen(Qt::gray);
    asymptotePen.setWidth(2);
    asymptotePen.setStyle(Qt::DashLine);
    asymptote->setPen(asymptotePen);

    for (size_t i = 0; i < epsilons.size(); ++i)
    {
        asymptote->append(epsilons[i], 100.0);
    }

    chart->addSeries(asymptote);

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

    double rounded_min = 70.0;
    double rounded_max = 110.0;
    axisY->setRange(rounded_min, rounded_max);

    double tick_interval = 10.0;
    axisY->setTickInterval(tick_interval);
    axisY->setTickCount(5);
    chart->addAxis(axisY, Qt::AlignLeft);

    for (auto& series : chart->series())
    {
        series->attachAxis(axisX);
        series->attachAxis(axisY);
    }

    return chart;
}


QChart* create_acc_per_iteration_chart(std::vector<std::vector<double>>& all_acc_per_iteration,
                                       const std::vector<int>& fixed_sizes)
{
    QChart* chart = new QChart();
    chart->setTitle("Точность алгоритма на каждой итерации");
    chart->setAnimationOptions(QChart::AllAnimations);
    chart->legend()->setVisible(true);
    chart->legend()->setAlignment(Qt::AlignBottom);

    QList<QColor> colors = {Qt::blue, Qt::red, Qt::green, Qt::magenta, Qt::cyan, Qt::darkYellow};

    if (all_acc_per_iteration.size() != fixed_sizes.size())
    {
        qWarning() << "Количество кривых не соответствует количеству размеров";
        return chart;
    }

    size_t max_iterations = 0;
    for (size_t i = 0; i < all_acc_per_iteration.size(); ++i)
    {
        if (all_acc_per_iteration[i].size() > max_iterations)
        {
            max_iterations = all_acc_per_iteration[i].size();
        }
    }
    // Ограничиваем максимальное количество итераций до 300
    max_iterations = std::min(max_iterations, size_t(500));

    for (size_t i = 0; i < fixed_sizes.size(); ++i)
    {
        QLineSeries* series = new QLineSeries();
        series->setName(QString("N=%1").arg(fixed_sizes[i]));

        QPen pen(colors[i % colors.size()]);
        pen.setWidth(2);
        series->setPen(pen);

        // Добавляем точки только для итераций до 300
        for (size_t iter = 0; iter < all_acc_per_iteration[i].size() && iter < max_iterations; ++iter) {
            series->append(iter + 1, all_acc_per_iteration[i][iter]);
        }

        chart->addSeries(series);
    }

    QLineSeries* asymptote = new QLineSeries();
    asymptote->setName("Асимптота 100%");
    QPen asymptotePen(Qt::gray);
    asymptotePen.setWidth(2);
    asymptotePen.setStyle(Qt::DashLine);
    asymptote->setPen(asymptotePen);

    // Асимптота также ограничивается до 300 итераций
    for (size_t iter = 1; iter <= max_iterations; ++iter)
    {
        asymptote->append(iter, 100.0);
    }

    chart->addSeries(asymptote);

    QValueAxis* axisX = new QValueAxis();
    axisX->setTitleText("Номер итерации");
    axisX->setLabelFormat("%d");
    axisX->setTickCount(std::min(max_iterations + 1, size_t(10)));
    // Устанавливаем диапазон оси X от 1 до 300
    axisX->setRange(1, max_iterations);
    chart->addAxis(axisX, Qt::AlignBottom);

    QValueAxis* axisY = new QValueAxis();
    axisY->setTitleText("Точность (%)");
    axisY->setLabelFormat("%.1f");
    axisY->setRange(50.0, 110.0);
    axisY->setTickCount(7);
    chart->addAxis(axisY, Qt::AlignLeft);

    for (auto& series : chart->series())
    {
        series->attachAxis(axisX);
        series->attachAxis(axisY);
    }

    return chart;
}


QChart* create_iterations_per_acc_chart(std::vector<std::vector<double>>& all_iters_per_acc,
                                        const std::vector<double>& eps,
                                        const std::vector<int>& fixed_sizes)
{
    QChart *chart = new QChart();
    chart->setTitle("Зависимость числа итераций от эпсилон(Аукционный)");

    QLogValueAxis *axisX = new QLogValueAxis();
    axisX->setTitleText("Значение ε");
    axisX->setLabelFormat("%.2e");
    axisX->setBase(10.0);
    axisX->setRange(*std::min_element(eps.begin(), eps.end()),
                    *std::max_element(eps.begin(), eps.end()));
    chart->addAxis(axisX, Qt::AlignBottom);

    QLogValueAxis *axisY = new QLogValueAxis();
    axisY->setTitleText("Количество итераций");
    axisY->setLabelFormat("%.0f");
    axisY->setBase(10.0);
    chart->addAxis(axisY, Qt::AlignLeft);

    for (size_t i = 0; i < fixed_sizes.size(); ++i)
    {
        QLineSeries *series = new QLineSeries();
        series->setName(QString("N=%1").arg(fixed_sizes[i]));

        for (size_t j = 0; j < eps.size(); ++j)
        {
            series->append(eps[j], all_iters_per_acc[i][j]);
        }

        chart->addSeries(series);
        series->attachAxis(axisX);
        series->attachAxis(axisY);
    }

    double maxY = 0;
    for (const auto& row : all_iters_per_acc)
    {
        auto maxIter = *std::max_element(row.begin(), row.end());
        if (maxIter > maxY) maxY = maxIter;
    }
    double minY = 1.0;
    axisY->setRange(minY, maxY * 1.1);

    chart->legend()->setVisible(true);
    chart->legend()->setAlignment(Qt::AlignTop);

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

    std::vector<int> fixed_sizes2 = {10, 30, 50, 100};
    std::vector<std::vector<double>> all_value_per_iteration(fixed_sizes2.size());

    std::vector<double> operation_count_auction_avg;
    std::vector<double> logic_operation_count_auction_avg;
    std::vector<double> change_operation_count_auction_avg;

    std::vector<double> operation_count_hungarian_avg;
    std::vector<double> logic_operation_count_hungarian_avg;
    std::vector<double> change_operation_count_hungarian_avg;


    std::vector<int> fixed_sizes3 = {20, 40, 70};
    std::vector<std::vector<double>> all_iterations_per_accuracy(fixed_sizes3.size());

    PARAMETRS::min_utility = 1.0;
    PARAMETRS::max_utility = 100.0;

    const double fixed_size_eps = 80;
    const double min_radius = 5;
    const double max_radius = 100;
    const double min_epsilon = 1e-5;
    const double max_epsilon = 10.0;

    double e = min_epsilon;
    while(e <= max_epsilon)
    {
        epsilons.push_back(e);
        e *= 10.0;
    }


    {
        for(int i = min_size; i <= max_size; i += step) {
            sizes.push_back(i);
        }

        // {
        //     for(int i = 0; i < fixed_visibility_radii.size(); ++i)
        //     {
        //         auto& auction_times_avg = all_auction_times_avg[i];
        //         auto& hungarian_times_avg = all_hungarian_times_avg[i];
        //         double fixed_visibility_radius = fixed_visibility_radii[i];

        //         for(auto& n : sizes)
        //         {
        //             int m = n;

        //             double auction_total = 0.0;
        //             double hungarian_total = 0.0;

        //             for (int repeat = 0; repeat < num_repeats; ++repeat)
        //             {
        //                 std::vector<std::vector<double>> alpha;
        //                 std::vector<std::vector<int>> visibility_robots;
        //                 generate_random_instance(n, m, alpha, visibility_robots, fixed_visibility_radius, gen);

        //                 AuctionAlgo<double> algo;
        //                 std::vector<int> auction_assignment;

        //                 auto start = std::chrono::high_resolution_clock::now();
        //                 algo.Start(n, m, alpha, visibility_robots, 1.0 / n, auction_assignment);
        //                 auto end = std::chrono::high_resolution_clock::now();
        //                 auction_total += std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() / 1000.0;

        //                 std::vector<int> hungarian_assignment;
        //                 std::vector<int> N_max(m, 1);
        //                 HungarianAlgo<double> hungarian_algo(n, m, alpha, N_max);

        //                 start = std::chrono::high_resolution_clock::now();
        //                 hungarian_algo.Start(hungarian_assignment);
        //                 end = std::chrono::high_resolution_clock::now();
        //                 hungarian_total += std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() / 1000.0;
        //             }

        //             auction_times_avg.push_back(auction_total / num_repeats);
        //             hungarian_times_avg.push_back(hungarian_total / num_repeats);

        //             std::cout << "Радиус: " << fixed_visibility_radius << " | Размер: " << n << "x" << m
        //                       << " | Аукционный: " << auction_times_avg.back() << " мс"
        //                       << " | Венгерский: " << hungarian_times_avg.back() << " мс" << std::endl;
        //         }
        //     }
        // }

        // {
        //     for(auto& n : sizes)
        //     {
        //         int m = n;

        //         int operation_count_auction = 0;
        //         int logic_operation_count_auction = 0;
        //         int change_operation_count_auction = 0;

        //         int operation_count_hungarian = 0;
        //         int logic_operation_count_hungarian = 0;
        //         int change_operation_count_hungarian = 0;

        //         int operation_count = 0;
        //         int logic_operation_count = 0;
        //         int change_operation_count = 0;

        //         for (int repeat = 0; repeat < num_repeats; ++repeat)
        //         {
        //             std::vector<std::vector<double>> alpha;
        //             std::vector<std::vector<int>> visibility_robots;
        //             generate_random_instance(n, m, alpha, visibility_robots, 200, gen);
        //             inverse_matrix(alpha);

        //             AuctionAlgo<double> algo;
        //             std::vector<int> auction_assignment;
        //             algo.Start(n, m, alpha, visibility_robots, 0.1, auction_assignment, std::nullopt, operation_count, logic_operation_count, change_operation_count);
        //             operation_count_auction += operation_count;
        //             logic_operation_count_auction += logic_operation_count;
        //             change_operation_count_auction += change_operation_count;

        //             std::vector<int> hungarian_assignment;
        //             std::vector<int> N_max(m, 1);
        //             HungarianAlgo<double> hungarian_algo(n, m, alpha, N_max);
        //             hungarian_algo.Start(hungarian_assignment, operation_count, logic_operation_count, change_operation_count);
        //             operation_count_hungarian += operation_count;
        //             logic_operation_count_hungarian += logic_operation_count;
        //             change_operation_count_hungarian += change_operation_count;
        //         }

        //         operation_count_auction_avg.push_back(operation_count_auction / num_repeats);
        //         logic_operation_count_auction_avg.push_back(logic_operation_count_auction / num_repeats);
        //         change_operation_count_auction_avg.push_back(change_operation_count_auction / num_repeats);

        //         operation_count_hungarian_avg.push_back(operation_count_hungarian / num_repeats);
        //         logic_operation_count_hungarian_avg.push_back(logic_operation_count_hungarian / num_repeats);
        //         change_operation_count_hungarian_avg.push_back(change_operation_count_hungarian / num_repeats);

        //         std::cout << "Размер: " << n << "x" << m
        //                   << "\nАукционный: " << operation_count_auction_avg.back() << " op. | " << logic_operation_count_auction_avg.back() << " l_op. | " << change_operation_count_auction_avg.back() << " ch_op."
        //                   << "\nВенгерский: " << operation_count_hungarian_avg.back() << " op. | " << logic_operation_count_hungarian_avg.back() << " l_op. | " << change_operation_count_hungarian_avg.back() << " ch_op.\n\n";
        //     }

        // }
    }

    // {
    //     for (int i = min_radius; i <= max_radius; ++i) {
    //         radii.push_back(i);
    //     }

    //     for(int i = 0; i < fixed_sizes.size(); ++i)
    //     {
    //         std::vector<double>& accuracy_avg = all_accuracy_avg[i];
    //         int fixed_size = fixed_sizes[i];

    //         for (auto& r : radii)
    //         {
    //             double accuracy_total = 0.0;

    //             for (int repeat = 0; repeat < num_repeats; ++repeat)
    //             {
    //                 std::vector<std::vector<double>> alpha;
    //                 std::vector<std::vector<int>> visibility_robots;
    //                 generate_random_instance(fixed_size, fixed_size, alpha, visibility_robots, r, gen);
    //                 inverse_matrix(alpha);

    //                 AuctionAlgo<double> algo;
    //                 std::vector<int> auction_assignment;
    //                 double auction_benefit = algo.Start(fixed_size, fixed_size, alpha, visibility_robots, 1e-2, auction_assignment);

    //                 std::vector<int> hungarian_assignment;
    //                 std::vector<int> N_max(fixed_size, 1);
    //                 HungarianAlgo<double> hungarian_algo(fixed_size, fixed_size, alpha, N_max);
    //                 double hungarian_benefit = hungarian_algo.Start(hungarian_assignment);

    //                 if (hungarian_benefit > 0) {
    //                     accuracy_total += auction_benefit / hungarian_benefit * 100;
    //                 } else {
    //                     accuracy_total += 0.0;
    //                 }
    //             }

    //             accuracy_avg.push_back(accuracy_total / num_repeats);
    //             std::cout << "Размер: " << fixed_size << " | Радиус: " << r
    //                       << " | Точность: " << accuracy_avg.back() << " %" << std::endl;
    //         }
    //     }
    // }

    // {

    //     for(int i = 0; i < fixed_visibility_radii.size(); ++i)
    //     {
    //         double fixed_visibility_radius = fixed_visibility_radii[i];
    //         auto& epsilon_times_avg = all_epsilon_times_avg[i];
    //         auto& epsilon_accuracy_avg = all_epsilon_accuracy_avg[i];

    //         for (auto& e : epsilons)
    //         {
    //             double auction_time_total = 0.0;
    //             double accuracy_total = 0.0;

    //             for (int repeat = 0; repeat < num_repeats; ++repeat)
    //             {
    //                 std::vector<std::vector<double>> alpha;
    //                 std::vector<std::vector<int>> visibility_robots;
    //                 generate_random_instance(fixed_size_eps, fixed_size_eps, alpha, visibility_robots, fixed_visibility_radius, gen);
    //                 inverse_matrix(alpha);

    //                 AuctionAlgo<double> algo;
    //                 std::vector<int> auction_assignment;

    //                 auto start = std::chrono::high_resolution_clock::now();
    //                 double auction_benefit = algo.Start(fixed_size_eps, fixed_size_eps, alpha, visibility_robots, e, auction_assignment);
    //                 auto end = std::chrono::high_resolution_clock::now();
    //                 auction_time_total += std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() / 1000.0;

    //                 std::vector<int> hungarian_assignment;
    //                 std::vector<int> N_max(fixed_size_eps, 1);
    //                 HungarianAlgo<double> hungarian_algo(fixed_size_eps, fixed_size_eps, alpha, N_max);
    //                 double hungarian_benefit = hungarian_algo.Start(hungarian_assignment);

    //                 if (hungarian_benefit > 0) {
    //                     accuracy_total += (auction_benefit / hungarian_benefit) * 100;
    //                 } else {
    //                     accuracy_total += 0.0;
    //                 }
    //             }

    //             epsilon_times_avg.push_back(auction_time_total / num_repeats);
    //             epsilon_accuracy_avg.push_back(accuracy_total / num_repeats);

    //             std::cout << "Радиус: "  << fixed_visibility_radius << " | ε: " << e
    //                       << " | Время аукционного: " << epsilon_times_avg.back() << " мс"
    //                       << " | Точность: " << epsilon_accuracy_avg.back() << " %" << std::endl;
    //         }
    //     }
    // }

    // {
    //     for(int i = 0; i < fixed_sizes2.size(); ++i)
    //     {
    //         std::vector<int> count_per_iteration;
    //         int rep = 100;

    //         for(int k = 0; k < rep; ++k)
    //         {
    //             double fixed_radius = 200;
    //             double size = fixed_sizes2[i];
    //             std::vector<double> value_per_iteration;

    //             std::vector<std::vector<double>> alpha;
    //             std::vector<std::vector<int>> visibility_robots;
    //             generate_random_instance(size, size, alpha, visibility_robots, fixed_radius, gen);
    //             inverse_matrix(alpha);

    //             AuctionAlgo<double> algo;
    //             std::vector<int> auction_assignment;
    //             algo.Start(size, size, alpha, visibility_robots, 1e-2, auction_assignment, value_per_iteration);

    //             std::vector<int> hungarian_assignment;
    //             std::vector<int> N_max(size, 1);
    //             HungarianAlgo<double> hungarian_algo(size, size, alpha, N_max);
    //             double hungarian_benefit = hungarian_algo.Start(hungarian_assignment);

    //             for(auto& val : value_per_iteration)
    //             {
    //                 val = (val / hungarian_benefit) * 100;
    //             }

    //             all_value_per_iteration[i].resize(std::max(value_per_iteration.size(), all_value_per_iteration[i].size()));
    //             count_per_iteration.resize(std::max(value_per_iteration.size(), all_value_per_iteration[i].size()));

    //             for(int j = 0; j < value_per_iteration.size(); ++j)
    //             {
    //                 all_value_per_iteration[i][j] += value_per_iteration[j];
    //                 count_per_iteration[j]++;
    //             }
    //         }

    //         for(int j = 0; j < all_value_per_iteration[i].size(); ++j)
    //         {
    //             all_value_per_iteration[i][j] += (count_per_iteration[0] - count_per_iteration[j]) * 100.0;
    //             all_value_per_iteration[i][j] /= rep;
    //         }
    //     }
    // }


    {
        for(int i = 0; i < fixed_sizes3.size(); ++i)
        {
            int rep = 100;

            auto& iterations_per_accuracy = all_iterations_per_accuracy[i];

            for(auto& e : epsilons)
            {
                long long total_iter = 0;

                for(int k = 0; k < rep; ++k)
                {
                    double fixed_radius = 200;
                    double size = fixed_sizes3[i];

                    std::vector<std::vector<double>> alpha;
                    std::vector<std::vector<int>> visibility_robots;
                    generate_random_instance(size, size, alpha, visibility_robots, fixed_radius, gen);
                    inverse_matrix(alpha);

                    AuctionAlgo<double> algo;
                    int number_of_iterations = 0;
                    std::vector<int> auction_assignment;
                    algo.Start(size, size, alpha, visibility_robots, e, auction_assignment, std::nullopt, std::nullopt, std::nullopt, std::nullopt, number_of_iterations);
                    total_iter += number_of_iterations;

                    std::vector<int> hungarian_assignment;
                    std::vector<int> N_max(size, 1);
                    HungarianAlgo<double> hungarian_algo(size, size, alpha, N_max);
                    double hungarian_benefit = hungarian_algo.Start(hungarian_assignment);
                }

                total_iter /= rep;
                iterations_per_accuracy.push_back(total_iter);
            }
        }
    }


    int completedCharts = 0;

    auto saveChart = [&completedCharts](QChartView* chartView, const QString& fileName)
    {
        chartView->setRenderHint(QPainter::Antialiasing);
        chartView->resize(800, 600);
        chartView->show();

        QTimer::singleShot(2000, [chartView, fileName, &completedCharts]() {
            QPixmap pixmap = chartView->grab();
            bool saved = pixmap.save(fileName, "PNG");
            if (saved)
            {
                QFile file(fileName);

                if (file.open(QIODevice::ReadOnly))
                {
                    completedCharts++;
                    std::cout << fileName.toStdString() << ", size: " << file.size() << " byte";
                    file.close();
                }
            }
        });
    };

    // QChartView *chartOpeartionView = new QChartView(create_operation_chart(sizes, operation_count_auction_avg, logic_operation_count_auction_avg, change_operation_count_auction_avg,
    //                                                                        operation_count_hungarian_avg, logic_operation_count_hungarian_avg, change_operation_count_hungarian_avg, 200));
    // saveChart(chartOpeartionView, "operation_chart.png");

    // QChartView *accuracyChartView = new QChartView(create_accuracy_chart(radii, all_accuracy_avg, fixed_sizes));
    // saveChart(accuracyChartView, "accuracy_chart.png");

    // QChartView *epsilonTimeChartView = new QChartView(create_epsilon_time_chart(epsilons, all_epsilon_times_avg, fixed_visibility_radii));
    // saveChart(epsilonTimeChartView, "epsilon_time_chart.png");

    // QChartView *epsilonAccuracyChartView = new QChartView(create_epsilon_accuracy_chart(epsilons, all_epsilon_accuracy_avg, fixed_visibility_radii));
    // saveChart(epsilonAccuracyChartView, "epsilon_accuracy_chart.png");

    // QChartView *AccuracyPerIterationChartView = new QChartView(create_acc_per_iteration_chart(all_value_per_iteration, fixed_sizes2));
    // saveChart(AccuracyPerIterationChartView, "accuracy_per_iteration_chart.png");


    QChartView *IterationsPerAccuracyChartView = new QChartView(create_iterations_per_acc_chart(all_iterations_per_accuracy, epsilons, fixed_sizes3));
    saveChart(IterationsPerAccuracyChartView, "iterations_per_accuracy_chart.png");

    return a.exec();
}
