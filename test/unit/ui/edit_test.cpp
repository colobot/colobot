#include "app/app.h"
#include "ui/edit.h"
#include "mocks/text_mock.h"

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <fstream>

class CEditTest : public testing::Test
{
public:
    CEditTest()
     : m_robotMain(nullptr)
     , m_engine(nullptr)
     , m_edit(nullptr)
    {}

    virtual void SetUp()
    {
        m_robotMain = new CRobotMain(&m_app);

        m_engine = new Gfx::CEngine(nullptr);

        m_edit = new Ui::CEdit;
    }

    virtual void TearDown()
    {
        delete m_robotMain;
        m_robotMain = nullptr;
        delete m_engine;
        m_engine = nullptr;
        delete m_edit;
        m_edit = nullptr;

    }
    virtual ~CEditTest()
    {

    };

protected:
    CApplication m_app;
    CRobotMain* m_robotMain;
    Gfx::CEngine * m_engine;
    Ui::CEdit * m_edit;
    CLogger m_logger;
};

using ::testing::_;
using ::testing::Return;

TEST_F(CEditTest, WriteTest)
{
    ASSERT_TRUE(true);
    CTextMock * text = dynamic_cast<CTextMock *>(m_engine->GetText());
    EXPECT_CALL(*text, GetCharWidth(_, _, _, _)).WillRepeatedly(Return(1.0f));
    EXPECT_CALL(*text, GetStringWidth(_, _, _, _)).WillOnce(Return(1.0f));
    std::string filename = "test.file";
    m_edit->SetMaxChar(Ui::EDITSTUDIOMAX);
    m_edit->SetAutoIndent(true);
    std::string inputScript = "{\ntext1\ntext2\n\ntext3\n{\ntext4\n}\n}";
    std::string expectedScript = "{\r\n\ttext1\r\n\ttext2\r\n\t\r\n\ttext3\r\n\t{\r\n\t\ttext4\r\n\t}\r\n}";
    m_edit->SetText(inputScript.c_str(), true);
    GetLogger()->Info("Writing text \n");
    m_edit->WriteText("script.txt");

    std::fstream scriptFile;

    scriptFile.open("script.txt", std::ios_base::binary | std::ios_base::in);
    std::string outputScript((std::istreambuf_iterator<char>(scriptFile)), std::istreambuf_iterator<char>());
    ASSERT_STREQ(expectedScript.c_str(), outputScript.c_str());
}

int main(int argc, char *argv[])
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

