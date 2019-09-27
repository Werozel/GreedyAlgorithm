#include <iostream>
#include <errno.h>
#include <string>
#include <fstream>
#include <sstream>
#include <unistd.h>
#include <fcntl.h>
#include <climits>
#include <sys/wait.h>
#include <chrono>
#include <iomanip>

#include "json.hpp"
#include "test_generator.hpp"
#include "algorithm.hpp"

using json = nlohmann::json;


int main() {
    int begin_number;
    std::cout << "Enter begin number for tests: ";
    std::cin >> begin_number;
    int tests_number;
    std::cout << "Enter how many tests to run: ";
    std::cin >> tests_number;
    generate(begin_number, tests_number);
    
    //Получаем файловый дескриптор для файла, куда будет выведен результат работы программы
    char filename[PATH_MAX];
    int output = open("output.txt", O_RDWR | O_TRUNC | O_CREAT, 0600);

    if (dup2(output, 1) < 0) {
        return 1;
    }

    std::cout << "Начинаю тестирование " << std::endl;
    int test_number = begin_number;
    int max_tests = begin_number + 10;
    while (test_number < max_tests) {
        // Открываю файл с тестом на чтение
        std::stringstream ss;
        ss << "test" << test_number << ".json";
        std::fstream f(ss.str(), std::ios::in);
        std::cout << std::endl << "TEST " << test_number << ":" << std::endl;

        //Получаю строку формата json в которой хранится текущий тест
        std::string test;
        f >> test;
        json j = json::parse(test);

        int fd[2];
        if (pipe(fd) < 0) {
            return 1;
        }

        FILE *out = fdopen(fd[1], "w");

        // Получаю количество процессоров из json
        int number_of_proc = j["proc_num"];
        fprintf(out, "%d ", number_of_proc);
        std::cout << number_of_proc << std::endl;

        // Получаю максимальную нагрузку на процессоры
        int *max_load = new int[number_of_proc];
        for (int i = 0; i < number_of_proc; i++) {
            max_load[i] = j["max_load"][i];
            fprintf(out, "%d ", max_load[i]);
            std::cout << max_load[i] << " ";
        }
        std::cout << std::endl;

        // Получаю количество задач из json
        int number_of_tasks = j["tasks_num"];
        fprintf(out, "%d ", number_of_tasks);
        std::cout << number_of_tasks << std::endl;

        // Получаю нагрузки, создаваемые задачами
        int *tasks_load = new int[number_of_tasks];
        for (int i = 0; i < number_of_tasks; i++) {
            tasks_load[i] = j["tasks_load"][i];
            fprintf(out, "%d ", tasks_load[i]);
            std::cout << tasks_load[i] << " ";
        }
        std::cout << std::endl;

        // Получаю интенсивность обмена данными для задач
        int **intensity = new int*[number_of_tasks];
        for (int i = 0; i < number_of_tasks; i++) {
            intensity[i] = new int[number_of_tasks];
            for (int t = 0; t < number_of_tasks; t++) {
                intensity[i][t] = j["intensity"][i][t];
                fprintf(out, "%d ", intensity[i][t]);
                std::cout << std::setw(3) << intensity[i][t] << " ";
            }
            std::cout << std::endl;
        }
        std::cout << std::endl;

        fclose(out);
        close(fd[1]);

        // Запускаю алгоритм с введенными данными
        std::cout << "OUT: ";
        
        if (dup2(fd[0], 0) < 0) {
            return 1;
        }
        
        // Запускаю алгоритм 
        auto begin_time = std::chrono::high_resolution_clock::now();
        algorithm();
        auto end_time = std::chrono::high_resolution_clock::now();
        close(fd[0]);
        
        // Считаю время работы программы
        std::cout << "Working time = " <<  (double)(std::chrono::duration_cast<std::chrono::nanoseconds>
                (end_time - begin_time).count()) / 1000000000 << "s" << std::endl << std::endl;
        
        // Освобождаю занятую память
        delete[] max_load;
        delete[] tasks_load;
        for (int i = 0; i < number_of_tasks; i++) {
            delete[] intensity[i];
        }
        delete[] intensity;
        
        test_number++;
    }

    std::cout << "Тестирование завершено" << std::endl;
    close(output);
}
