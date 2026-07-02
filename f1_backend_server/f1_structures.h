#pragma once

#include <string>

using namespace std;

// Структура, которая описывает одно Гран-При
struct F1Meeting {
	int id;
	string officialName;
	string countryName;
	string circuitKey;
	string dateStart;
};
