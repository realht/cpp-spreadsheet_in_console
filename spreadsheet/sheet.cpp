#include "sheet.h"

#include <algorithm>
#include <functional>
#include <iostream>
#include <optional>
#include <sstream>

using namespace std::literals;

Sheet::~Sheet() {}

void Sheet::SetCell(Position pos, std::string text) {
    if (pos.IsValid()) {
        auto& cell = sheet_[pos];
        if (!cell) {
            cell = std::make_unique<Cell>(*this);   
        }
        
        if (text.size() && this->GetPtrCell(pos)->GetText().empty()) {
            ++col_count[pos.col];
            ++row_count[pos.row];
            UpdateSize();
        }
        cell->Set(std::move(text)); 

    }
    else {
        throw InvalidPositionException("wrong element position");
    }
}

const CellInterface* Sheet::GetCell(Position pos) const {
    return const_cast<Sheet*>(this)->GetCell(pos);
}

CellInterface* Sheet::GetCell(Position pos) {
    if (pos.IsValid()) {
        if (sheet_.count(pos)) {
            return sheet_[pos].get();
        }
    }
    else {
        throw InvalidPositionException("wrong element position");
    }
    return nullptr;
}

const Cell* Sheet::GetPtrCell(Position pos) const {
    if (pos.IsValid()) {
        if (sheet_.count(pos)) {
            return sheet_.at(pos).get();
        }
    }
    else {
        throw InvalidPositionException("wrong element position");
    }
    return nullptr;
    
}

Cell* Sheet::GetPtrCell(Position pos) {
    return const_cast<Cell*>(static_cast<const Sheet&>(*this).GetPtrCell(pos));
}

void Sheet::ClearCell(Position pos) {
    if (pos.IsValid()) {
        if (sheet_.count(pos)) {
            sheet_.erase(pos);
            UpdateSize(pos);
        }
    } 
    else {
        throw InvalidPositionException("wrong element position");
    }
}

Size Sheet::GetPrintableSize() const {
    return size_;
}

void Sheet::PrintValues(std::ostream& output) const {
    for (int y = 0; y < size_.rows; ++y) {
        bool second = false;
        for (int x = 0; x < size_.cols; ++x) {
            if (second) {
                output << '\t';
            }
            second = true;

            if (sheet_.count({ y,x })) {
                std::visit(PrintValue{ output }, sheet_.at({ y,x }).get()->GetValue());
            }
        }
        output << '\n';
    }
}

void Sheet::PrintTexts(std::ostream& output) const {
    for (int y = 0; y < size_.rows; ++y) {
        bool second = false;
        for (int x = 0; x < size_.cols; ++x) {
            if (second) {
                output << '\t';
            }
            second = true;
            output << ( sheet_.count({y,x}) ? sheet_.at({ y,x }).get()->GetText() : "" );
        }
        output << '\n';
    }
}

std::unique_ptr<SheetInterface> CreateSheet() {
    return std::make_unique<Sheet>();
}

void Sheet::UpdateSize() {
    Size result;
    if (row_count.size() > 0) {
        result.rows = row_count.rbegin()->first + 1;
    }
    if (col_count.size() > 0) {
        result.cols = col_count.rbegin()->first + 1;
    }
    size_ = result;
}

void Sheet::UpdateSize(const Position& pos) {
    --col_count[pos.col];
    if (col_count[pos.col] < 1) {
        col_count.erase(pos.col);
    }
    --row_count[pos.row];
    if (row_count[pos.row] < 1) {
        row_count.erase(pos.row);
    }
    UpdateSize();
}
