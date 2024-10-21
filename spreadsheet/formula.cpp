#include "formula.h"
#include "FormulaAST.h"

#include <algorithm>
#include <cassert>
#include <cctype>
#include <sstream>

using namespace std::literals;

namespace {
class Formula : public FormulaInterface {
public:

    explicit Formula(std::string expression) try
    : ast_(ParseFormulaAST(expression)) {
    }
    catch (const FormulaException& ex) {
        throw FormulaException("Wrong formula");
    }

    Value Evaluate(const SheetInterface& sheet) const override {
        
        const ExecuteFromSheet lambda = [&sheet](const Position pos) -> double {
            if (!pos.IsValid()) {
                throw FormulaError(FormulaError::Category::Ref);
            }

            if (!sheet.GetCell(pos)) {
                const_cast<SheetInterface&>(sheet).SetCell(pos, "");
            }

            const auto cell = sheet.GetCell(pos);

            if (std::holds_alternative<double>(cell->GetValue())) {
                return std::get<double>(cell->GetValue());
            }

            double result = 0;
            if (std::holds_alternative<std::string>(cell->GetValue())) {
                auto value = std::get<std::string>(cell->GetValue());

                if (!value.empty()) {
                    std::istringstream input(value);
                    if (!(input >> result) || !input.eof()) {
                        throw FormulaError(FormulaError::Category::Value);
                    }
                }
            }
            return result;
        };
        
        Value result;
        try {
            return ast_.Execute(lambda);
        }
        catch (const FormulaError& fe) {
            return fe;
        }
    }

    std::string GetExpression() const override {
        std::ostringstream out;
        ast_.PrintFormula(out);
        return out.str();
    }

    std::vector<Position> GetReferencedCells() const override {
        std::forward_list<Position> temp = ast_.GetCells();
        temp.unique();
        return std::vector<Position>{temp.begin(), temp.end()};
    }

private:
    FormulaAST ast_;
    double cache_;
};
}  // namespace

std::unique_ptr<FormulaInterface> ParseFormula(std::string expression) {
    return std::make_unique<Formula>(std::move(expression));
}