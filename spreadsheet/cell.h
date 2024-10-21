#pragma once
#include "common.h"
#include "formula.h"
#include "sheet.h"

#include "unordered_set"
#include <optional>

class Sheet;
class FormulaInterface;

const static char CHAR_APOSTR = '\'';
const static char CHAR_EQUALS = '=';

class Cell : public CellInterface {
public:
    Cell(Sheet& sheet);
    ~Cell();

    void Set(std::string text);
    void Clear();

    Value GetValue() const override;
    std::string GetText() const override;

    std::vector<Position> GetReferencedCells() const override;

    void InvalidateCache();

private:

    class Impl {
    public:
        virtual Value GetValue(const SheetInterface& sheet) const = 0;
        virtual std::string GetText() const = 0;
        virtual std::vector<Position> GetReferencedCells() const = 0;
        virtual void InvalidateCache() {};
    };

    class EmptyImpl : public Impl {
    public:
        Value GetValue(const SheetInterface& sheet) const override;
        std::string GetText() const override;
        std::vector<Position> GetReferencedCells() const override;
    };

    class TextImpl : public Impl {
    public:
        TextImpl(std::string text);
        Value GetValue(const SheetInterface& sheet) const override;
        std::string GetText() const override;
        std::vector<Position> GetReferencedCells() const override;

    private:
        std::string text_;
    };

    class FormulaImpl : public Impl {
    public:
        FormulaImpl(std::string text);
        Value GetValue(const SheetInterface& sheet) const override;
        std::string GetText() const override;
        std::vector<Position> GetReferencedCells() const override;
        void InvalidateCache() override;

    private:
        std::unique_ptr<FormulaInterface> form_;
        mutable std::optional<double> cache_;
        void UpdateCache(double) const ;
    };

    Sheet& sheet_;
    std::unique_ptr<Impl> impl_;
    std::unordered_set<Cell*> ref_cells_; // referenced cells
    std::unordered_set<Cell*> dep_cells_; // dependent cells

    void CheckCyclicity(const Impl&);
};