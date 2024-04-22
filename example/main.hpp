#pragma once
#include "impl.hpp"

struct Initializer final : public utils::impl<Initializer>
{
    void on_begin(const std::string& name) override;
    void on_success(const std::string& name) override;
    void on_error(const std::string& name) override;
};