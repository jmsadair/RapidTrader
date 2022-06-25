#ifndef FAST_EXCHANGE_MAIN_SYMBOL_H
#define FAST_EXCHANGE_MAIN_SYMBOL_H
#include <string>
#include <iomanip>

struct Symbol
{
    std::string name;
    uint32_t id;

    inline Symbol(std::string name_, uint32_t id_)
        : name(std::move(name_))
        , id(id_)
    {}

    [[nodiscard]] inline bool operator==(const Symbol &other) const
    {
        return name == other.name && id == other.id;
    }

    [[nodiscard]] inline bool operator!=(const Symbol &other) const
    {
        return !(*this == other);
    }
};

std::ostream &operator<<(std::ostream &os, const Symbol &symbol)
{
    os << "Symbol name: " << symbol.name << "\nSymbol ID: " << symbol.id << "\n";
}
#endif // FAST_EXCHANGE_MAIN_SYMBOL_H
