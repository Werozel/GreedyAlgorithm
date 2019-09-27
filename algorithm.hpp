#include <iostream>
#include <cstring>
#include <vector>

// Проверяет есть ли распределенные задачи
bool is_blank(int *distribution, int number_of_tasks)
{
    for (int i = 0; i < number_of_tasks; i++) {
        if (distribution[i] != -1) {
            return false;
        }
    }
    return true;
}

// Проверяет есть ли еще нераспределенные задачи
bool still_tasks_to_distr(int *distribution, int number_of_tasks)
{
    for (int i = 0; i < number_of_tasks; i++) {
        if (distribution[i] == -1) {
            return true;
        }
    }
    return false;
}

//Ищем задачу с максимальной общей загрузкой
template <typename LoadType, typename IntensType>
int get_max_intense_task(int *distribution, int number_of_tasks, IntensType **intensity)
{
    //Номер задачи, наиболее интенсивно обменивающейся данными с остальными
    int max_intense_task = -1;
    LoadType max_intens = LoadType();
    //Есть распределенные задачи
    bool first_task_flag = false;
    if (is_blank(distribution, number_of_tasks)) {
        //Ни одна задача не распределена
        first_task_flag = true;
    }

    for (int i = 0; i < number_of_tasks; i++) {
        LoadType curr_intens = LoadType();
        for (int j = 0; j < number_of_tasks; j++) {
            // Ищем задачи, распределенные на процессор и суммируем интенсивность обмена данными рассматриваемой (i-ой)
            // задачи с распределенными. Либо, если ни одна задача не распределена, ищем ту, которая наиболее интенсивно
            // обменивается данными со всеми остальными
            if (i != j and (first_task_flag or (not first_task_flag and i != j and distribution[j] != -1))) {
                curr_intens += intensity[i][j];
            }
        }
        if (curr_intens > max_intens and distribution[i] == -1) {
            max_intens = curr_intens;
            max_intense_task = i;
        }
    }

    return max_intense_task;
}

//Ищет наиболее подходящий процессор для заданной задачи согласно алгоритму
template <typename LoadType, typename IntensType>
int get_best_processor(int *distribution, int number_of_proc, LoadType *max_load, int number_of_tasks,
                       LoadType *tasks_load, int curr_task, LoadType *curr_proc_load, IntensType **intensity)
{
    bool found_proc_flag = false;
    if (is_blank(distribution, number_of_tasks)) {
        found_proc_flag = true;
    }
    int curr_best_proc = 0;
    IntensType max_intensity = IntensType();
    // Рассматриваем все процессоры, чтобы получить тот на котором задачи, которые обменивается данными с curr_task
    // максимально интенсивно
    for (int curr_proc = 0; curr_proc < number_of_proc; curr_proc++)
    {
        //Не рассматриваем те процессы, которые перегрузятся, если на них поместить текущую задачу
        if (curr_proc_load[curr_proc] + tasks_load[curr_task] > max_load[curr_proc]) {
            continue;
        }

        //Если нагрузка на процессор 0, то задач на нем нет, значит нет и обмена данными
        IntensType curr_intensity = IntensType();
        if (curr_proc_load[curr_proc] != 0) {
            //Ищем процессы, которые создают нагрузку на рассматриваемый процессор и записываем в вектор
            std::vector<int> tasks_that_loads_proc;
            for (int j = 0; j < number_of_tasks; j++) {
                if (distribution[j] == curr_proc) {
                    tasks_that_loads_proc.push_back(j);
                }
            }

            // Считаем интенсивность обмена данными с теми задачами, что уже распределены на curr_proc
            for (auto x: tasks_that_loads_proc) {
                curr_intensity += intensity[curr_task][x];
            }
        }

        //Проверяем: интенсивность обмена с задачами на рассматриваемом процессоре будет больше, чем максимум,
        //что мы уже нашли? Если больше, то выбираем curr_proc как оптимальный.
        //Если интенсивность на двух процессорах равна, выбираем тот, у которого осталось больше запаса нагрузки
        if (curr_intensity > max_intensity) {
            max_intensity = curr_intensity;
            curr_best_proc = curr_proc;
            found_proc_flag = true;
        } else if (curr_intensity == max_intensity) {
            found_proc_flag = true;
            if (max_load[curr_proc] - curr_proc_load[curr_proc] >
                max_load[curr_best_proc] - curr_proc_load[curr_best_proc]) {
                max_intensity = curr_intensity;
                curr_best_proc = curr_proc;
            }
        }
    }
    if (found_proc_flag) {
        return curr_best_proc;
    } else {
        return -1;
    }
}

template <typename LoadType, typename IntensType>
int* get_distribution(int number_of_proc, LoadType *max_load,
                      int number_of_tasks, LoadType *tasks_load, IntensType **intensity)
{
    int *distribution = new int[number_of_tasks];
    std::memset(distribution, -1, number_of_tasks * sizeof(int));
    LoadType *curr_proc_load = new LoadType[number_of_proc];
    std::memset(curr_proc_load, 0, number_of_proc * sizeof(LoadType));

    // Ищем задачу, которая максимально интенсивно обменивается данными со всеми остальными задачами
    // И распределяем ее на подходящий процессор
    int iterations = 0;
    while (still_tasks_to_distr(distribution, number_of_tasks)) {
        int curr_task = get_max_intense_task <LoadType, IntensType>(distribution, number_of_tasks, intensity);
        int curr_proc = get_best_processor(distribution, number_of_proc, max_load, number_of_tasks, tasks_load,
                                           curr_task, curr_proc_load, intensity);

        if (curr_proc == -1) {
            return nullptr;
        }

        distribution[curr_task] = curr_proc;
        curr_proc_load[curr_proc] += tasks_load[curr_task];
    }

    delete[] curr_proc_load;
    return distribution;
}

int algorithm() {
    int number_of_proc = 0;
    std::cin >> number_of_proc;

    int *max_load = new int[number_of_proc];
    for (int i = 0; i < number_of_proc; i++) {
        std::cin >> max_load[i];
    }
    std::cout << std::endl;

    int number_of_tasks = 0;
    std::cin >> number_of_tasks;

    int *tasks_load = new int[number_of_tasks];
    for (int i = 0; i < number_of_tasks; i++) {
        std::cin >> tasks_load[i];
    }

    int **intensity = new int*[number_of_tasks];
    for (int i = 0; i < number_of_tasks; i++) {
        intensity[i] = new int[number_of_tasks];
        for (int j = 0; j < number_of_tasks; j++) {
            std::cin >> intensity[i][j];
        }
    }

    // Подсчет максимально возможной нагрузки на сеть
    double max_possible_load = 0;
    for (int i = 0; i < number_of_tasks; i++) {
        for (int j = 0; j < i; j++) {
            max_possible_load += intensity[i][j];
        }
    }

    // Получаем распределение задач на процессоры
    int *distribution = get_distribution<int, int>(number_of_proc, max_load, number_of_tasks, tasks_load, intensity);
    // Если алгоритм не дал решения, то функция вернет nullptr
    if (distribution == nullptr) {
        std::cout << "No Sulution" << std::endl;
        delete[] max_load;
        delete[] tasks_load;
        for (int i = 0; i < number_of_tasks; i++) {
            delete[] intensity[i];
        }
        delete[] intensity;
        return 0;
    }

    for (int i = 0; i < number_of_tasks; i++) {
        std::cout << distribution[i] + 1 << " ";
    }
    std::cout << std::endl;

    // Подсчет получившейся нагрузки на сеть
    double curr_net_load = 0;
    for (int i = 0; i < number_of_tasks; i++) {
        for (int j = 0; j < i; j++) {
            if (distribution[i] != distribution[j]) {
                curr_net_load += intensity[i][j];
            }
        }
    }

    //std::cout << "Max possible load = " << max_possible_load << std::endl;
    //std::cout << "Current net load = " << curr_net_load << std::endl;
    std::cout << "Quality of solution = " << curr_net_load / max_possible_load << std::endl;

    delete[] distribution;
    delete[] max_load;
    delete[] tasks_load;
    for (int i = 0; i < number_of_tasks; i++) {
        delete[] intensity[i];
    }
    delete[] intensity;
    return 0;
}
