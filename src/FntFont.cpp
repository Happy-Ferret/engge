#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include "FntFont.h"
#include "_NGUtil.h"

#define PLACEHOLDER_CHAR '?'

namespace ng
{
void CharSet::addKerning(Kerning k)
{
    m_kernings.push_back(k);
}

short CharSet::getKerning(int first, int second) const
{
    for (auto &k : m_kernings)
    {
        if (k.first == first && k.second != second)
            return k.amount;
    }
    return 0;
}

void CharSet::addChar(int id, sf::Glyph &cd)
{
    m_chars[id] = cd;
}

const sf::Glyph &CharSet::getChar(int id) const
{
    // Find the character
    auto it = m_chars.find(id);
    if (it != m_chars.end())
        return it->second;
    // If not found, find a placeholder
    it = m_chars.find(PLACEHOLDER_CHAR);
    return it->second;
}

FntFont::FntFont()
    : _pSettings(nullptr)
{
}

FntFont::~FntFont()
{
}

void FntFont::setSettings(EngineSettings *settings)
{
    _pSettings = settings;
}

bool FntFont::loadFromFile(const std::string &path)
{
    // Parse .fnt file
    std::cout << "FntFont: parsing \"" << path << "\"..." << std::endl;
    if (!parse(path))
        return false;

    // Load resources
    std::cout << "FntFont: loading textures..." << std::endl;
    m_textures.resize(m_chars.pages.size());

    for (size_t i = 0; i < m_chars.pages.size(); i++)
    {
        const std::string texPath = m_chars.pages[i];
        std::vector<char> buffer;
        _pSettings->readEntry(texPath, buffer);
        if (!m_textures[i].loadFromMemory(buffer.data(), buffer.size()))
        {
            std::cout << "ERROR: FntFont::loadFromFile(): "
                      << "Couldn't load texture file \"" << texPath << "\"" << std::endl;
            return false;
        }
    }

    return true;
}

float FntFont::getKerning(sf::Uint32 first, sf::Uint32 second, unsigned int characterSize) const
{
    return m_chars.getKerning(first, second);
}

const sf::Texture &FntFont::getTexture(unsigned int characterSize) const
{
    return m_textures[0];
}

bool FntFont::parse(const std::string &path)
{
    std::vector<char> buffer;
    _pSettings->readEntry(path, buffer);
    GGPackBufferStream input(buffer);

    // Note : the '>>' operator is formatting, so we use short typed values
    // to be sure that they will be read as integers

    while (input.tell() != input.getLength())
    {
        unsigned int i = 0;
        std::stringstream lineStream;
        std::string tag, pair, key, value;
        std::string line;

        getLine(input, line);
        lineStream << line;
        lineStream >> tag;

        //std::cout << lineStream.str() << std::endl;

        if (tag == "info")
        {
            // Not implemented yet
            lineStream.str("");
        }
        else if (tag == "common")
        {
            while (!lineStream.eof())
            {
                lineStream >> pair;
                i = pair.find('=');
                key = pair.substr(0, i);
                value = pair.substr(i + 1);
                std::stringstream converter;
                converter << value;

                if (key == "lineHeight")
                    converter >> m_chars.lineHeight;
                else if (key == "base")
                    converter >> m_chars.base;
                else if (key == "scaleW")
                    converter >> m_chars.scaleW;
                else if (key == "scaleH")
                    converter >> m_chars.scaleH;
                else if (key == "packed")
                    converter >> m_chars.packed;
                else if (key == "alphaChnl")
                    converter >> m_chars.alphaChnl;
                else if (key == "redChnl")
                    converter >> m_chars.redChnl;
                else if (key == "greenChnl")
                    converter >> m_chars.greenChnl;
                else if (key == "blueChnl")
                    converter >> m_chars.blueChnl;
                /*else if(key == "pages") // pages are automatically counted
                            converter >> ?*/
            }
        }
        else if (tag == "page")
        {
            unsigned short id = 0;
            while (!lineStream.eof())
            {
                lineStream >> pair;
                i = pair.find('=');
                key = pair.substr(0, i);
                value = pair.substr(i + 1);
                std::stringstream converter;
                converter << value;

                if (key == "id")
                {
                    converter >> id;
                    if (id >= m_chars.pages.size())
                        m_chars.pages.resize(id + 1);
                }
                else if (key == "file")
                {
                    // Remove quotes
                    const std::string filename = value.substr(1, value.length() - 2);
                    m_chars.pages[id] = filename;
                }
            }
        }
        else if (tag == "char")
        {
            // Note : char count is ignored because not needed
            sf::Glyph glyph;
            int id, x, y, width, height, xoffset, yoffset, page, chnl;
            while (!lineStream.eof())
            {
                lineStream >> pair;
                i = pair.find('=');
                key = pair.substr(0, i);
                value = pair.substr(i + 1);
                std::stringstream converter;
                converter << value;

                if (key == "id")
                    converter >> id;
                else if (key == "x")
                    converter >> x;
                else if (key == "y")
                    converter >> y;
                else if (key == "width")
                    converter >> width;
                else if (key == "height")
                    converter >> height;
                else if (key == "xoffset")
                    converter >> xoffset;
                else if (key == "yoffset")
                    converter >> yoffset;
                else if (key == "xadvance")
                    converter >> glyph.advance;
                else if (key == "page")
                    converter >> page;
                else if (key == "chnl")
                    converter >> chnl;
            }

            glyph.textureRect = sf::IntRect(x, y, width, height);
            glyph.bounds = sf::FloatRect(xoffset, yoffset, width, height);
            m_chars.addChar(id, glyph);
        }
        else if (tag == "kerning")
        {
            Kerning k;
            // Note : Kerning count is ignored because not needed
            while (!lineStream.eof())
            {
                lineStream >> pair;
                i = pair.find('=');
                key = pair.substr(0, i);
                value = pair.substr(i + 1);
                std::stringstream converter;
                converter << value;

                if (key == "first")
                    converter >> k.first;
                else if (key == "second")
                    converter >> k.second;
                else if (key == "amount")
                    converter >> k.amount;
            }

            m_chars.addKerning(k);
        }
    }

    return true;
}

int FntFont::getLineHeight()
{
    return m_chars.lineHeight;
}

const sf::Glyph &
FntFont::getGlyph(sf::Uint32 codePoint, unsigned int characterSize, bool bold, float outlineThickness) const
{
    return m_chars.getChar((int)codePoint);
}

sf::Vector2i FntFont::getTextSize(const std::wstring &text, int begin, int end)
{
    if (begin < 0)
        return sf::Vector2i();
    if (end < 0 || end >= static_cast<int>(text.size()))
        end = text.size() - 1;

    // TODO FntFont::getTextSize() : exact height instead of max height

    sf::Vector2i size(0, getLineHeight());
    for (int i = begin; i <= end; ++i)
    {
        const sf::Glyph &cd = m_chars.getChar(text[i]);
        size.x += cd.advance;
    }
    return size;
}

} // namespace ng
