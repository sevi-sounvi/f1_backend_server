#include <iostream>
#include <string>
#include <vector>
#include <windows.h>
#include "network.h"
#include "f1_structures.h"
#include <nlohmann/json.hpp> // Подключаем библиотеку nlohmann/json



using namespace std;
using json = nlohmann::json;


int main() {
    // Настраиваем консоль Windows, чтобы она корректно отображала UTF-8 (текст на английском/русском)
    SetConsoleOutputCP(CP_UTF8);

    cout << "=== F1 BACKEND SERVER СТАРТУЕТ ===" << endl;
    cout << "Загрузка данных о пилотах текущего сезона..." << endl;

    // Хост API (базовый адрес) и путь к данным о встречах (meetings)
    wstring host = L"api.openf1.org";
    wstring path = L"/v1/meetings?year=2026";

    // Вызываем нашу функцию. Получаем сырую строку JSON из сети
    string rawJsonData = makeHttpRequest(host, path);

    // Создаем вектор
    vector<F1Meeting> calendar;

    // Парсим сырую строку в объект библиотеки json
    try {
        json parsedJson = json::parse(rawJsonData);

        // API возвращает массив гонок. Проходим по каждому элементу массива циклом
        for (const auto& item : parsedJson) {
            F1Meeting meeting;

            // Достаем данные из JSON по ключам (названия ключей мы берем из документации OpenF1 API)
            meeting.id = item.value("meeting_key", 0);
            meeting.officialName = item.value("meeting_official_name", "Unknow Race");
            meeting.countryName = item.value("country_name", "Unknown Country");
            meeting.circuitKey = to_string(item.value("circuit_key", 0));
            meeting.dateStart = item.value("date_start", "No date");

            // Добавляем Гран-При в календарь
            calendar.push_back(meeting);
        }

        // Красиво выводим результат на экран
        cout << "\nУспешно распарсено гонок: " << calendar.size() << "\n" << endl;
        cout << "--------------------------------------------------" << endl;
        for (const auto& race : calendar) {
            cout << "[" << race.id << "] " << race.officialName << endl;
            cout << "    Страна: " << race.countryName << " | Дата начала: " << race.dateStart << endl;
            cout << "--------------------------------------------------" << endl;
        }
    }
    catch(const json::parse_error& e) {
        cerr << "Ошибка парсинга JSON: " << e.what() << endl;
        return 1;
    }

    cout << "\n=== ЗАПРОС ЗАВЕРШЕН ===" << endl;
    return 0;
}