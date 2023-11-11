#pragma once

#include "cell.h"
#include "common.h"

#include <functional>
#include <vector>


class Sheet : public SheetInterface
{
public:
    ~Sheet();

// Задаёт содержимое ячейки.
// * Если текст начинается с символа "'" (апостроф), то при выводе значения
// ячейки методом GetValue() он опускается. Можно использовать, если нужно
// начать текст со знака "=", но чтобы он не интерпретировался как формула.
    void SetCell(Position pos, std::string text) override;

    // Возвращает значение ячейки.
// Если ячейка пуста, может вернуть nullptr.
    const CellInterface* GetCell(Position pos) const override;
    CellInterface* GetCell(Position pos) override;

    //возвращает указатель на ячейку по ее позиции
    Cell* GetCellPtr(Position pos);
    const Cell* GetCellPtr(Position pos) const;

    void ClearCell(Position pos) override;

    Size GetPrintableSize() const override;

    void PrintValues(std::ostream& output) const override;
    void PrintTexts(std::ostream& output) const override;

    CellInterface::Value GetValue(Position pos) const;

private:
    std::vector<std::vector<std::unique_ptr<Cell>>> cells_;

};
