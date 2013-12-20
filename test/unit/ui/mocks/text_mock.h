#include "common/logger.h"

#include "graphics/engine/text.h"

#include <gmock/gmock.h>

class CTextMock : public Gfx::CText
{
public:
    CTextMock(Gfx::CEngine* engine) : CText(engine)
    {
    }

    virtual ~CTextMock()
    {
    };

    MOCK_METHOD4(GetCharWidth, float(Gfx::UTF8Char ch,
                                     Gfx::FontType type,
                                     float size,
                                     float offset));
    MOCK_METHOD4(GetStringWidth, float(const std::string &text,
                                       std::vector<Gfx::FontMetaChar>::iterator format,
                                       std::vector<Gfx::FontMetaChar>::iterator end,
                                       float size));
    MOCK_METHOD3(GetStringWidth, float(std::string text,
                                       Gfx::FontType font,
                                       float size));
    MOCK_METHOD4(GetStringWidth, float(Gfx::UTF8Char ch,
                                       Gfx::FontType font,
                                       float size,
                                       float offset));

};

