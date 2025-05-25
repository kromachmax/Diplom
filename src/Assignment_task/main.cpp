#include <iostream>
#include <vector>
#include <cmath>
#ifdef _WIN32
#include <winsock2.h>
#include <windows.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#endif
#include <filesystem>
#include <fstream>

#include "AuctionAlgo.hpp"
#include "HungarianAlgo.hpp"
#include "httplib.h"
#include "json.hpp"

using json = nlohmann::json;

void open_browser(const std::string& url) {
#ifdef _WIN32
    ShellExecuteA(NULL, "open", url.c_str(), NULL, NULL, SW_SHOWNORMAL);
#else
    std::string command = "xdg-open " + url;
    int result = system(command.c_str());
    if (result != 0) {
        std::cerr << "Warning: Failed to open browser with xdg-open\n";
    }
#endif
}

void generate_instance(
    int n, int m,
    const std::vector<Point>& robot_coords,
    const std::vector<Point>& task_coords,
    std::vector<std::vector<double>>& alpha_auction,
    std::vector<std::vector<int>>& visibility_robots
    )
{
    alpha_auction.assign(n, std::vector<double>(m, -std::numeric_limits<double>::infinity()));
    visibility_robots.assign(n, std::vector<int>(n, 0));

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

int main()
{
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#else
    std::setlocale(LC_ALL, "en_US.UTF-8");
#endif

    AuctionAlgo<double> algo;
    PARAMETRS::min_utility = 1.0;
    PARAMETRS::max_utility = 30.0;
    PARAMETRS::visibility_radius = 100.0;

    httplib::Server svr;

    svr.Get("/", [](const httplib::Request&, httplib::Response& res) {
        std::ifstream file("index.html");
        if (file) {
            std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
            res.set_content(content, "text/html");
        } else {
            res.status = 404;
            res.set_content("File not found", "text/plain");
        }
    });

    svr.Post("/run_auction", [&](const httplib::Request& req, httplib::Response& res) {
        try {
            json input = json::parse(req.body);
            int n = input["n"].get<int>();
            int m = input["m"].get<int>();
            std::vector<Point> robot_coords(n);
            std::vector<Point> task_coords(m);

            std::cout << "Size: " << n << 'x' << m << '\n';

            std::cout << "Robot_coords: \n";
            for (int i = 0; i < n; ++i)
            {
                robot_coords[i] = {input["robot_coords"][i][0].get<double>(), input["robot_coords"][i][1].get<double>()};
                std::cout << robot_coords[i].x << ' ' << robot_coords[i].y << '\n';
            }

            std::cout << "Task_coords: \n";
            for (int j = 0; j < m; ++j)
            {
                task_coords[j] = {input["task_coords"][j][0].get<double>(), input["task_coords"][j][1].get<double>()};
                std::cout << task_coords[j].x << ' ' << task_coords[j].y << '\n';
            }

            std::vector<std::vector<double>> alpha;
            std::vector<std::vector<int>> visibility_robots;
            generate_instance(n, m, robot_coords, task_coords, alpha, visibility_robots);

            std::vector<int> auction_assignment;
            double auction_utility = algo.Start(n, m, alpha, visibility_robots, PARAMETRS::epsilon, auction_assignment);

            std::cout << "Auction assignment: ";
            for (int i = 0; i < auction_assignment.size(); ++i) {
                std::cout << auction_assignment[i] << " ";
            }
            std::cout << "\nAuction Utility: " << auction_utility << std::endl;

            std::vector<int> hungarian_assignment;
            std::vector<int> N_max(m, 1);
            HungarianAlgo<double> hungarian_algo(n, m, alpha, N_max);
            double hungarian_utility = hungarian_algo.Start(hungarian_assignment);

            std::cout << "Hungarian assignment: ";
            for (int i = 0; i < hungarian_assignment.size(); ++i) {
                std::cout << hungarian_assignment[i] << " ";
            }
            std::cout << "\nHungarian Utility: " << hungarian_utility << std::endl;

            json response;
            response["auction_assignment"] = auction_assignment;
            response["auction_utility"] = auction_utility;
            response["hungarian_assignment"] = hungarian_assignment;
            response["hungarian_utility"] = hungarian_utility;
            response["visibility_radius"] = PARAMETRS::visibility_radius;

            res.set_content(response.dump(), "application/json");
        }
        catch (const std::exception& e)
        {
            res.status = 500;
            std::cerr << "Error: " << e.what() << std::endl;
            res.set_content("{\"error\":\"" + std::string(e.what()) + "\"}", "application/json");
        }
    });

    svr.Options("/run_auction", [](const httplib::Request&, httplib::Response& res) {
        res.set_header("Access-Control-Allow-Origin", "*");
        res.set_header("Access-Control-Allow-Methods", "POST, GET, OPTIONS");
        res.set_header("Access-Control-Allow-Headers", "Content-Type");
        res.status = 200;
    });

    if (!std::filesystem::exists("index.html")) {
        std::cerr << "Error: index.html not found in build folder\n";
        return 1;
    }

    open_browser("http://localhost:8000");

    std::cout << "Starting server at http://localhost:8000\n";
    svr.listen("localhost", 8000);

    return 0;
}
