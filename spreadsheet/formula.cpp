#include "formula.h"
#include "sheet.h"

#include <algorithm>
#include <cassert>
#include <cctype>
#include <sstream>

using namespace std::literals;

std::ostream& operator<<(std::ostream& output, FormulaError fe)
{
    return output << "#ARITHM!";
}

namespace
{
class Formula : public FormulaInterface
{
public:

explicit Formula(std::string expression) try :
        ast_(ParseFormulaAST(expression)) {}
    catch (...)
    {
        throw FormulaException("incorrect formula");
    }


    Value Evaluate(const SheetInterface& sheet) const
    {
        const Sheet* _sheet = dynamic_cast<const Sheet*>(&sheet);

        auto lambda = [&_sheet](Position pos)
        {
            return _sheet->GetValue(pos);
        };

        try
        {
            return ast_.Execute(lambda);
        }
        catch(const FormulaError& error)
        {
            return error;
        }
    }


    std::string GetExpression() const override
    {
        std::ostringstream out;
        ast_.PrintFormula(out);
        return out.str();
    }


    std::vector<Position> GetReferencedCells() const override
    {
        std::vector<Position> cells;

        for (const auto& cell : ast_.GetCells())
        {
            if (cell.IsValid())
            {
                cells.push_back(cell);
            }
            else
            {
                continue;
            }
        }
        return cells;
    }

private:
    FormulaAST ast_;
};
}//end namespace


std::unique_ptr<FormulaInterface> ParseFormula(std::string expression)
{
    return std::make_unique<Formula>(std::move(expression));
}
