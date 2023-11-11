#pragma once

#include "common.h"
#include "formula.h"

#include <functional>
#include <stack>
#include <set>
#include <optional>

class Sheet;

class Cell : public CellInterface
{
public:
    Cell(Sheet& sheet);
    ~Cell();

    void Set(std::string text, Position pos, Sheet* sheet);
    void Clear();

    Value GetValue() const override;
    std::string GetText() const override;
    std::vector<Position> GetReferencedCells() const override;

    bool CheckCircularDependency(const std::vector<Position>& referencedCells) const;
    bool IsReferenced() const;
    void InvalidateCache(bool reset = false);

private:
    class Impl;

    std::unique_ptr<Impl> impl_;
    Sheet& sheet_;

    std::set<Cell*> referenced_cells_; // ячейки в расчете которых участвует эта ячейка
    std::set<Cell*> using_cells_;


    class Impl
    {
    public:
        virtual Value GetValue() const = 0;
        virtual std::string GetText() const = 0;
        virtual std::vector<Position> GetReferencedCells() const;

        virtual bool HasCache();
        virtual void InvalidateCellCache();

        virtual ~Impl() = default;
    };

    class EmptyImpl : public Impl
    {
    public:

        Value GetValue() const override;
        std::string GetText() const override;
    };

    class TextImpl : public Impl
    {
    public:

        explicit TextImpl(std::string text);
        Value GetValue() const override;
        std::string GetText() const override;

    private:
        std::string text_;
    };

    class FormulaImpl : public Impl
    {
    public:
        explicit FormulaImpl(std::string text, SheetInterface& sheet);

        Value GetValue() const override;
        std::string GetText() const override;
        std::vector<Position> GetReferencedCells() const override;

        bool HasCache() override;
        void InvalidateCellCache() override;

    private:
        mutable std::optional<FormulaInterface::Value> cache_;
        std::unique_ptr<FormulaInterface> formula_ptr_;
        SheetInterface& sheet_;
    };


    bool CheckCircularDependenciesRecursion( Cell* cell,  std::unordered_set<Cell*>& visitedPos);
    bool CheckCircularDependencies( const Impl& impl, Position pos);
};
