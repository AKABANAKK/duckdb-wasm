#include <sstream>

#include "duckdb/common/types/date.hpp"
#include "duckdb/common/types/timestamp.hpp"
#include "duckdb/execution/operator/persistent/buffered_csv_reader.hpp"
#include "duckdb/web/io/ifstream.h"
#include "duckdb/web/io/memory_filesystem.h"
#include "duckdb/web/test/config.h"
#include "duckdb/web/webdb.h"
#include "gtest/gtest.h"

using namespace duckdb::web;

namespace {

struct JSONInsertTest {
    struct TestPrinter {
        std::string operator()(const ::testing::TestParamInfo<JSONInsertTest>& info) const {
            return std::string{info.param.name};
        }
    };
    std::string_view name;
    std::string_view input;
    std::string_view options;
    std::string_view query;
    std::string expected_output;
};

struct JSONInsertTestSuite : public testing::TestWithParam<JSONInsertTest> {};

TEST_P(JSONInsertTestSuite, TestImport) {
    constexpr const char* path = "TEST";

    auto& test = GetParam();
    std::vector<char> input_buffer{test.input.data(), test.input.data() + test.input.size()};
    auto memory_filesystem = std::make_unique<io::MemoryFileSystem>();
    ASSERT_TRUE(memory_filesystem->RegisterFileBuffer(path, std::move(input_buffer)).ok());

    auto db = std::make_shared<WebDB>(NATIVE, std::move(memory_filesystem));
    WebDB::Connection conn{*db};
    auto maybe_ok = conn.InsertJSONFromPath(path, test.options);
    ASSERT_TRUE(maybe_ok.ok()) << maybe_ok.message();

    auto result = conn.connection().Query(std::string{test.query});
    ASSERT_STREQ(result->ToString().c_str(), std::string{test.expected_output}.c_str());
}

// clang-format off
static std::vector<JSONInsertTest> JSON_IMPORT_TEST = {
    {
        .name = "rows_integers",
        .input = R"JSON([
    {"a":1, "b":2, "c":3},
    {"a":4, "b":5, "c":6},
    {"a":7, "b":8, "c":9},
])JSON",
        .options = R"JSON({
            "schema": "main",
            "name": "foo"
        })JSON",
        .query = "SELECT * FROM main.foo",
        .expected_output = 
R"TXT(a	b	c	
INTEGER	INTEGER	INTEGER	
[ Rows: 3]
1	2	3	
4	5	6	
7	8	9	

)TXT"
    },
    {
        .name = "cols_integers",
        .input = R"JSON({
    "a": [1, 4, 7],
    "b": [2, 5, 8],
    "c": [3, 6, 9]
})JSON",
        .options = R"JSON({
            "schema": "main",
            "name": "foo"
        })JSON",
        .query = "SELECT * FROM main.foo",
        .expected_output = 
R"TXT(a	b	c	
INTEGER	INTEGER	INTEGER	
[ Rows: 3]
1	2	3	
4	5	6	
7	8	9	

)TXT"
    },
    {
        .name = "options_1",
        .input = R"JSON([
    {"a":1, "b":2, "c":3},
    {"a":4, "b":5, "c":6},
    {"a":7, "b":8, "c":9},
])JSON",
        .options = R"JSON({
            "schema": "main",
            "name": "foo",
            "shape": "row-array",
            "columns": [
                { "name": "a", "sqlType": "int32" },
                { "name": "b", "sqlType": "int16" },
                { "name": "c", "sqlType": "utf8" }
            ]
        })JSON",
        .query = "SELECT * FROM main.foo",
        .expected_output = 
R"TXT(a	b	c	
INTEGER	SMALLINT	VARCHAR	
[ Rows: 3]
1	2	3	
4	5	6	
7	8	9	

)TXT"
    },
};
// clang-format on

INSTANTIATE_TEST_SUITE_P(JSONInsertTest, JSONInsertTestSuite, testing::ValuesIn(JSON_IMPORT_TEST),
                         JSONInsertTest::TestPrinter());

}  // namespace
