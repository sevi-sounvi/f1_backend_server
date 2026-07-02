#include <iostream>
#include <string>
#include <windows.h>
#include "network.h"

using namespace std;


int main() {
    // Настраиваем консоль Windows, чтобы она корректно отображала UTF-8 (текст на английском/русском)
    SetConsoleOutputCP(CP_UTF8);

    cout << "=== F1 BACKEND SERVER СТАРТУЕТ ===" << endl;
    cout << "Загрузка данных о пилотах текущего сезона..." << endl;

    // Хост API (базовый адрес) и путь к данным о встречах (meetings)
    wstring host = L"api.openf1.org";
    wstring path = L"/v1/meetings?year=2026";

    // Вызываем нашу функцию
    string rawJsonData = makeHttpRequest(host, path);

    // Выводим сырые данные (JSON), которые прислал сервер
    cout << "\nОтвет от F1 API:\n" << endl;

    // Выведем первые 1000 символов ответа, чтобы не спамить всю консоль
    if (rawJsonData.length() > 1000) {
        cout << rawJsonData.substr(0, 1000) << "\n... [и еще " << rawJsonData.length() - 1000 << " символов] ..." << endl;
    }
    else {
        cout << rawJsonData << endl;
    }

    cout << "\n=== ЗАПРОС ЗАВЕРШЕН ===" << endl;
    return 0;
}