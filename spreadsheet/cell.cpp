#include "cell.h"

#include <cassert>
#include <iostream>
#include <string>
#include <optional>
#include <stack>
#include <queue>

Cell::Cell(Sheet& sheet)
	: sheet_(sheet),
	impl_(std::make_unique<EmptyImpl>(EmptyImpl{}))
{}

Cell::~Cell() {}

void Cell::Set(std::string text) {

	std::unique_ptr<Impl> cell_type;
	if (text.empty()) {
		cell_type = std::make_unique<EmptyImpl>(EmptyImpl{});
	}
	else if (text.size() > 1 && text.at(0) == CHAR_EQUALS) {
		cell_type = std::make_unique<FormulaImpl>(FormulaImpl{ std::move(text.substr(1)) });
	}
	else {
		cell_type = std::make_unique<TextImpl>(TextImpl{std::move(text) });
	}

	CheckCyclicity(*cell_type);

	impl_ = std::move(cell_type);

	ref_cells_.clear();

	//update ref and dep cells
	for (const Position& pos : impl_->GetReferencedCells()) {
		Cell* temp_cell = sheet_.GetPtrCell(pos);

		if (!temp_cell) {
			sheet_.SetCell(pos, "");
			temp_cell = sheet_.GetPtrCell(pos);
		}
		ref_cells_.insert(temp_cell);
		sheet_.GetPtrCell(pos)->dep_cells_.insert(this);
	}

	//invalidate cache for dep
	InvalidateCache();
}

void Cell::Clear() {
	Set("");
}

Cell::Value Cell::GetValue() const {
	return impl_->GetValue(sheet_);
}

std::string Cell::GetText() const {
	return impl_->GetText();
}

std::vector<Position> Cell::GetReferencedCells() const {
	return impl_->GetReferencedCells();
}

void Cell::InvalidateCache() {
	if (impl_->GetReferencedCells().size()) {
		impl_->InvalidateCache();
		for (const auto& cell : dep_cells_) {
			cell->InvalidateCache();
		}
	}
}

void Cell::CheckCyclicity(const Impl& impl) {
	if (impl.GetReferencedCells().empty()) {
		return;
	}

	std::unordered_set<const Cell*> uniq_ref_cells;
	for (const Position& pos : impl.GetReferencedCells()) {
		if (!sheet_.GetPtrCell(pos)) {
			sheet_.SetCell(pos, "");
		}
		uniq_ref_cells.insert(sheet_.GetPtrCell(pos));
	}

	std::unordered_set<const Cell*> checked_cells;
	std::stack<const Cell*> to_check;
	to_check.push(this);

	while (!to_check.empty()) {
		const Cell* cur = to_check.top();
		to_check.pop();

		if (uniq_ref_cells.find(cur) != uniq_ref_cells.end()) {
			throw CircularDependencyException("!!!CDD: Circular Dependency Detected!!!");
		}

		for (const auto dep : cur->dep_cells_) {
			if (checked_cells.find(dep) == checked_cells.end()) {
				to_check.push(dep);
			}
		}
		checked_cells.insert(cur);
	}
}


CellInterface::Value Cell::EmptyImpl::GetValue(const SheetInterface& sheet) const {
	return "";
}
std::string Cell::EmptyImpl::GetText() const {
	return "";
}
std::vector<Position> Cell::EmptyImpl::GetReferencedCells() const {
	return {};
}


Cell::TextImpl::TextImpl(std::string text)
	: text_(text)
{}

CellInterface::Value Cell::TextImpl::GetValue(const SheetInterface& sheet) const {
	if (!text_.empty() && *text_.begin() == CHAR_APOSTR) {
		return text_.substr(1);
	}
	return text_;
}

std::string Cell::TextImpl::GetText() const {
	return text_;
}

std::vector<Position> Cell::TextImpl::GetReferencedCells() const {
	return {};
}


Cell::FormulaImpl::FormulaImpl(std::string text)
	: form_(ParseFormula(text))
{}

CellInterface::Value Cell::FormulaImpl::GetValue(const SheetInterface& sheet) const {
	if (cache_.has_value()) {
		return cache_.value();
	}

	FormulaInterface::Value res = form_->Evaluate(sheet);
	if (std::holds_alternative<double>(res)) {
		cache_ = std::get<double>(res);
		return std::get<double>(res);
	}
	else {
		return std::get<FormulaError>(res);
	}
}

std::string Cell::FormulaImpl::GetText() const {
	return CHAR_EQUALS + form_->GetExpression();
}

std::vector<Position> Cell::FormulaImpl::GetReferencedCells() const {
	return form_->GetReferencedCells();
}

void Cell::FormulaImpl::InvalidateCache() {
	cache_.reset();
}