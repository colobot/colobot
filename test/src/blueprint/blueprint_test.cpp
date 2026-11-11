/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2023, Daniel Roux, EPSITEC SA & TerranovaTeam
 * http://epsitec.ch; http://colobot.info; http://github.com/colobot
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see http://gnu.org/licenses
 */

#include "app/app.h"

#include "blueprint/blueprint.h"
#include "blueprint/blueprint_parser_exceptions.h"
#include "blueprint/blueprint_storage.h"
#include "blueprint/part/transportable_blueprint.h"

#include "common/system/system_other.h"

#include "object/object_type.h"

#include <filesystem>
#include <gtest/gtest.h>
#include <hippomocks.h>
#include <physfs.h>

using namespace HippoMocks;

class CBlueprintStorageUT : public testing::Test
{
protected:
    CBlueprintStorageUT() = default;
    ~CBlueprintStorageUT() noexcept = default;

    void SetUp() override;
    void TearDown() override;

protected:
    std::unique_ptr<CApplication> m_app;
    MockRepository m_mocks;
    CSystemUtils* m_systemUtils = nullptr;
};

void CBlueprintStorageUT::SetUp()
{
    // CApplication is required by CLevelParser::Load
    m_systemUtils = m_mocks.Mock<CSystemUtils>();
    m_mocks.OnCall(m_systemUtils, CSystemUtils::GetDataPath).Return("");
    m_mocks.OnCall(m_systemUtils, CSystemUtils::GetLangPath).Return("");
    m_mocks.OnCall(m_systemUtils, CSystemUtils::GetSaveDir).Return("");
    m_app = std::make_unique<CApplication>(m_systemUtils);

    // PHYSFS is required by CResourceManager::Exists
    ASSERT_FALSE(PHYSFS_isInit());
    ASSERT_EQ(PHYSFS_init("test"), 1);
    ASSERT_EQ(PHYSFS_mount(std::filesystem::current_path().string().c_str(), nullptr, 1), 1);
}

void CBlueprintStorageUT::TearDown()
{
    m_app.reset();

    if (PHYSFS_isInit())
    {
        PHYSFS_deinit();
    }
}

TEST_F(CBlueprintStorageUT, BadFiles)
{
    CBlueprintStorage storage;

    // Missing file -> throw exception
    const auto nonExistingFile = "blueprints_bad_filename.txt";
    ASSERT_FALSE(PHYSFS_exists(nonExistingFile));
    EXPECT_THROW(storage.LoadFromFile(nonExistingFile), CLevelParserException);

    // Bad command -> throw exception
    const auto badCmdFile = "blueprints_bad_cmd.txt";
    ASSERT_TRUE(PHYSFS_exists(badCmdFile));
    EXPECT_THROW(storage.LoadFromFile(badCmdFile), UnusedCmdException);

    // Bad key -> throw exception
    const auto badKeyFile = "blueprints_bad_key.txt";
    ASSERT_TRUE(PHYSFS_exists(badKeyFile));
    EXPECT_THROW(storage.LoadFromFile(badKeyFile), UnusedParamException);
}

TEST_F(CBlueprintStorageUT, GoodFile)
{
    CBlueprintStorage storage;

    // Test hardcoded properties
    storage.LoadFromHardcode();
    EXPECT_TRUE(storage.GetBlueprint(OBJECT_ATOMIC)->transportable->enabled);
    EXPECT_FALSE(storage.GetBlueprint(OBJECT_ATOMIC)->transportable->isRadioactive);
    EXPECT_FALSE(storage.GetBlueprint(OBJECT_BOMB)->transportable->enabled);
    EXPECT_FALSE(storage.GetBlueprint(OBJECT_BOMB)->transportable->isRadioactive);

    // Test loaded properties
    const auto blueprintFile = "blueprints.txt";
    ASSERT_TRUE(PHYSFS_exists(blueprintFile));
    ASSERT_NO_THROW(storage.LoadFromFile(blueprintFile));
    EXPECT_TRUE(storage.GetBlueprint(OBJECT_ATOMIC)->transportable->enabled);
    EXPECT_TRUE(storage.GetBlueprint(OBJECT_ATOMIC)->transportable->isRadioactive);
    EXPECT_TRUE(storage.GetBlueprint(OBJECT_BOMB)->transportable->enabled);
    EXPECT_FALSE(storage.GetBlueprint(OBJECT_BOMB)->transportable->isRadioactive);

    // Test default properties
    storage.Clear();
    EXPECT_FALSE(storage.GetBlueprint(OBJECT_ATOMIC)->transportable->enabled);
    EXPECT_FALSE(storage.GetBlueprint(OBJECT_ATOMIC)->transportable->isRadioactive);
    EXPECT_FALSE(storage.GetBlueprint(OBJECT_BOMB)->transportable->enabled);
    EXPECT_FALSE(storage.GetBlueprint(OBJECT_BOMB)->transportable->isRadioactive);
}
