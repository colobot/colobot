#include "common/logger.h"
#include "graphics/engine/text.h"

#include <gmock/gmock.h>

class CTextMock : public Gfx::CText
{
public:
    CTextMock(CInstanceManager *iMan, Gfx::CEngine* engine) : CText(iMan, engine)
    {
    }

    virtual ~CTextMock()
    {
    };

    MOCK_METHOD4(GetCharWidth, float(Gfx::UTF8Char, Gfx::FontType, float, float));
    MOCK_METHOD3(GetStringWidth, float(const std::string &, Gfx::FontType, float));

};

