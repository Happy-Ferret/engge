#pragma once
#include <array>
#include <string>
#include "SFML/Graphics/Color.hpp"

namespace ng
{
struct Verb
{
    int id;
    std::string image;
    std::string func;
    std::string text;
    std::string key;
};

class VerbSlot
{
  public:
    void setVerb(int index, const Verb &verb) { _verbs.at(index) = verb; }
    const Verb &getVerb(int index) const { return _verbs.at(index); }
    size_t getVerbIndex(int id) const
    {
        for (size_t i = 0; i < _verbs.size(); i++)
        {
            if (_verbs.at(i).id == id)
                return i;
        }
        return -1;
    }

  private:
    std::array<Verb, 10> _verbs;
};

struct VerbUiColors
{
    sf::Color sentence;
    sf::Color verbNormal;
    sf::Color verbNormalTint;
    sf::Color verbHighlight;
    sf::Color verbHighlightTint;
    sf::Color dialogNormal;
    sf::Color dialogHighlight;
    sf::Color inventoryFrame;
    sf::Color inventoryBackground;
};
} // namespace ng