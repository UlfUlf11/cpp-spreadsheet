#include "cell.h"
#include "sheet.h"

#include <cassert>
#include <iostream>
#include <string>
#include <optional>
#include <set>
#include <stack>

Cell::Cell(Sheet& sheet) : impl_(std::make_unique<EmptyImpl>()),
    sheet_(sheet) {}

Cell::~Cell() = default;

void Cell::Set(std::string text, Position pos, Sheet* sheet)
{
    std::unique_ptr<Impl> impl;

    if(text == impl_->GetText())
    {
        return;
    }
    else if (text.empty())
    {
        impl = std::make_unique<EmptyImpl>();
    }
    else if (text.size() >= 2 && text[0] == FORMULA_SIGN)
    {
        impl = std::make_unique<FormulaImpl>(std::move(text), sheet_);
    }
    else
    {
        impl= std::make_unique<TextImpl>(std::move(text));
    }

    if (CheckCircularDependencies(*impl,  pos))
    {
        throw CircularDependencyException("formula has circular dependency");
    }

    impl_ = std::move(impl);

    InvalidateCache(true);
}


bool Cell::CheckCircularDependenciesRecursion( Cell* cell,  std::unordered_set<Cell*>& visitedPos)
{
    if(visitedPos.count(this) != 0)
    {
        return true;
    }

    if(cell == this)
    {
        return true;
    }

    for (auto dependentPos : cell->GetReferencedCells())
    {
        Cell* ref_cell = sheet_.GetCellPtr(dependentPos);

        if (CheckCircularDependenciesRecursion(ref_cell, visitedPos))
        {
            return true;
        }
        visitedPos.insert(ref_cell);
    }

    return false;
}


bool Cell::CheckCircularDependencies( const Impl& impl, Position started_pos)
{
    const auto& cells = impl.GetReferencedCells();
    std::unordered_set<Cell*> empty_visited_pos;

    //если ячейка ссылается сама на себя...
    auto it = std::find(cells.begin(), cells.end(), started_pos);
    if(it != cells.end())
    {
        return true;
    }

    for ( const auto& position : cells)
    {
        Cell* ref_cell = sheet_.GetCellPtr(position);

        if (CheckCircularDependenciesRecursion(ref_cell, empty_visited_pos))
        {
            return true;
        }
    }
    
    return false;
}


void Cell::Clear()
{
    impl_ = std::make_unique<EmptyImpl>();
}


Cell::Value Cell::GetValue() const
{
    return impl_->GetValue();
}


std::string Cell::GetText() const
{
    return impl_->GetText();
}


std::vector<Position> Cell::GetReferencedCells() const
{
    return impl_->GetReferencedCells();
}


bool Cell::IsReferenced() const
{
    return !referenced_cells_.empty();
}


void Cell::InvalidateCache(bool reset)
{
    if (impl_->HasCache() || reset)
    {
        impl_->InvalidateCellCache();

        for (Cell* dependent : referenced_cells_)
        {
            dependent->InvalidateCache();
        }
    }
}


//для всех ячеек кроме формульной
std::vector<Position> Cell::Impl::GetReferencedCells() const
{
    return {};
}


//для всех ячеек кроме формульной
bool Cell::Impl::HasCache()
{
    return true;
}


//для всех ячеек кроме формульной
void Cell::Impl::InvalidateCellCache() {}


Cell::Value Cell::EmptyImpl::GetValue() const
{
    return "";
}


std::string Cell::EmptyImpl::GetText() const
{
    return "";
}


Cell::TextImpl::TextImpl(std::string text) : text_(std::move(text)) {}


Cell::Value Cell::TextImpl::GetValue() const
{
    if (text_.at(0) == ESCAPE_SIGN)
    {
        return text_.substr(1);
    }
    else
    {
        return text_;
    }
}


std::string Cell::TextImpl::GetText() const
{
    return text_;
}


Cell::FormulaImpl::FormulaImpl(std::string text, SheetInterface& sheet) : formula_ptr_(ParseFormula(text.substr(1)))
    , sheet_(sheet) {}


Cell::Value Cell::FormulaImpl::GetValue() const
{

    if (!cache_)
    {
        cache_ = formula_ptr_->Evaluate(sheet_);
    }

    return std::visit([](auto& helper)
    {
        return Value(helper);
    }, *cache_);
}


std::string Cell::FormulaImpl::GetText() const
{
    return FORMULA_SIGN + formula_ptr_->GetExpression();
}


std::vector<Position> Cell::FormulaImpl::GetReferencedCells() const
{
    return formula_ptr_->GetReferencedCells();
}


bool Cell::FormulaImpl::HasCache()
{
    return cache_.has_value();
}


void Cell::FormulaImpl::InvalidateCellCache()
{
    cache_.reset();
}
