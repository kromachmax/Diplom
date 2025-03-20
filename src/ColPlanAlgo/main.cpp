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

//15.1801 19.9286 11.4982 13.1915 12.1426 15.2867 8.46667 21.214 1.68299 0.364091 0.773497 1.15186 11.3952 10.1304 12.2005 8.42385 2.74773 4.85009 21.0053 11.308
//16.5437 1.03732 5.34964 7.53279 2.34668 15.0859 9.62777 0.607328 0.489124 22.116 21.4557 3.4701 2.78653 18.1885 13.3989 7.27855 8.11205 5.81433 2.16527 1.57359
//4.75808 17.1115 3.94969 7.51811 12.6807 15.0594 20.1541 6.25476 4.41528 22.81 17.1251 15.9022 19.5629 16.9879 1.08951 3.36232 9.34335 3.34572 0.59654 5.71244
//17.9353 9.63706 12.8318 17.8462 17.9507 0.098002 0.0513347 6.38559 15.7037 16.5146 16.6449 12.1591 6.94461 15.2575 18.7072 13.2817 11.1944 16.6836 10.3776 22.5375
//15.6888 19.5776 16.2175 14.6868 8.68653 10.8959 0.328971 14.1583 1.33668 11.8117 3.21393 10.516 16.1077 22.9917 3.97456 18.954 18.94 2.07398 15.9448 17.9719
//17.1891 11.8002 22.7068 22.04 19.0636 9.49567 9.95297 16.5755 11.8217 12.0727 5.64711 22.2533 3.10236 15.331 3.81129 12.4413 19.1195 19.989 7.5649 12.5414
//18.3834 20.5539 20.3642 4.92147 5.05474 6.31747 0.8708 1.98804 6.32832 22.1166 2.84343 19.8095 16.6949 11.2806 13.8324 12.157 2.70123 22.0499 5.54039 21.7709
//6.88556 17.7856 1.73327 14.047 1.75774 18.6697 7.20842 9.46635 18.0649 2.88611 14.0414 9.8703 15.2815 8.60941 1.74551 4.7611 13.8784 18.9636 1.72264 2.54564
//10.7627 15.9396 7.09377 18.4165 14.6324 11.7605 14.9188 18.9238 14.3706 19.0316 10.1316 18.8758 20.1784 15.2121 18.2502 19.8991 4.1647 11.8264 10.9135 5.94409
//11.9211 12.8016 5.13036 20.0261 11.3233 20.0223 7.05673 12.348 3.96981 1.94757 5.68523 22.8642 21.8152 11.3541 16.575 15.2513 3.4442 9.26815 7.22158 8.67424
//19.7987 9.17116 16.8526 17.3613 7.05938 12.196 9.51671 6.3265 15.7774 10.0339 0.317228 12.4463 11.5054 14.8396 9.70581 19.7801 5.78476 4.02777 15.9784 15.6436
//4.39874 1.32707 22.4874 9.27689 5.27578 12.9278 5.04808 2.95551 22.3304 19.9615 7.80792 22.5488 19.298 16.9311 11.4664 15.114 22.9144 18.7324 8.2234 2.58047
//2.37104 7.18611 8.08515 10.6044 0.0486703 5.99501 0.440989 11.7495 17.4392 3.62116 10.7906 15.6547 0.296711 4.94994 5.81303 12.0724 21.1077 3.68174 4.57493 5.57661
//0.622851 7.92945 2.78731 0.0404399 8.57427 15.4532 11.7417 7.33581 22.5905 17.7625 8.08657 11.9633 21.0613 14.4158 15.1637 5.16834 1.32441 10.5803 12.0048 8.56175
//3.27876 18.6021 5.07024 9.69528 12.4266 7.24204 7.25204 14.3215 10.3726 15.5863 15.9929 8.01816 19.8428 14.9775 15.5666 21.9894 6.18827 12.2304 22.9939 13.7633
//18.1102 3.66755 7.8907 18.6532 8.03767 19.3568 8.63718 6.08272 14.094 7.28613 4.18651 9.05489 13.3289 6.21469 5.63247 20.9734 1.55784 15.1215 0.734673 19.4366
//12.6251 8.00475 3.69809 2.94251 5.12712 5.96226 19.0847 14.134 1.62413 0.818601 17.3563 11.6716 6.78229 19.2528 14.238 11.6016 1.58897 6.89929 19.6904 1.77229
//8.14988 15.4947 21.4047 2.4966 13.6387 16.257 22.7702 0.181731 1.32224 21.7904 20.2104 14.4193 6.40159 3.22728 0.544135 20.0256 14.1866 3.74397 18.7672 19.9035
//10.2284 0.989243 12.1818 8.23055 19.8572 5.13508 22.9411 7.88469 5.77655 7.34341 19.7221 13.5145 4.09324 11.1619 17.9688 7.76687 17.9095 4.35251 7.17841 17.8747
//11.3984 11.6139 6.69259 10.6757 8.80724 6.44855 4.63459 2.52142 10.9388 5.16773 14.8764 12.863 3.9465 19.4162 12.3623 13.3606 20.8456 8.98278 8.10143 14.8737

//10 10 4 7 8 8 3 5 3 2 1 10 9 7 10 8 3 7 5 1

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    std::random_device rd;
    std::mt19937 gen(rd());

    std::uniform_real_distribution<> valueDist(0.0, 23.0);
    std::uniform_int_distribution<> nDist(1, 10);

    const int NUM_ITERATIONS = 1000;

    std::vector<int> matrixSizes; // Размеры матриц (n * m)
    std::vector<double> answers_3_7;
    std::vector<double> answers_3_9;
    std::vector<long long> times_3_7; // Время выполнения COI_3_7
    std::vector<long long> times_3_9; // Время выполнения COI_3_9
    std::vector<double> answer_diffs; // Разница решений (answer_3_9 - answer_3_7)

    for(int iter = 0; iter < NUM_ITERATIONS; iter++)
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

#ifdef DEBUG
        std::cout << '\n';
        for (std::size_t i = 0; i < D.size(); ++i)
        {
            for (std::size_t j = 0; j < D[0].size(); ++j)
            {
                std::cout << D[i][j] << ' ';
            }
            std::cout << '\n';
        }
        std::cout << '\n';

        for(std::size_t i = 0; i < N.size(); ++i)
        {
            std::cout << N[i] << ' ';
        }
        std::cout << '\n';
#endif

        try
        {
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


#ifdef DEBUG
            std::cout << "\nIteration " << iter + 1 << ":" << std::endl;
            std::cout << "Matrix size: " << n << " x " << m << std::endl;
            std::cout << "COI_3_7 Answer: " << answer_3_7 << ", Time: " << duration_3_7.count() << " microseconds" << std::endl;
            std::cout << "COI_3_9 Answer: " << answer_3_9 << ", Time: " << duration_3_9.count() << " microseconds" << std::endl;
#endif
        }
        catch (const std::exception& e)
        {
            std::cout << "General exception: " << e.what() << '\n';
            exit(EXIT_FAILURE);
        }
    }


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
    axisX_time->setTitleText("Matrix Size (n)");
    axisX_time->setRange(0, NUM_ITERATIONS);
    axisX_time->setLabelFormat("%.0f");

    QValueAxis *axisY_time = new QValueAxis();
    axisY_time->setTitleText("Time (microseconds)");
    axisY_time->setRange(0, 50000);
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
    axisX_diff->setTitleText("Matrix Size (n)");
    axisX_diff->setRange(0, NUM_ITERATIONS);
    axisX_diff->setLabelFormat("%.0f");

    QValueAxis *axisY_diff = new QValueAxis();
    axisY_diff->setTitleText("Answer Difference");
    axisY_diff->setRange(-100, 100);
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
