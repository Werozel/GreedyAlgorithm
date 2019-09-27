#include <iostream>
#include <fstream>
#include <vector>
#include <ctime>
#include <string>
#include <sstream>
#include "json.hpp"

using json = nlohmann::json;

template <typename Type>
int get_random(int n, const Type *chances)
{
    double rnd = std::rand() % 101;
    int res = 0;
    double perc = 0;
    for (int i = 0; i < n; i++) {
        perc += chances[i];
        if (rnd <= perc) {
            return i;
        }
    }
    return res;
}

bool is_blank (int n, const int *chances)
{
    for (int i = 0; i < n; i++) {
        if (chances[i] != -1) {
            return false;
        }
    }
    return true;
}

int generate(int begin_number, int tests_number)
{
    std::srand(std::time(nullptr));

    std::fstream fin("input.txt", std::ios::in);

    // Ввод количества процессоров
    int number_of_proc = 0;
    fin >> number_of_proc;

    // Ввод вероятностей сопоставления задачам конкретных нагрузок на процессор
    double tasks_load_chance[3] = {0, 0, 0};
    fin >> tasks_load_chance[0] >> tasks_load_chance[1] >> tasks_load_chance[2];
    if (tasks_load_chance[0] + tasks_load_chance[1] + tasks_load_chance[2] != 100) {
        std::cout << "Sum of tasks load chances doesn't equal 100!\nTry again:" << std::endl;
        return 0;
    }

    // Ввод вероятностей сопоставления процессорам конкретных пределов нагрузки
    double max_load_chance[4] = {0, 0, 0, 0};
    fin >> max_load_chance[0] >> max_load_chance[1] >>
        max_load_chance[2] >> max_load_chance[3];
    if (max_load_chance[0] + max_load_chance[1] +
        max_load_chance[2] + max_load_chance[3] != 100) {
        std::cout << "Sum of max load chances doesn't equal 100!" << std::endl;
        return 0;
    }

    // Ввод доли(Q) от суммарного по всем процессорам "запаса производительности"
    double percentage = 0;
    fin >> percentage;
    if (percentage > 100) {
        std::cout << "Percentage can't be more than 100!" << std::endl;
        return 0;
    }

    // Ввод вероятностей сопоставления парам задач конкретных интенсивностей обмена данными
    double intense_chance[4] = {0, 0, 0, 0};
    fin >> intense_chance[0] >> intense_chance[1] >> intense_chance[2] >> intense_chance[3];
    if (intense_chance[0] + intense_chance[1] + intense_chance[2] + intense_chance[3] != 100) {
        std::cout << "Sum of intense chances doesn't equal 100!" << std::endl;
        return 0;
    }

    fin.close();

    // Генерация 10 тестов для конкретного входного набора
    for (int r = begin_number; r < tests_number; r++) {

        // Выбор пределов нагрузки на процессоры
        int proc_possible_load[4] = {40, 60, 80, 100};
        int max_proc_load[number_of_proc];
        int loads_sum = 0;
        for (int i = 0; i < number_of_proc; i++) {
            loads_sum += max_proc_load[i] = proc_possible_load[get_random(4, max_load_chance)];
        }

        // Рассчет "запаса производительности"
        int margin_of_safety = (int) (loads_sum * percentage/ 100);

        // Добавление новых задач
        int tasks_possible_load[3] = {5, 10, 20};
        int tasks_load_sum = 0;
        std::vector<int> tasks_load;
        while (not is_blank(3, tasks_possible_load)) {
            int i = get_random(3, tasks_load_chance);
            // Если добавление новой задачи с такой нагрузкой
            // повлечет за собой перегрузку процессоров,
            // то запретить добавлять задачи с такой нагрузкой
            if (tasks_load_sum + tasks_possible_load[i] > margin_of_safety) {
                tasks_possible_load[i] = -1;
            } else {
                //Иначе, если нагрузка не запрещена, то доавить текущую задачу
                if (tasks_possible_load[i] != -1) {
                    tasks_load.push_back(tasks_possible_load[i]);
                    tasks_load_sum += tasks_possible_load[i];
                }
            }
        }
        int number_of_tasks = tasks_load.size();

        //Генерирование интенсивности обмена данными между задачами
        int possible_intensity[4] = {0, 10, 50, 100};
        int intensity[number_of_tasks][number_of_tasks];
        for (int i = 0; i < number_of_tasks; i++) {
            for (int j = 0; j <= i; j++) {
                if (j == i) {
                    intensity[i][j] = 0;
                } else {
                    int t = get_random(4, intense_chance);
                    intensity[i][j] = intensity[j][i] = possible_intensity[t];
                }
            }
        }

        //Запись результата в json файл
        json j;
        j["proc_num"] = number_of_proc;
        j["max_load"] = std::vector<int>();
        for (int i = 0; i < number_of_proc; i++) {
            j["max_load"].push_back(max_proc_load[i]);
        }
        j["tasks_num"] = number_of_tasks;
        j["tasks_load"] = std::vector<int>();
        for (int i = 0; i < number_of_tasks; i++) {
            j["tasks_load"].push_back(tasks_load[i]);
        }
        j["intensity"] = std::vector< std::vector<int> >();
        for (int i = 0; i < number_of_tasks; i++) {
            std::vector<int> tmp;
            for (int j = 0; j < number_of_tasks; j++) {
                tmp.push_back(intensity[i][j]);
            }
            j["intensity"].push_back(tmp);
        }

        std::stringstream filename;
        filename << "test" << r << ".json";
        std::fstream fout(filename.str(), std::ios::out | std::ios::trunc);
        std::string s = j.dump();
        fout << s <<std::endl;
        fout.close();
    }


    return 0;
}

