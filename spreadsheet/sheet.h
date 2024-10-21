#pragma once
#include "common.h"
#include "cell.h"

#include <functional>
#include <map>
#include <string>
#include <iostream>

class Cell;

struct SheetHash {
    int a = 37;
    std::size_t operator()(const Position& pos) const {
        return (pos.col + 1) * a + (pos.row + 1) * a * a;
    }
};

class Sheet : public SheetInterface {
public:
    ~Sheet();

    void SetCell(Position pos, std::string text) override;

    const CellInterface* GetCell(Position pos) const override;
    CellInterface* GetCell(Position pos) override;

    const Cell* GetPtrCell(Position pos) const;
    Cell* GetPtrCell(Position pos);

    void ClearCell(Position pos) override;

    Size GetPrintableSize() const override;

    void PrintValues(std::ostream& output) const override;
    void PrintTexts(std::ostream& output) const override;

private:
    Size size_;
    std::unordered_map<Position, std::unique_ptr<Cell>, SheetHash> sheet_;
    std::map<int, int> col_count;
    std::map<int, int> row_count;

    void UpdateSize(const Position& pos);
    void UpdateSize();

    struct PrintValue {
        std::ostream& out;
        void operator()(const double value) const {
            out << value;
        }
        void operator()(const std::string& text) const {
            out << text;
        }
        void operator()(const FormulaError& fe) const {
            out << fe;
        }
    };

};