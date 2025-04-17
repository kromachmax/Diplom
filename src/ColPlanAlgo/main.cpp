#include <QtWidgets/QApplication>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>
#include <QtCharts/QLogValueAxis>
#include <QMainWindow>
#include <QDebug>
#include <iostream>
#include <vector>
#include <random>
#include <chrono>
#include <ranges>
#include <algorithm>

#include "COI_3_7.hpp"
#include "COI_3_9.hpp"
#include "COI_3_1.hpp"
#include "solving_LP.hpp"
#include "HungarianAlgo.hpp"
#include "AuctionAlgo.hpp"

#define LINEAR
#define COI_3_1_def

QT_CHARTS_USE_NAMESPACE
using namespace std;
using namespace std::chrono;


int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    std::random_device rd;
    std::mt19937 gen(rd());

    std::uniform_real_distribution<> valueDist(1.0, 30.0);
    std::uniform_int_distribution<> nDist(1, 1);

    const int NUM_ITERATIONS = 100;

    //// ==================================================================

    std::vector<int> matrixSizes; // Размеры матриц (n * m)

    std::vector<double> answers_hunAlgo;
    std::vector<double> answers_lpAlgo;
#ifdef COI_3_1_def
    std::vector<double> answers_3_1;
#endif
    std::vector<double> answers_3_7;
    std::vector<double> answers_3_9;
    std::vector<double> answers_Auction;


    std::vector<long long> times_hunAlgo;
    std::vector<long long> times_lpAlgo;
#ifdef COI_3_1_def
    std::vector<long long> times_3_1;  // Время выполнения COI_3_1
#endif
    std::vector<long long> times_3_7;  // Время выполнения COI_3_7
    std::vector<long long> times_3_9;  // Время выполнения COI_3_9
    std::vector<long long> times_Auction;

    std::vector<double> answer_diffs_1;  // Разница решений (answer_3_9 - answer_3_7)
#ifdef COI_3_1_def
    std::vector<double> answer_diffs_2; // Разница решений (answer_3_1 - answer_3_9)
#endif
    std::vector<double> answer_diffs_3; // Разница решений (answer_LP - answer_3_9)
    std::vector<double> answer_diffs_4; // Разница решений (answer_LP - answer_3_1)
    std::vector<double> answer_diffs_5;

    //// ==================================================================

    for(int iter = 2; iter < NUM_ITERATIONS; iter++)
    {
        int n = iter + 1;
        int m = n;

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

        try
        {
            //// ==================================================================

            auto start_LP = high_resolution_clock::now();

            double answer_LP = solveAssignmentProblem_LP(D);

            auto end_LP = high_resolution_clock::now();
            auto duration_LP = duration_cast<microseconds>(end_LP - start_LP);

            //// ==================================================================

            auto start_hunAlgo = high_resolution_clock::now();

            HungarionAlgo<double> hunAlgo;
            hunAlgo.Update(n, m, D, N);
            double answer_hunAlgo = hunAlgo.Start();

            auto end_hunAlgo = high_resolution_clock::now();
            auto duration_hunAlgo = duration_cast<microseconds>(end_hunAlgo - start_hunAlgo);

            //// ==================================================================

#ifdef COI_3_1_def
            auto start_3_1 = high_resolution_clock::now();

            COI_3_1<double> coi_3_1;
            coi_3_1.Update(n, m, D, N);
            double answer_3_1 = coi_3_1.Start();
            auto end_3_1 = high_resolution_clock::now();
            auto duration_3_1 = duration_cast<microseconds>(end_3_1 - start_3_1);
#endif

            //// ==================================================================

            auto start_3_7 = high_resolution_clock::now();

            COI_3_7<double> coi_3_7;
            coi_3_7.Update(n, m, D, N);
            double answer_3_7 = coi_3_7.Start();

            auto end_3_7 = high_resolution_clock::now();
            auto duration_3_7 = duration_cast<microseconds>(end_3_7 - start_3_7);

            //// ==================================================================

            auto start_3_9 = high_resolution_clock::now();

            COI_3_9<double> coi_3_9;
            coi_3_9.Update(n, m, D, N);
            double answer_3_9 = coi_3_9.Start();

            auto end_3_9 = high_resolution_clock::now();
            auto duration_3_9 = duration_cast<microseconds>(end_3_9 - start_3_9);

            //// ==================================================================

            auto start_Auction = high_resolution_clock::now();

            AuctionAlgo<double> auctionAlgo;
            std::vector<int> assigment(n, 0);
            double eps = 0.001;
            double answer_Auction = auctionAlgo.Start(n, m, D, eps, assigment);

            auto end_Auction = high_resolution_clock::now();
            auto duration_Auction = duration_cast<microseconds>(end_Auction - start_Auction);

            //// ==================================================================

            matrixSizes.push_back(n);

            answers_hunAlgo.push_back(answer_hunAlgo);
            answers_lpAlgo.push_back(answer_LP);
#ifdef COI_3_1_def
            answers_3_1.push_back(answer_3_1);
#endif
            answers_3_7.push_back(answer_3_7);
            answers_3_9.push_back(answer_3_9);
            answers_Auction.push_back(answer_Auction);



            times_hunAlgo.push_back(duration_hunAlgo.count());
            times_lpAlgo.push_back(duration_LP.count());
#ifdef COI_3_1_def
            times_3_1.push_back(duration_3_1.count());
#endif
            times_3_7.push_back(duration_3_7.count());
            times_3_9.push_back(duration_3_9.count());
            times_Auction.push_back(duration_Auction.count());

            answer_diffs_1.push_back(answer_3_9 - answer_3_7);
#ifdef COI_3_1_def
            answer_diffs_2.push_back(answer_3_1 - answer_3_9);
#endif
            answer_diffs_3.push_back(answer_hunAlgo - answer_3_9);
            answer_diffs_4.push_back(answer_hunAlgo - answer_3_1);
            answer_diffs_5.push_back(answer_hunAlgo - answer_Auction);

            //// ==================================================================

            std::cout << "\nIteration " << iter + 1 << ":" << std::endl;
            std::cout << "Matrix size: " << n << " x " << m << std::endl;
            std::cout << "LP_Answer: " << answer_LP <<  ", Time: " << duration_LP.count() << " microseconds" << std::endl;
            std::cout << "Hungarian_Answer: " << answer_hunAlgo <<  ", Time: " << duration_hunAlgo.count() << " microseconds" << std::endl;
#ifdef COI_3_1_def
            std::cout << "COI_3_1 Answer: " << answer_3_1 << ", Time: " << duration_3_1.count() << " microseconds" << std::endl;
#endif
            std::cout << "COI_3_7 Answer: " << answer_3_7 << ", Time: " << duration_3_7.count() << " microseconds" << std::endl;
            std::cout << "COI_3_9 Answer: " << answer_3_9 << ", Time: " << duration_3_9.count() << " microseconds" << std::endl;
            std::cout << "Auction_Answer: " << answer_Auction << ", Time: " << duration_Auction.count() << " microseconds" << std::endl;
        }
        catch (const std::exception& e)
        {
            std::cout << "General exception: " << e.what() << '\n';
            exit(EXIT_FAILURE);
        }
    }

#ifdef COI_3_1_def
    auto max_time_3_1 = *std::ranges::max_element(times_3_1);
#endif
    auto max_time_3_7 = *std::ranges::max_element(times_3_7);
    auto max_time_3_9 = *std::ranges::max_element(times_3_9);
    auto max_time_LP  = *std::ranges::max_element(times_lpAlgo);
    auto max_time_hunAlgo = *std::ranges::max_element(times_hunAlgo);
    auto max_time_Auction = *std::ranges::max_element(times_Auction);

    auto max_diff_1 =     *std::max_element(answer_diffs_1.begin(), answer_diffs_1.end(),
                                            [](double a, double b) { return std::abs(a) < std::abs(b); });
#ifdef COI_3_1_def
    auto max_diff_2 =     *std::max_element(answer_diffs_2.begin(), answer_diffs_2.end(),
                                            [](double a, double b) { return std::abs(a) < std::abs(b); });
#endif

    auto max_diff_3 =     *std::max_element(answer_diffs_3.begin(), answer_diffs_3.end(),
                                            [](double a, double b) { return std::abs(a) < std::abs(b); });

    auto max_diff_4 =     *std::max_element(answer_diffs_4.begin(), answer_diffs_4.end(),
                                            [](double a, double b) { return std::abs(a) < std::abs(b); });

    auto max_diff_5 =     *std::max_element(answer_diffs_5.begin(), answer_diffs_5.end(),
                                            [](double a, double b) { return std::abs(a) < std::abs(b); });


    //// ============================================================================================================

#ifdef RELEASE

    QLineSeries *series_time_LP = new QLineSeries();
    series_time_LP->setName("LP Time");
    QLineSeries *series_time_hunAlgo = new QLineSeries();
    series_time_hunAlgo->setName("HungarianAlgo Time");

    for (size_t i = 0; i < matrixSizes.size(); ++i)
    {
        series_time_LP->append(matrixSizes[i], times_lpAlgo[i]);
        series_time_hunAlgo->append(matrixSizes[i], times_hunAlgo[i]);
    }

    QChart *chart_time = new QChart();

    chart_time->addSeries(series_time_LP);
    chart_time->addSeries(series_time_hunAlgo);
    chart_time->setTitle("Execution Time vs Matrix Size");
    chart_time->legend()->setAlignment(Qt::AlignBottom);

    QValueAxis *axisX_time = new QValueAxis();
    axisX_time->setTitleText("Matrix Size (n)");
    axisX_time->setRange(0, NUM_ITERATIONS);
    axisX_time->setLabelFormat("%.0f");

    QLogValueAxis *axisY_time = new QLogValueAxis();
    axisY_time->setTitleText("Time (microseconds)");
    axisY_time->setBase(10); // Основание логарифма (10 или e)
    axisY_time->setRange(0.01, std::max<long long>(max_time_hunAlgo, max_time_LP) );

    axisY_time->setLabelFormat("%.0e"); // Формат: 1e+06, 1e+05, ... 1e+00

    chart_time->addAxis(axisX_time, Qt::AlignBottom);
    chart_time->addAxis(axisY_time, Qt::AlignLeft);

    series_time_LP->attachAxis(axisX_time);
    series_time_LP->attachAxis(axisY_time);
    series_time_hunAlgo->attachAxis(axisX_time);
    series_time_hunAlgo->attachAxis(axisY_time);

    QChartView *chartView_time = new QChartView(chart_time);
    chartView_time->setRenderHint(QPainter::Antialiasing);

    QMainWindow window_time;
    window_time.setCentralWidget(chartView_time);
    window_time.resize(800, 600);
    window_time.setWindowTitle("Execution Time vs Matrix Size");
    window_time.show();


    //// ============================================================================================================

#endif

#ifdef RELEASE

    QLineSeries *series_time_hunAlgo_2 = new QLineSeries();
    series_time_hunAlgo_2->setName("HungarianAlgo Time");
#ifdef COI_3_1_def
    QLineSeries *series_time_3_1 = new QLineSeries();
    series_time_3_1->setName("COI_3_1 Time");
#endif
    QLineSeries *series_time_3_7 = new QLineSeries();
    series_time_3_7->setName("COI_3_7 Time");
    QLineSeries *series_time_3_9 = new QLineSeries();
    series_time_3_9->setName("COI_3_9 Time");
    QLineSeries *series_time_Auction = new QLineSeries();
    series_time_Auction->setName("Auction Time");

    for (size_t i = 0; i < matrixSizes.size(); ++i)
    {
        series_time_hunAlgo_2->append(matrixSizes[i], times_hunAlgo[i]);
#ifdef COI_3_1_def
        series_time_3_1->append(matrixSizes[i], times_3_1[i]);
#endif
        series_time_3_7->append(matrixSizes[i], times_3_7[i]);
        series_time_3_9->append(matrixSizes[i], times_3_9[i]);
        series_time_Auction->append(matrixSizes[i], times_Auction[i]);
    }

    QChart *chart_time_2 = new QChart();

    chart_time_2->addSeries(series_time_hunAlgo_2);
#ifdef COI_3_1_def
    chart_time_2->addSeries(series_time_3_1);
#endif
    chart_time_2->addSeries(series_time_3_7);
    chart_time_2->addSeries(series_time_3_9);
    chart_time_2->addSeries(series_time_Auction);
    chart_time_2->setTitle("Execution Time vs Matrix Size");
    chart_time_2->legend()->setAlignment(Qt::AlignBottom);

    QValueAxis *axisX_time_2 = new QValueAxis();
    axisX_time_2->setTitleText("Matrix Size (n)");
    axisX_time_2->setRange(0, NUM_ITERATIONS);
    axisX_time_2->setLabelFormat("%.0f");


#ifdef LINEAR
    QValueAxis *axisY_time_2 = new QValueAxis();
    axisY_time_2->setTitleText("Time (microseconds)");
#ifdef COI_3_1_def
    axisY_time->setRange( 0, std::max<long long>({max_time_3_9, max_time_3_7, max_time_hunAlgo, max_time_3_1, max_time_Auction}) );
    axisX_time_2->setLabelFormat("%.0e");
#else
    axisY_time_2->setRange(0, std::max<long long>({max_time_3_7, max_time_3_9, max_time_hunAlgo}) );
#endif
#else
    QLogValueAxis *axisY_time_2 = new QLogValueAxis();
    axisY_time_2->setTitleText("Time (microseconds)");
    axisY_time_2->setBase(2); // Основание логарифма (10 или e)
#ifdef COI_3_1_def
    axisY_time->setRange( 0, std::max<long long>({max_time_3_7, max_time_3_9, max_time_hunAlgo, max_time_3_1}) );
#else
    axisY_time_2->setRange(0.1, std::max<long long>({max_time_3_7, max_time_3_9, max_time_hunAlgo}) );
#endif
    axisY_time_2->setLabelFormat("%.0e"); // Формат: 1e+06, 1e+05, ... 1e+00
#endif

    chart_time_2->addAxis(axisX_time_2, Qt::AlignBottom);
    chart_time_2->addAxis(axisY_time_2, Qt::AlignLeft);

    series_time_hunAlgo_2->attachAxis(axisX_time_2);
    series_time_hunAlgo_2->attachAxis(axisY_time_2);
#ifdef COI_3_1_def
    series_time_3_1->attachAxis(axisX_time);
    series_time_3_1->attachAxis(axisY_time);
#endif
    series_time_3_7->attachAxis(axisX_time_2);
    series_time_3_7->attachAxis(axisY_time_2);
    series_time_3_9->attachAxis(axisX_time_2);
    series_time_3_9->attachAxis(axisY_time_2);

    QChartView *chartView_time_2 = new QChartView(chart_time_2);
    chartView_time_2->setRenderHint(QPainter::Antialiasing);

    QMainWindow window_time_2;
    window_time_2.setCentralWidget(chartView_time_2);
    window_time_2.resize(800, 600);
    window_time_2.setWindowTitle("Execution Time vs Matrix Size");
    window_time_2.show();


    //// ============================================================================================================


    QLineSeries *series_diff_1 = new QLineSeries();
    series_diff_1->setName("Answer Difference (COI_3_9 - COI_3_7)");

    for (size_t i = 0; i < matrixSizes.size(); ++i)
    {
        series_diff_1->append(matrixSizes[i], answer_diffs_1[i]);
    }

    QChart *chart_diff_1 = new QChart();
    chart_diff_1->addSeries(series_diff_1);
    chart_diff_1->setTitle("Answer Difference vs Matrix Size");
    chart_diff_1->legend()->setAlignment(Qt::AlignBottom);

    QValueAxis *axisX_diff_1 = new QValueAxis();
    axisX_diff_1->setTitleText("Matrix Size (n)");
    axisX_diff_1->setRange(0, NUM_ITERATIONS);
    axisX_diff_1->setLabelFormat("%.0f");

    QValueAxis *axisY_diff_1 = new QValueAxis();
    axisY_diff_1->setTitleText("Answer Difference");
    axisY_diff_1->setRange(-max_diff_1, max_diff_1);
    axisY_diff_1->setLabelFormat("%.0f");

    chart_diff_1->addAxis(axisX_diff_1, Qt::AlignBottom);
    chart_diff_1->addAxis(axisY_diff_1, Qt::AlignLeft);

    series_diff_1->attachAxis(axisX_diff_1);
    series_diff_1->attachAxis(axisY_diff_1);

    QChartView *chartView_diff_1 = new QChartView(chart_diff_1);
    chartView_diff_1->setRenderHint(QPainter::Antialiasing);

    QMainWindow window_diff_1;
    window_diff_1.setCentralWidget(chartView_diff_1);
    window_diff_1.resize(800, 600);
    window_diff_1.setWindowTitle("Answer Difference vs Matrix Size");
    window_diff_1.show();


    //// ============================================================================================================

#ifdef COI_3_1_def
    QLineSeries *series_diff_2 = new QLineSeries();
    series_diff_2->setName("Answer Difference (COI_3_1 - COI_3_9)");

    for (size_t i = 0; i < matrixSizes.size(); ++i)
    {
        series_diff_2->append(matrixSizes[i], answer_diffs_2[i]);
    }

    QChart *chart_diff_2 = new QChart();
    chart_diff_2->addSeries(series_diff_2);
    chart_diff_2->setTitle("Answer Difference vs Matrix Size");
    chart_diff_2->legend()->setAlignment(Qt::AlignBottom);

    QValueAxis *axisX_diff_2 = new QValueAxis();
    axisX_diff_2->setTitleText("Matrix Size (n)");
    axisX_diff_2->setRange(0, NUM_ITERATIONS);
    axisX_diff_2->setLabelFormat("%.0f");

    QValueAxis *axisY_diff_2 = new QValueAxis();
    axisY_diff_2->setTitleText("Answer Difference");
    axisY_diff_2->setRange(-max_diff_2, max_diff_2);
    axisY_diff_2->setLabelFormat("%.0f");

    chart_diff_2->addAxis(axisX_diff_2, Qt::AlignBottom);
    chart_diff_2->addAxis(axisY_diff_2, Qt::AlignLeft);

    series_diff_2->attachAxis(axisX_diff_2);
    series_diff_2->attachAxis(axisY_diff_2);

    QChartView *chartView_diff_2 = new QChartView(chart_diff_2);
    chartView_diff_2->setRenderHint(QPainter::Antialiasing);

    QMainWindow window_diff_2;
    window_diff_2.setCentralWidget(chartView_diff_2);
    window_diff_2.resize(800, 600);
    window_diff_2.setWindowTitle("Answer Difference vs Matrix Size");
    window_diff_2.show();
#endif


    //// ============================================================================================================


    QLineSeries *series_diff_3 = new QLineSeries();
    series_diff_3->setName("Answer Difference (LP - COI_3_9)");

    for (size_t i = 0; i < matrixSizes.size(); ++i)
    {
        series_diff_3->append(matrixSizes[i], answer_diffs_3[i]);
    }

    QChart *chart_diff_3 = new QChart();
    chart_diff_3->addSeries(series_diff_3);
    chart_diff_3->setTitle("Answer Difference vs Matrix Size");
    chart_diff_3->legend()->setAlignment(Qt::AlignBottom);

    QValueAxis *axisX_diff_3 = new QValueAxis();
    axisX_diff_3->setTitleText("Matrix Size (n)");
    axisX_diff_3->setRange(0, NUM_ITERATIONS);
    axisX_diff_3->setLabelFormat("%.0f");

    QValueAxis *axisY_diff_3 = new QValueAxis();
    axisY_diff_3->setTitleText("Answer Difference");
    axisY_diff_3->setRange(-max_diff_3, max_diff_3);
    axisY_diff_3->setLabelFormat("%.0f");

    chart_diff_3->addAxis(axisX_diff_3, Qt::AlignBottom);
    chart_diff_3->addAxis(axisY_diff_3, Qt::AlignLeft);

    series_diff_3->attachAxis(axisX_diff_3);
    series_diff_3->attachAxis(axisY_diff_3);

    QChartView *chartView_diff_3 = new QChartView(chart_diff_3);
    chartView_diff_3->setRenderHint(QPainter::Antialiasing);

    QMainWindow window_diff_3;
    window_diff_3.setCentralWidget(chartView_diff_3);
    window_diff_3.resize(800, 600);
    window_diff_3.setWindowTitle("Answer Difference vs Matrix Size");
    window_diff_3.show();

    //// ============================================================================================================


    QLineSeries *series_diff_4 = new QLineSeries();
    series_diff_4->setName("Answer Difference (LP - COI_3_1)");

    for (size_t i = 0; i < matrixSizes.size(); ++i)
    {
        series_diff_4->append(matrixSizes[i], answer_diffs_4[i]);
    }

    QChart *chart_diff_4 = new QChart();
    chart_diff_4->addSeries(series_diff_4);
    chart_diff_4->setTitle("Answer Difference vs Matrix Size");
    chart_diff_4->legend()->setAlignment(Qt::AlignBottom);

    QValueAxis *axisX_diff_4 = new QValueAxis();
    axisX_diff_4->setTitleText("Matrix Size (n)");
    axisX_diff_4->setRange(0, NUM_ITERATIONS);
    axisX_diff_4->setLabelFormat("%.0f");

    QValueAxis *axisY_diff_4 = new QValueAxis();
    axisY_diff_4->setTitleText("Answer Difference");
    axisY_diff_4->setRange(-max_diff_4, max_diff_4);
    axisY_diff_4->setLabelFormat("%.0f");

    chart_diff_4->addAxis(axisX_diff_4, Qt::AlignBottom);
    chart_diff_4->addAxis(axisY_diff_4, Qt::AlignLeft);

    series_diff_4->attachAxis(axisX_diff_4);
    series_diff_4->attachAxis(axisY_diff_4);

    QChartView *chartView_diff_4 = new QChartView(chart_diff_4);
    chartView_diff_4->setRenderHint(QPainter::Antialiasing);

    QMainWindow window_diff_4;
    window_diff_4.setCentralWidget(chartView_diff_4);
    window_diff_4.resize(800, 600);
    window_diff_4.setWindowTitle("Answer Difference vs Matrix Size");
    window_diff_4.show();

    //// ============================================================================================================


    QLineSeries *series_diff_5 = new QLineSeries();
    series_diff_5->setName("Answer Difference (LP - Auction)");

    for (size_t i = 0; i < matrixSizes.size(); ++i)
    {
        series_diff_5->append(matrixSizes[i], answer_diffs_5[i]);
    }

    QChart *chart_diff_5 = new QChart();
    chart_diff_5->addSeries(series_diff_5);
    chart_diff_5->setTitle("Answer Difference vs Matrix Size");
    chart_diff_5->legend()->setAlignment(Qt::AlignBottom);

    QValueAxis *axisX_diff_5 = new QValueAxis();
    axisX_diff_5->setTitleText("Matrix Size (n)");
    axisX_diff_5->setRange(0, NUM_ITERATIONS);
    axisX_diff_5->setLabelFormat("%.0f");

    QValueAxis *axisY_diff_5 = new QValueAxis();
    axisY_diff_5->setTitleText("Answer Difference");
    axisY_diff_5->setRange(-max_diff_5, max_diff_5);
    axisY_diff_5->setLabelFormat("%.0f");

    chart_diff_5->addAxis(axisX_diff_5, Qt::AlignBottom);
    chart_diff_5->addAxis(axisY_diff_5, Qt::AlignLeft);

    series_diff_5->attachAxis(axisX_diff_5);
    series_diff_5->attachAxis(axisY_diff_5);

    QChartView *chartView_diff_5 = new QChartView(chart_diff_5);
    chartView_diff_5->setRenderHint(QPainter::Antialiasing);

    QMainWindow window_diff_5;
    window_diff_5.setCentralWidget(chartView_diff_5);
    window_diff_5.resize(800, 600);
    window_diff_5.setWindowTitle("Answer Difference vs Matrix Size");
    window_diff_5.show();


#endif

    return app.exec();
}
