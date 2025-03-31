#include <glpk.h>
#include <iostream>
#include <vector>

template<typename T>
double solveAssignmentProblem_LP(std::vector<std::vector<T>>& d)
{
    glp_prob *lp = glp_create_prob();
    glp_set_prob_name(lp, "AssignmentProblem");
    glp_set_obj_dir(lp, GLP_MAX); // Максимизация

    const int n = d.size();

    int num_vars = n * n;
    glp_add_cols(lp, num_vars);

    for (int j = 0; j < n; j++)
    {
        for (int l = 0; l < n; l++)
        {
            int idx = j * n + l + 1; // Индекс переменной (1-based в GLPK)
            glp_set_col_name(lp, idx, ("n_" + std::to_string(j+1) + "_" + std::to_string(l+1)).c_str());
            glp_set_col_bnds(lp, idx, GLP_DB, 0.0, 1.0); // 0 <= n_{j,l} <= 1
            glp_set_col_kind(lp, idx, GLP_BV); // Бинарная переменная (0 или 1)
            glp_set_obj_coef(lp, idx, d[j][l]); // Коэффициент в целевой функции
        }
    }

    glp_add_rows(lp, n); // N строк для ограничений (3.4)
    for (int j = 0; j < n; j++)
    {
        glp_set_row_name(lp, j + 1, ("robot_" + std::to_string(j+1)).c_str());
        glp_set_row_bnds(lp, j + 1, GLP_FX, 1.0, 1.0); // Сумма = 1
    }

    glp_add_rows(lp, n); // Ещё N строк для ограничений (3.5)
    for (int l = 0; l < n; l++)
    {
        glp_set_row_name(lp, n + l + 1, ("target_" + std::to_string(l+1)).c_str());
        glp_set_row_bnds(lp, n + l + 1, GLP_FX, 1.0, 1.0); // Сумма = 1
    }

    // Заполняем матрицу ограничений
    std::vector<int> ia(1); // Индексы строк
    std::vector<int> ja(1); // Индексы столбцов
    std::vector<double> ar(1); // Значения
    int idx = 1;

    for (int j = 0; j < n; j++)
    {
        for (int l = 0; l < n; l++)
        {
            ia.push_back(j + 1); // Строка (ограничение для робота j)
            ja.push_back(j * n + l + 1); // Столбец (переменная n_{j,l})
            ar.push_back(1.0); // Коэффициент
            idx++;
        }
    }

    for (int l = 0; l < n; l++)
    {
        for (int j = 0; j < n; j++)
        {
            ia.push_back(n + l + 1); // Строка (ограничение для цели l)
            ja.push_back(j * n + l + 1); // Столбец (переменная n_{j,l})
            ar.push_back(1.0); // Коэффициент
            idx++;
        }
    }

    // Загружаем матрицу ограничений
    glp_load_matrix(lp, idx - 1, ia.data(), ja.data(), ar.data());

    // Настраиваем параметры для отключения вывода
    glp_smcp parm_simplex;
    glp_init_smcp(&parm_simplex);
    parm_simplex.msg_lev = GLP_MSG_OFF; // Отключаем сообщения симплекс-метода

    glp_iocp parm_intopt;
    glp_init_iocp(&parm_intopt);
    parm_intopt.msg_lev = GLP_MSG_OFF; // Отключаем сообщения MILP

    // Решаем задачу
    glp_simplex(lp, &parm_simplex); // Сначала симплекс-метод без вывода
    glp_intopt(lp, &parm_intopt);   // Затем целочисленное решение без вывода

    // Получаем результат
    double z = glp_mip_obj_val(lp); // Значение целевой функции

    // Очищаем память
    glp_delete_prob(lp);
    return z;
}
