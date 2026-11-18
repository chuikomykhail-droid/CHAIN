#pragma once
#include <string>
#include <vector>

// Открывает окно SDL3 и рисует график y = f(x) по точкам сетки (X[i], Y[i]).
// Функция блокирующая: возвращается, когда окно закрыто (крестик / Esc / Q).
// Окно можно растягивать — масштаб пересчитывается автоматически.
void plotFunction(const std::vector<double>& X,
                  const std::vector<double>& Y,
                  const std::string& title = "f(x)");

// Несколько кривых на одной сетке X (например, численное и точное решения).
void plotFunctions(const std::vector<double>& X,
                   const std::vector<std::vector<double>>& Ys,
                   const std::string& title = "f(x)");
