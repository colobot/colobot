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

    MOCK_METHOD4(GetCharWidth, float(Gfx::UTF8Char, Gfx::FontType, float, float));
    MOCK_METHOD4(GetStringWidth, float(const std::string &text,
                                       std::vector<Gfx::FontMetaChar>::iterator format,
                                       std::vector<Gfx::FontMetaChar>::iterator end, float size));
    MOCK_METHOD3(GetStringWidth, float(const std::string &, Gfx::FontType, float));

};

