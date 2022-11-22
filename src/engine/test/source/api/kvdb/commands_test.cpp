#include <api/kvdb/commands.hpp>

#include <filesystem>
#include <fstream>

#include <gtest/gtest.h>

// TODO: "createKvdbCmd" tests section (To avoid conflicts) ------------------------------
class kvdbAPICreateCommand : public ::testing::Test
{

protected:
    static constexpr auto DB_NAME = "TEST_DB";
    static constexpr auto DB_NAME_2 = "TEST_DB_2";
    static constexpr auto DB_NAME_3 = "TEST_DB_3";
    static constexpr auto DB_NAME_WITH_SPACES = "TEST_DB SEPARATE NAME";
    static constexpr auto DB_DIR = "/tmp/kvdbTestDir/";
    static constexpr auto FILE_PATH = "/tmp/file";

    std::shared_ptr<KVDBManager> kvdbManager;

    virtual void SetUp()
    {
        if (std::filesystem::exists(DB_DIR))
        {
            std::filesystem::remove_all(DB_DIR);
        }
        kvdbManager = std::make_shared<KVDBManager>(DB_DIR);
        kvdbManager->addDb(DB_NAME);
    }

    virtual void TearDown()
    {
        if (std::filesystem::exists(kvdbAPICreateCommand::FILE_PATH))
        {
            std::filesystem::remove(kvdbAPICreateCommand::FILE_PATH);
        }
    }
};

TEST_F(kvdbAPICreateCommand, createKvdbCmdSimpleAddition)
{
    api::CommandFn cmd;
    ASSERT_NO_THROW(cmd = api::kvdb::cmds::createKvdbCmd(kvdbAPICreateCommand::kvdbManager));
    json::Json params {
        fmt::format("{{\"name\": \"{}\"}}", kvdbAPICreateCommand::DB_NAME_2).c_str()};
    auto response = cmd(params);
    // ASSERT_EQ(response.message().value(),"");
    ASSERT_TRUE(response.isValid());
    ASSERT_EQ(response.message().value(), "OK");
    ASSERT_EQ(response.error(), 200);

    // check response
    ASSERT_TRUE(response.message().has_value());
    ASSERT_EQ(response.message().value(), "OK");
}

TEST_F(kvdbAPICreateCommand, createKvdbCmdNamesInArray)
{
    api::CommandFn cmd;
    ASSERT_NO_THROW(cmd = api::kvdb::cmds::createKvdbCmd(kvdbAPICreateCommand::kvdbManager));
    json::Json params {fmt::format("{{\"name\": [\"{}\",\"{}\"]}}",
                                   kvdbAPICreateCommand::DB_NAME_2,
                                   kvdbAPICreateCommand::DB_NAME_3)
                           .c_str()};
    auto response = cmd(params);
    ASSERT_TRUE(response.isValid());
    ASSERT_EQ(response.error(), 400);
}

TEST_F(kvdbAPICreateCommand, createKvdbCmdEmptyName)
{
    api::CommandFn cmd;
    ASSERT_NO_THROW(cmd = api::kvdb::cmds::createKvdbCmd(kvdbAPICreateCommand::kvdbManager));
    json::Json params {fmt::format("{{\"name\": \"\"}}").c_str()};
    auto response = cmd(params);
    ASSERT_TRUE(response.isValid());
    ASSERT_EQ(response.error(), 400);
}

TEST_F(kvdbAPICreateCommand, createKvdbCmdEmptyParams)
{
    api::CommandFn cmd;
    ASSERT_NO_THROW(cmd = api::kvdb::cmds::createKvdbCmd(kvdbAPICreateCommand::kvdbManager));
    json::Json params {};
    auto response = cmd(params);
    ASSERT_TRUE(response.isValid());
    ASSERT_EQ(response.error(), 400);
}

TEST_F(kvdbAPICreateCommand, createKvdbCmdRepeatedName)
{
    api::CommandFn cmd;
    ASSERT_NO_THROW(cmd = api::kvdb::cmds::createKvdbCmd(kvdbAPICreateCommand::kvdbManager));
    json::Json params {
        fmt::format("{{\"name\": \"{}\"}}", kvdbAPICreateCommand::DB_NAME).c_str()};
    auto response = cmd(params);
    ASSERT_TRUE(response.isValid());
    ASSERT_EQ(response.error(), 400);

    // check response
    ASSERT_TRUE(response.message().has_value());
    ASSERT_EQ(
        response.message().value(),
        fmt::format("DB with name [{}] already exists.", kvdbAPICreateCommand::DB_NAME));
}

TEST_F(kvdbAPICreateCommand, createKvdbCmdNameWithSpaces)
{
    api::CommandFn cmd;
    ASSERT_NO_THROW(cmd = api::kvdb::cmds::createKvdbCmd(kvdbAPICreateCommand::kvdbManager));
    json::Json params {
        fmt::format("{{\"name\": \"{}\"}}", kvdbAPICreateCommand::DB_NAME_WITH_SPACES)
            .c_str()};
    auto response = cmd(params);
    ASSERT_TRUE(response.isValid());
    ASSERT_EQ(response.error(), 200);

    // check response
    ASSERT_TRUE(response.message().has_value());
    ASSERT_EQ(response.message().value(), "OK");
}

TEST_F(kvdbAPICreateCommand, createKvdbCmdWithFilling)
{
    // file creation
    if (!std::filesystem::exists(kvdbAPICreateCommand::FILE_PATH))
    {
        std::ofstream exampleFile(kvdbAPICreateCommand::FILE_PATH);
        if (exampleFile.is_open())
        {
            exampleFile << "keyA:valueA\n";
            exampleFile << "keyB:valueB\n";
            exampleFile.close();
        }
    }

    api::CommandFn cmd;
    ASSERT_NO_THROW(cmd = api::kvdb::cmds::createKvdbCmd(kvdbAPICreateCommand::kvdbManager));
    json::Json params {fmt::format("{{\"name\": \"{}\", \"path\":\"{}\"}}",
                                   kvdbAPICreateCommand::DB_NAME_2,
                                   kvdbAPICreateCommand::FILE_PATH)
                           .c_str()};
    auto response = cmd(params);
    ASSERT_TRUE(response.isValid());
    ASSERT_EQ(response.message().value(), "OK");
    ASSERT_EQ(response.error(), 200);

    // check response
    ASSERT_TRUE(response.message().has_value());
    ASSERT_EQ(response.message().value(), "OK");

    // check value
    auto handle = kvdbManager->getDB(kvdbAPICreateCommand::DB_NAME_2);
    if (!handle)
    {
        kvdbManager->addDb(kvdbAPICreateCommand::DB_NAME_2, false);
    }
    handle = kvdbManager->getDB(kvdbAPICreateCommand::DB_NAME_2);
    ASSERT_TRUE(handle);
    ASSERT_EQ("valueA", handle->read("keyA"));
    ASSERT_EQ("valueB", handle->read("keyB"));
}

TEST_F(kvdbAPICreateCommand, createKvdbCmdWithWrongFilling)
{
    // file creation
    if (!std::filesystem::exists(kvdbAPICreateCommand::FILE_PATH))
    {
        std::ofstream exampleFile(kvdbAPICreateCommand::FILE_PATH);
        if (exampleFile.is_open())
        {
            exampleFile << "keyA-valueA\n";
            exampleFile << "keyB,valueB\n";
            exampleFile.close();
        }
    }

    api::CommandFn cmd;
    ASSERT_NO_THROW(cmd = api::kvdb::cmds::createKvdbCmd(kvdbAPICreateCommand::kvdbManager));
    json::Json params {fmt::format("{{\"name\": \"{}\", \"path\":\"{}\"}}",
                                   kvdbAPICreateCommand::DB_NAME_2,
                                   kvdbAPICreateCommand::FILE_PATH)
                           .c_str()};
    auto response = cmd(params);
    ASSERT_TRUE(response.isValid());
    ASSERT_EQ(response.error(), 200);
    // check value
    auto handle = kvdbManager->getDB(kvdbAPICreateCommand::DB_NAME_2);
    if (!handle)
    {
        kvdbManager->addDb(kvdbAPICreateCommand::DB_NAME_2, false);
    }
    handle = kvdbManager->getDB(kvdbAPICreateCommand::DB_NAME_2);
    ASSERT_TRUE(handle);
    ASSERT_TRUE(handle->read("keyA").empty());
    ASSERT_TRUE(handle->read("keyB").empty());
}

TEST_F(kvdbAPICreateCommand, createKvdbCmdSingleValueFile)
{
    // file creation
    if (!std::filesystem::exists(kvdbAPICreateCommand::FILE_PATH))
    {
        std::ofstream exampleFile(kvdbAPICreateCommand::FILE_PATH);
        if (exampleFile.is_open())
        {
            exampleFile << "keyA\n";
            exampleFile << "keyB\n";
            exampleFile.close();
        }
    }

    api::CommandFn cmd;
    ASSERT_NO_THROW(cmd = api::kvdb::cmds::createKvdbCmd(kvdbAPICreateCommand::kvdbManager));
    json::Json params {fmt::format("{{\"name\": \"{}\", \"path\":\"{}\"}}",
                                   kvdbAPICreateCommand::DB_NAME_2,
                                   kvdbAPICreateCommand::FILE_PATH)
                           .c_str()};
    auto response = cmd(params);
    ASSERT_TRUE(response.isValid());
    ASSERT_EQ(response.error(), 200);

    // check response
    ASSERT_TRUE(response.message().has_value());
    ASSERT_EQ(response.message().value(), "OK");

    // check value
    auto handle = kvdbManager->getDB(kvdbAPICreateCommand::DB_NAME_2);
    if (!handle)
    {
        kvdbManager->addDb(kvdbAPICreateCommand::DB_NAME_2, false);
    }
    handle = kvdbManager->getDB(kvdbAPICreateCommand::DB_NAME_2);
    ASSERT_TRUE(handle);
    ASSERT_TRUE(handle->hasKey("keyA"));
    ASSERT_TRUE(handle->hasKey("keyB"));
}

TEST_F(kvdbAPICreateCommand, createKvdbCmdNonExistingFile)
{
    api::CommandFn cmd;
    ASSERT_NO_THROW(cmd = api::kvdb::cmds::createKvdbCmd(kvdbAPICreateCommand::kvdbManager));
    json::Json params {fmt::format("{{\"name\": \"{}\", \"path\":\"{}\"}}",
                                   kvdbAPICreateCommand::DB_NAME_2,
                                   kvdbAPICreateCommand::FILE_PATH)
                           .c_str()};
    auto response = cmd(params);
    ASSERT_TRUE(response.isValid());
    ASSERT_EQ(response.error(), 400);
}

// TODO: "deleteKvdbCmd" tests section (To avoid conflicts) ------------------------------
class kvdbAPIDeleteCommand : public ::testing::Test
{

protected:
    static constexpr auto DB_NAME = "TEST_DB";
    static constexpr auto DB_NAME_WITH_SPACES = "TEST_DB NAME";
    static constexpr auto DB_NAME_NOT_AVAILABLE = "TEST_DB_NOT_AVAILABLE";
    static constexpr auto DB_DIR = "/tmp/kvdbTestDir/";

    std::shared_ptr<KVDBManager> kvdbManager;

    virtual void SetUp()
    {
        if (std::filesystem::exists(DB_DIR))
        {
            std::filesystem::remove_all(DB_DIR);
        }
        kvdbManager = std::make_shared<KVDBManager>(DB_DIR);
        kvdbManager->addDb(DB_NAME);
    }

    size_t getNumberOfKVDBLoaded() { return kvdbManager->getAvailableKVDBs().size(); }

    virtual void TearDown() {}
};

TEST_F(kvdbAPIDeleteCommand, deleteKvdbCmdSimpleLoaded)
{
    // add DB name with spaces
    kvdbAPIDeleteCommand::kvdbManager->addDb(DB_NAME_WITH_SPACES);

    // delete first DB
    api::CommandFn cmd;
    ASSERT_NO_THROW(cmd = api::kvdb::cmds::deleteKvdbCmd(kvdbAPIDeleteCommand::kvdbManager));
    json::Json params {
        fmt::format("{{\"mustBeLoaded\": true, \"name\": \"{}\"}}", kvdbAPIDeleteCommand::DB_NAME).c_str()};
    auto response = cmd(params);
    ASSERT_TRUE(response.isValid());
    ASSERT_EQ(response.error(), 200);

    // check response
    ASSERT_TRUE(response.message().has_value());
    ASSERT_EQ(response.message().value(), "OK");

    // check remaining available DBs
    ASSERT_EQ(kvdbAPIDeleteCommand::getNumberOfKVDBLoaded(), 1);

    // delete DB named with spaces
    json::Json params_with_spaces {
        fmt::format("{{\"mustBeLoaded\": true, \"name\": \"{}\"}}", kvdbAPIDeleteCommand::DB_NAME_WITH_SPACES)
            .c_str()};
    response = cmd(params_with_spaces);
    ASSERT_TRUE(response.isValid());
    ASSERT_EQ(response.error(), 200);

    // check response
    ASSERT_TRUE(response.message().has_value());
    ASSERT_EQ(response.message().value(), "OK");

    // trying to delete again already deleted DB
    response = cmd(params);
    ASSERT_TRUE(response.isValid());
    ASSERT_EQ(response.error(), 400);
}

TEST_F(kvdbAPIDeleteCommand, deleteKvdbCmdDoesntExistLoaded)
{
    api::CommandFn cmd;
    ASSERT_NO_THROW(cmd = api::kvdb::cmds::deleteKvdbCmd(kvdbAPIDeleteCommand::kvdbManager));
    json::Json params {
        fmt::format("{{\"mustBeLoaded\": true, \"name\": \"{}\"}}", kvdbAPIDeleteCommand::DB_NAME_NOT_AVAILABLE)
            .c_str()};
    auto response = cmd(params);
    ASSERT_TRUE(response.isValid());
    ASSERT_EQ(response.error(), 400);

    // check remaining available DBs
    ASSERT_EQ(kvdbAPIDeleteCommand::getNumberOfKVDBLoaded(), 1);
}

// TODO: "dumpKvdbCmd" tests section (To avoid conflicts) --------------------------------
class kvdbAPIDumpCommand : public ::testing::Test
{

protected:
    static constexpr auto DB_NAME = "TEST_DB";
    static constexpr auto DB_NAME_2 = "TEST_DB_2";
    static constexpr auto DB_DIR = "/tmp/kvdbTestDir/";
    static constexpr auto FILE_PATH = "/tmp/file";

    std::shared_ptr<KVDBManager> kvdbManager;

    virtual void SetUp()
    {
        if (std::filesystem::exists(DB_DIR))
        {
            std::filesystem::remove_all(DB_DIR);
        }

        if (std::filesystem::exists(FILE_PATH))
        {
            std::filesystem::remove(FILE_PATH);
        }

        kvdbManager = std::make_shared<KVDBManager>(DB_DIR);
        kvdbManager->addDb(DB_NAME);
    }

    virtual void TearDown() {}
};

TEST_F(kvdbAPIDumpCommand, dumpKvdbCmdEmptyName)
{
    api::CommandFn cmd;
    ASSERT_NO_THROW(cmd = api::kvdb::cmds::dumpKvdbCmd(kvdbAPIDumpCommand::kvdbManager));
    json::Json params {
        fmt::format("{{\"name\": \"\"}}").c_str()};
    auto response = cmd(params);
    ASSERT_TRUE(response.isValid());
    ASSERT_EQ(response.error(), 400);

    // check response
    ASSERT_TRUE(response.message().has_value());
    ASSERT_EQ(response.message().value(), "Field \"name\" is empty.");
}

TEST_F(kvdbAPIDumpCommand, dumpKvdbCmdNoParameters)
{
    // delete first DB
    api::CommandFn cmd;
    ASSERT_NO_THROW(cmd = api::kvdb::cmds::dumpKvdbCmd(kvdbAPIDumpCommand::kvdbManager));
    json::Json params {};
    auto response = cmd(params);
    ASSERT_TRUE(response.isValid());
    ASSERT_EQ(response.error(), 400);

    // check response
    ASSERT_TRUE(response.message().has_value());
    ASSERT_EQ(response.message().value(), "Field \"name\" is missing.");
}

TEST_F(kvdbAPIDumpCommand, dumpKvdbCmdSimpleExecution)
{
    // create file with content
    if (!std::filesystem::exists(kvdbAPIDumpCommand::FILE_PATH))
    {
        std::ofstream exampleFile(kvdbAPIDumpCommand::FILE_PATH);
        if (exampleFile.is_open())
        {
            exampleFile << "keyA:ValA\n";
            exampleFile << "keyB:ValB\n";
            exampleFile.close();
        }
    }
    kvdbAPIDumpCommand::kvdbManager->CreateAndFillKVDBfromFile(DB_NAME_2, FILE_PATH);

    api::CommandFn cmd;
    ASSERT_NO_THROW(cmd = api::kvdb::cmds::dumpKvdbCmd(kvdbAPIDumpCommand::kvdbManager));
    json::Json params {
        fmt::format("{{\"name\": \"{}\"}}",DB_NAME_2).c_str()};
    auto response = cmd(params);
    ASSERT_TRUE(response.isValid());
    ASSERT_EQ(response.error(), 200);

    // check response
    ASSERT_TRUE(response.message().has_value());
    ASSERT_EQ(response.message().value(), "OK");

    //check content
    auto kvdbList = response.data().getArray();
    auto valor = response.data().prettyStr();
    ASSERT_TRUE(kvdbList.has_value());
    ASSERT_EQ(kvdbList.value().size(),2);
    ASSERT_EQ(kvdbList.value().at(0).getString("/value").value(),"ValA");
    ASSERT_EQ(kvdbList.value().at(1).getString("/value").value(),"ValB");
    ASSERT_EQ(kvdbList.value().at(0).getString("/key").value(),"keyA");
    ASSERT_EQ(kvdbList.value().at(1).getString("/key").value(),"keyB");
}

TEST_F(kvdbAPIDumpCommand, dumpKvdbCmdSimpleEmpty)
{
    api::CommandFn cmd;
    ASSERT_NO_THROW(cmd = api::kvdb::cmds::dumpKvdbCmd(kvdbAPIDumpCommand::kvdbManager));
    json::Json params {
        fmt::format("{{\"name\": \"{}\"}}",DB_NAME).c_str()};
    auto response = cmd(params);
    ASSERT_TRUE(response.isValid());
    ASSERT_EQ(response.error(), 400);

    // check response
    ASSERT_TRUE(response.message().has_value());
    ASSERT_EQ(response.message().value(), "KVDB has no content");
}

TEST_F(kvdbAPIDumpCommand, dumpKvdbCmdKVDBOnlyKeys)
{
    // create file with content
    if (!std::filesystem::exists(kvdbAPIDumpCommand::FILE_PATH))
    {
        std::ofstream exampleFile(kvdbAPIDumpCommand::FILE_PATH);
        if (exampleFile.is_open())
        {
            exampleFile << "keyA\n";
            exampleFile << "keyB\n";
            exampleFile.close();
        }
    }
    kvdbAPIDumpCommand::kvdbManager->CreateAndFillKVDBfromFile(DB_NAME_2, FILE_PATH);

    api::CommandFn cmd;
    ASSERT_NO_THROW(cmd = api::kvdb::cmds::dumpKvdbCmd(kvdbAPIDumpCommand::kvdbManager));
    json::Json params {
        fmt::format("{{\"name\": \"{}\"}}",DB_NAME_2).c_str()};
    auto response = cmd(params);
    ASSERT_TRUE(response.isValid());
    ASSERT_EQ(response.error(), 200);

    // check response
    ASSERT_TRUE(response.message().has_value());
    ASSERT_EQ(response.message().value(), "OK");

    //check content
    auto kvdbList = response.data().getArray();
    auto valor = response.data().prettyStr();
    ASSERT_TRUE(kvdbList.has_value());
    ASSERT_EQ(kvdbList.value().size(),2);
    ASSERT_EQ(kvdbList.value().at(0).getString("/key").value(),"keyA");
    ASSERT_EQ(kvdbList.value().at(1).getString("/key").value(),"keyB");
}

// TODO: "getKvdbCmd" tests section (To avoid conflicts) ---------------------------------

// TODO: "insertKvdbCmd" tests section (To avoid conflicts) ------------------------------

class kvdbAPIInsertCommand : public ::testing::Test
{

protected:
    static constexpr auto DB_NAME = "TEST_DB";
    static constexpr auto DB_DIR = "/tmp/kvdbTestDir/";

    std::shared_ptr<KVDBManager> kvdbManager;

    virtual void SetUp()
    {
        if (std::filesystem::exists(DB_DIR))
        {
            std::filesystem::remove_all(DB_DIR);
        }
        kvdbManager = std::make_shared<KVDBManager>(DB_DIR);
        kvdbManager->addDb(DB_NAME, false);
    }

    virtual void TearDown() {}
};

TEST_F(kvdbAPIInsertCommand, test)
{
    api::CommandFn cmd {};
    ASSERT_NO_THROW(cmd = api::kvdb::cmds::insertKvdbCmd(kvdbAPIInsertCommand::kvdbManager));
}

// TODO: "listKvdbCmd" tests section (To avoid conflicts) --------------------------------

class kvdbAPIListCommand : public ::testing::Test
{

protected:
    static constexpr auto DB_NAME = "TEST_DB";
    static constexpr auto DB_NAME_2 = "TEST_DB_2";
    static constexpr auto DB_NAME_DIFFERENT_START = "NOT_TEST_DB";
    static constexpr auto DB_DIR = "/tmp/kvdbTestDir/";

    std::shared_ptr<KVDBManager> kvdbManager;

    virtual void SetUp()
    {
        if (std::filesystem::exists(DB_DIR))
        {
            std::filesystem::remove_all(DB_DIR);
        }
        kvdbManager = std::make_shared<KVDBManager>(DB_DIR);
        kvdbManager->addDb(DB_NAME, false);
    }

    virtual void TearDown() {}
};

TEST_F(kvdbAPIListCommand, listKvdbCmdSingleDBLoaded)
{
    // add DB to loaded list
    kvdbManager->addDb(DB_NAME);

    api::CommandFn cmd;
    ASSERT_NO_THROW(cmd = api::kvdb::cmds::listKvdbCmd(kvdbAPIListCommand::kvdbManager));
    json::Json params {fmt::format("{{\"mustBeLoaded\": true}}").c_str()};
    auto response = cmd(params);
    ASSERT_TRUE(response.isValid());
    ASSERT_EQ(response.error(), 200);

    //check response
    auto kvdbList = response.data().getArray();
    ASSERT_TRUE(kvdbList.has_value());
    ASSERT_EQ(kvdbList.value().size(),1);
    ASSERT_EQ(kvdbList.value().at(0).getString().value(),DB_NAME);
}

TEST_F(kvdbAPIListCommand, listKvdbCmdNoneLoaded)
{
    // Deletes the only DB from the list
    kvdbAPIListCommand::kvdbManager->deleteDB(DB_NAME);

    api::CommandFn cmd;
    ASSERT_NO_THROW(cmd = api::kvdb::cmds::listKvdbCmd(kvdbAPIListCommand::kvdbManager));
    json::Json params {fmt::format("{{\"mustBeLoaded\": true}}").c_str()};
    auto response = cmd(params);
    ASSERT_TRUE(response.isValid());
    ASSERT_EQ(response.error(), 200);

    //check response
    auto val = response.data().prettyStr();
    auto kvdbList = response.data().getArray();
    ASSERT_FALSE(kvdbList.has_value());
}

TEST_F(kvdbAPIListCommand, listKvdbCmdMultipleLoaded)
{
    // Adds another DB to the list
    kvdbAPIListCommand::kvdbManager->addDb(DB_NAME);
    kvdbAPIListCommand::kvdbManager->addDb(DB_NAME_2);

    api::CommandFn cmd;
    ASSERT_NO_THROW(cmd = api::kvdb::cmds::listKvdbCmd(kvdbAPIListCommand::kvdbManager));
    json::Json params {fmt::format("{{\"mustBeLoaded\": true}}").c_str()};
    auto response = cmd(params);
    ASSERT_TRUE(response.isValid());
    ASSERT_EQ(response.error(), 200);

    //check response
    auto kvdbList = response.data().getArray();
    ASSERT_TRUE(kvdbList.has_value());
    ASSERT_EQ(kvdbList.value().size(),2);
    ASSERT_EQ(kvdbList.value().at(0).getString().value(),DB_NAME_2);
    ASSERT_EQ(kvdbList.value().at(1).getString().value(),DB_NAME);
}

TEST_F(kvdbAPIListCommand, listKvdbCmdWithFilteringLoaded)
{
    // Adds DB to the list
    kvdbAPIListCommand::kvdbManager->addDb(DB_NAME);
    // add a db wicha name starts different than the others
    kvdbAPIListCommand::kvdbManager->addDb(DB_NAME_DIFFERENT_START);

    api::CommandFn cmd;
    ASSERT_NO_THROW(cmd = api::kvdb::cmds::listKvdbCmd(kvdbAPIListCommand::kvdbManager));
    json::Json params_with_name_not {
    fmt::format("{{\"mustBeLoaded\": true, \"name\": \"NOT\"}}").c_str()};
    auto response = cmd(params_with_name_not);
    ASSERT_TRUE(response.isValid());
    ASSERT_EQ(response.error(), 200);

    //check response with different name filtered
    auto kvdbList = response.data().getArray();
    ASSERT_TRUE(kvdbList.has_value());
    ASSERT_EQ(kvdbList.value().size(),1);
    ASSERT_EQ(kvdbList.value().at(0).getString().value(),DB_NAME_DIFFERENT_START);

    // same procces filtering with previous name start
    json::Json params_with_name_test {
    fmt::format("{{\"mustBeLoaded\": true, \"name\": \"TEST_\"}}").c_str()};
    response = cmd(params_with_name_test);
    ASSERT_TRUE(response.isValid());
    ASSERT_EQ(response.error(), 200);

    kvdbList = response.data().getArray();
    ASSERT_TRUE(kvdbList.has_value());
    ASSERT_EQ(kvdbList.value().size(),1);
    ASSERT_EQ(kvdbList.value().at(0).getString().value(),DB_NAME);

    // checks without filtering
    ASSERT_TRUE(response.isValid());
    ASSERT_EQ(response.error(), 200);
    json::Json params_with_empty_name {
    fmt::format("{{\"mustBeLoaded\": true, \"name\": \"\"}}").c_str()};
    response = cmd(params_with_empty_name);
    kvdbList = response.data().getArray();
    ASSERT_TRUE(kvdbList.has_value());
    ASSERT_EQ(kvdbList.value().size(),2);
    ASSERT_EQ(kvdbList.value().at(1).getString().value(),DB_NAME);
    ASSERT_EQ(kvdbList.value().at(0).getString().value(),DB_NAME_DIFFERENT_START);

    // checks without filtering
    ASSERT_TRUE(response.isValid());
    ASSERT_EQ(response.error(), 200);
    json::Json params_with_wrong_name {
    fmt::format("{{\"mustBeLoaded\": true, \"name\": \"wrong_match\"}}").c_str()};
    response = cmd(params_with_wrong_name);
    kvdbList = response.data().getArray();
    ASSERT_FALSE(kvdbList.has_value());
}

// TODO: "removeKvdbCmd" tests section (To avoid conflicts) ------------------------------