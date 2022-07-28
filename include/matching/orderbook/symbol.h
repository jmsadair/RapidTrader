#ifndef RAPID_TRADER_SYMBOL_H
#define RAPID_TRADER_SYMBOL_H
struct Symbol
{
    uint32_t id;
    std::string name;
    Symbol(uint32_t id_, std::string name_)
        : id(id_), name(std::move(name_))
    {}
};
#endif // RAPID_TRADER_SYMBOL_H
