#pragma once

#include <iosfwd>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

// Позиция ячейки. Индексация с нуля.
struct Position {
    int row = 0;
    int col = 0;

    bool operator==(Position rhs) const;
    bool operator<(Position rhs) const;

    bool IsValid() const;
    std::string ToString() const;

    static Position FromString(std::string_view str);

    static const int MAX_ROWS = 16384;
    static const int MAX_COLS = 16384;
    static const Position NONE;
};

struct Size {
    int rows = 0;
    int cols = 0;

    bool operator==(Size rhs) const;
};

// Описывает ошибки, которые могут возникнуть при вычислении формулы.
class FormulaError {
public:
    enum class Category {
        Ref,    // ссылка на ячейку с некорректной позицией
        Value,  // ячейка не может быть трактована как число
        Arithmetic,  // некорректная арифметическая операция
    };

    FormulaError(Category category);

    Category GetCategory() const;

    bool operator==(FormulaError rhs) const;

    std::string_view ToString() const;

private:
    Category category_;
};

std::ostream& operator<<(std::ostream& output, FormulaError fe);

// Исключение, выбрасываемое при попытке передать в метод некорректную позицию
class InvalidPositionException : public std::out_of_range {
public:
    using std::out_of_range::out_of_range;
};

// Исключение, выбрасываемое при попытке задать синтаксически некорректную формулу
class FormulaException : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

// Исключение, выбрасываемое при попытке задать формулу, которая приводит к
// циклической зависимости между ячейками
class CircularDependencyException : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

class CellInterface {
public:
    // Либо текст ячейки, либо значение формулы, либо сообщение об ошибке
    using Value = std::variant<std::string, double, FormulaError>;

    virtual ~CellInterface() = default;

    // Возвращает видимое значение ячейки.
    virtual Value GetValue() const = 0;

    // Возвращает внутренний текст ячейки
    virtual std::string GetText() const = 0;

    // Возвращает список ячеек, которые непосредственно задействованы в данной формуле
    virtual std::vector<Position> GetReferencedCells() const = 0;
};

inline constexpr char FORMULA_SIGN = '=';
inline constexpr char ESCAPE_SIGN = '\'';

// Интерфейс таблицы
class SheetInterface {
public:
    virtual ~SheetInterface() = default;

    // Задаёт содержимое ячейки
    virtual void SetCell(Position pos, std::string text) = 0;

    // Возвращает значение ячейки.
    virtual const CellInterface* GetCell(Position pos) const = 0;
    virtual CellInterface* GetCell(Position pos) = 0;

    // Очищает ячейку
    virtual void ClearCell(Position pos) = 0;

    // Вычисляет размер области, которая участвует в печати
    virtual Size GetPrintableSize() const = 0;

    // Выводит всю таблицу в переданный поток
    virtual void PrintValues(std::ostream& output) const = 0;
    virtual void PrintTexts(std::ostream& output) const = 0;
};

// Создаёт готовую к работе пустую таблицу
std::unique_ptr<SheetInterface> CreateSheet();