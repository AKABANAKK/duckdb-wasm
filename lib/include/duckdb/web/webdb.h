#ifndef INCLUDE_DUCKDB_WEB_WEBDB_H_
#define INCLUDE_DUCKDB_WEB_WEBDB_H_

#include <cstring>
#include <duckdb/common/file_system.hpp>
#include <initializer_list>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>

#include "arrow/io/buffered.h"
#include "arrow/io/interfaces.h"
#include "arrow/ipc/writer.h"
#include "duckdb.hpp"
#include "duckdb/main/query_result.hpp"
#include "duckdb/parser/parser.hpp"
#include "duckdb/web/config.h"
#include "duckdb/web/io/buffered_filesystem.h"
#include "duckdb/web/io/default_filesystem.h"
#include "duckdb/web/io/file_page_buffer.h"
#include "duckdb/web/miniz_zipper.h"
#include "nonstd/span.h"

namespace duckdb {
namespace web {

class WebDB {
   public:
    /// A connection
    class Connection {
       protected:
        /// The webdb
        WebDB& webdb_;
        /// The connection
        duckdb::Connection connection_;

        /// The current result (if any)
        std::unique_ptr<duckdb::QueryResult> current_query_result_ = nullptr;
        /// The current arrow schema (if any)
        std::shared_ptr<arrow::Schema> current_schema_ = nullptr;

       public:
        /// Constructor
        Connection(WebDB& webdb);

        /// Get a connection
        auto& connection() { return connection_; }
        /// Get the filesystem
        duckdb::FileSystem& filesystem();

        /// Run a query and return an arrow buffer
        arrow::Result<std::shared_ptr<arrow::Buffer>> RunQuery(std::string_view text);
        /// Send a query and return an arrow buffer
        arrow::Result<std::shared_ptr<arrow::Buffer>> SendQuery(std::string_view text);
        /// Fetch query results and return an arrow buffer
        arrow::Result<std::shared_ptr<arrow::Buffer>> FetchQueryResults();

        /// Import a csv file
        arrow::Status ImportCSVTable(std::string_view path, std::string_view options);
        /// Import a json file
        arrow::Status ImportJSONTable(std::string_view path, std::string_view options);
    };

   protected:
    /// The buffer manager
    std::shared_ptr<io::FilePageBuffer> file_page_buffer_;
    /// The (shared) database
    std::shared_ptr<duckdb::DuckDB> database_;
    /// The connections
    std::unordered_map<Connection*, std::unique_ptr<Connection>> connections_;
    /// The database config
    duckdb::DBConfig db_config_;

    /// The pinned web files (if any)
    std::unordered_map<std::string_view, std::unique_ptr<io::WebFileSystem::WebFileHandle>> pinned_web_files_ = {};
    /// The zipper (if loaded)
    std::unique_ptr<Zipper> zip_ = nullptr;

   public:
    /// Constructor
    WebDB(std::unique_ptr<duckdb::FileSystem> fs = io::CreateDefaultFileSystem(), const char* path = nullptr);
    /// Destructor
    ~WebDB();

    /// Get the filesystem
    auto& filesystem() { return database_->GetFileSystem(); }
    /// Get the database
    auto& database() { return *database_; }
    /// Get the buffer manager
    auto& file_page_buffer() { return *file_page_buffer_; }
    /// Get the zipper
    auto* zip() { return zip_.get(); }

    /// Get the version
    std::string_view GetVersion();
    /// Get the feature flags
    uint32_t GetFeatureFlags() { return WEBDB_FEATURES; }
    /// Tokenize a script and return tokens as json
    std::string Tokenize(std::string_view text);
    /// Create a connection
    Connection* Connect();
    /// End a connection
    void Disconnect(Connection* connection);
    /// Flush all file buffers
    void FlushFiles();
    /// Flush file by path
    void FlushFile(std::string_view path);

    /// Register a file URL
    arrow::Status RegisterFileURL(std::string_view file_name, std::string_view file_url);
    /// Register a file URL
    arrow::Status RegisterFileBuffer(std::string_view file_name, std::unique_ptr<char[]> buffer, size_t buffer_length);
    /// Drop all files
    arrow::Status DropFiles();
    /// Drop a file
    arrow::Status DropFile(std::string_view file_name);
    /// Set a file descriptor
    arrow::Status SetFileDescriptor(uint32_t file_id, uint32_t fd);
    /// Set a file descriptor
    arrow::Result<std::string> GetFileInfo(uint32_t file_id);
    /// Copy a file to a buffer
    arrow::Result<std::shared_ptr<arrow::Buffer>> CopyFileToBuffer(std::string_view path);
    /// Copy a file to a path
    arrow::Status CopyFileToPath(std::string_view path, std::string_view out);

    /// Enable file statistics
    arrow::Status EnableFileStatistics(std::string_view path, bool enable);
    /// Export file block acceses
    arrow::Result<std::shared_ptr<arrow::Buffer>> ExportFileBlockStatistics(std::string_view path);

    /// Get the static webdb instance
    static WebDB& Get();
};

}  // namespace web
}  // namespace duckdb

#endif  // INCLUDE_DUCKDB_WEB_WEBDB_H_
