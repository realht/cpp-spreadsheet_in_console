#pragma once

#include "common.h"

#include <memory>
#include <vector>

// Формула, позволяющая вычислять и обновлять арифметическое выражение
class FormulaInterface {
public:
    using Value = std::variant<double, FormulaError>;

    virtual ~FormulaInterface() = default;

    // Возвращает вычисленное значение формулы для переданного листа либо ошибку
    virtual Value Evaluate(const SheetInterface& sheet) const = 0;

    // Возвращает выражение, которое описывает формулу
    virtual std::string GetExpression() const = 0;

    // Возвращает список ячеек, которые непосредственно задействованы в вычислении формулы
    virtual std::vector<Position> GetReferencedCells() const = 0;
};

// Парсит переданное выражение и возвращает объект формулы
// Бросает FormulaException в случае, если формула синтаксически некорректна
std::unique_ptr<FormulaInterface> ParseFormula(std::string expression);