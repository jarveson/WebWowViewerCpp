//
// Created by Deamon on 10/28/2023.
//

#include "FileListWindow.h"
#include "../../../database/csvtest/csv.h"

namespace FileListDB {
    struct FileRecord_FT5 {
        int fileDataId;
        std::string fileName;
        std::string fileType;
    };

    inline static auto makeStorage(const std::string &dataBaseFile) {
        using namespace sqlite_orm;
        return make_storage(dataBaseFile,
                            make_trigger("files_after_insert",
                                         after()
                                             .insert()
                                             .on<FileRecord>()
                                             .begin(
                                                 insert(into<FileRecord_FT5>(),
                                                        columns(&FileRecord_FT5::fileDataId,
                                                                &FileRecord_FT5::fileName),
                                                        values(std::make_tuple(new_(&FileRecord::fileDataId),
                                                                               new_(&FileRecord::fileName)
                                                               )
                                                        )
                                                 )
                                             )
                                             .end()),
                            make_trigger("files_after_delete",
                                         after()
                                             .delete_()
                                             .on<FileRecord>()
                                             .begin(
                                                 remove_all<FileRecord_FT5>(where(is_equal(old(&FileRecord::fileDataId), &FileRecord_FT5::fileDataId)))
                                             )
                                             .end()),
                            make_trigger("files_after_update",
                                         after()
                                             .delete_()
                                             .on<FileRecord>()
                                             .begin(
                                                 update_all(set(assign(&FileRecord_FT5::fileName, &FileRecord::fileName)),
                                                            where(is_equal(old(&FileRecord::fileDataId), &FileRecord_FT5::fileDataId)))
                                             )
                                             .end()),
                            make_virtual_table("files_FT5", using_fts5(make_column("fileName", &FileRecord_FT5::fileName), make_column("fileDataId", &FileRecord_FT5::fileDataId))),
                            make_index("idx_files_name", &FileRecord::fileName),
                            make_index("idx_files_type", &FileRecord::fileType),
                            make_table("files",
                                       make_column("fileDataId", &FileRecord::fileDataId, primary_key()),
                                       make_column("fileName", &FileRecord::fileName),
                                       make_column("file_type", &FileRecord::fileType)
                            )
        );
    };
}
class StatementHolderAbstract {
public:
    virtual ~StatementHolderAbstract() {};
    virtual std::vector<FileListDB::FileRecord> execute(decltype(FileListDB::makeStorage("")) &storage, int limit, int offset) = 0;
    virtual int getTotal() = 0;
};

decltype(FileListDB::makeStorage("")) *u_storage;

template<class T, std::enable_if_t<
    std::is_arithmetic_v<T>
>...>
constexpr auto const_abs(T const& x) noexcept
{
    return x < 0 ? -x : x;
}

template<int order>
class StatementHolderA {
public:
    std::function<int()> calcTotal;

public:
    auto createStatement(decltype(FileListDB::makeStorage("")) &storage,
                                       const std::string &searchClause, bool sortAsc) {
        using namespace sqlite_orm;

        auto whereClause = where(
            or_(
                like(&FileListDB::FileRecord::fileName, searchClause),
                like(&FileListDB::FileRecord::fileDataId, searchClause)
            )
        );

        calcTotal = [whereClause, &storage]() {
            return storage.count<FileListDB::FileRecord>(whereClause);
        };

        calcTotal();



        if constexpr (order == 1) {
            return [statement =
            storage.prepare(get_all<FileListDB::FileRecord>(
                whereClause,
                !sortAsc ?
                order_by(&FileListDB::FileRecord::fileDataId).desc() :
                order_by(&FileListDB::FileRecord::fileDataId).asc(),
                limit(2, offset(3))
            ))](decltype(storage) &storage, int limit, int offset) mutable {
                using namespace sqlite_orm;

                get<2>(statement) = limit;
                get<3>(statement) = offset;

                return storage.execute(statement);
            };
        } else if constexpr (order == 2) {
            return [statement =
            storage.prepare(get_all<FileListDB::FileRecord>(
                whereClause,
                !sortAsc ?
                order_by(&FileListDB::FileRecord::fileDataId).desc() :
                order_by(&FileListDB::FileRecord::fileDataId).asc(),
                limit(2, offset(3))
            ))](decltype(storage) &storage, int limit, int offset) mutable {
                using namespace sqlite_orm;

                get<2>(statement) = limit;
                get<3>(statement) = offset;

                return storage.execute(statement);
            };
        } else if constexpr (order == 3) {
            return [statement =
            storage.prepare(get_all<FileListDB::FileRecord>(
                whereClause,
                !sortAsc ?
                order_by(&FileListDB::FileRecord::fileDataId).desc() :
                order_by(&FileListDB::FileRecord::fileDataId).asc(),
                limit(2, offset(3))
            ))](decltype(storage) &storage, int limit, int offset) mutable {
                using namespace sqlite_orm;

                get<2>(statement) = limit;
                get<3>(statement) = offset;

                return storage.execute(statement);
            };
        }

        //std::cout << statement.sql() << std::endl;
//    std::cout << get<0>(statement) << std::endl;
//    std::cout << get<1>(statement) << std::endl;
//    std::cout << get<2>(statement) << std::endl;
//    std::cout << get<3>(statement) << std::endl;
    }
};

template<int order>
class StatementHolder: public StatementHolderAbstract, public StatementHolderA<order> {
private:
    decltype(((StatementHolderA<order> *)(nullptr))->createStatement(*u_storage,"", false)) m_statementLambda;
public:
    StatementHolder(decltype(FileListDB::makeStorage("")) &storage,const std::string &searchClause, bool sortAsc) :
        m_statementLambda(StatementHolderA<order>::createStatement(storage, searchClause, sortAsc)) {

    }

    std::vector<FileListDB::FileRecord> execute(decltype(FileListDB::makeStorage("")) &storage, int limit, int offset) override {
        return m_statementLambda(storage, limit, offset);
    }
    int getTotal() override {
        return StatementHolderA<order>::calcTotal();
    };
};




std::unique_ptr<StatementHolderAbstract>
statementFactory(decltype(FileListDB::makeStorage("")) &storage, const std::string &searchClause, int order) {
    switch(abs(order)) {
        case 2:
            return std::make_unique<StatementHolder<2>>(storage, searchClause, order > 0);
        case 3:
            return std::make_unique<StatementHolder<2>>(storage, searchClause, order > 0);
        case 1:
        default:
            return std::make_unique<StatementHolder<1>>(storage, searchClause, order > 0);
    }
}


enum class EnumParamsChanged {
    OFFSET_LIMIT = 0, SEARCH_STRING = 1, SORTING
};

class FileListLambdaInst : public FileListLamda {
public:
    FileListLambdaInst(std::string &searchClause, int &recordsTotal) : m_recordsTotal(recordsTotal)
    {
        dbThread = std::thread([&]() {
            decltype(FileListDB::makeStorage("")) storage = FileListDB::makeStorage("fileList.db3");
            storage.pragma.synchronous(0);
            storage.pragma.journal_mode(sqlite_orm::journal_mode::MEMORY);

            storage.sync_schema();

            std::unique_ptr<StatementHolderAbstract> statement = statementFactory(storage, searchClause, 1);

            auto doQuery = [&storage] (decltype(*statement) &statement, int limit, int offset) mutable {

            };

            while (!this->m_isTerminating) {
                auto stateWasChanged = stateChangeAwaiter.waitForNewInput();
                if (m_isTerminating)
                    continue;

                switch (stateWasChanged) {
                    case EnumParamsChanged::OFFSET_LIMIT:

                    case EnumParamsChanged::SEARCH_STRING:
                        calcTotal();
                        break;

                    case EnumParamsChanged::SORTING:
                        statement = statementFactory(storage, searchClause, 1);
                        break;
                }

                std::vector<DBResults> results;
                {
                    std::unique_lock lock(paramsChange);
                    for (auto const & request : m_requests) {
                        auto &result1 = results.emplace_back();
                        result1.offset = 0;
                        result1.records = statement->execute(storage, request.limit, request.offset);
                    }
                }
                setResults(results);
            }
        });
    }
    const std::vector<DBResults> getResults() override{
        std::unique_lock lock(resultsChange);

        return m_results;
    }
    void makeRequest(const std::vector<DbRequest> &newRequest) override {
        std::unique_lock lock(paramsChange);
        m_requests = newRequest;
    };
private:

    void setResults(const std::vector<DBResults> &results) {
        std::unique_lock lock(resultsChange);

        m_results = results;
    }

private:
    std::mutex paramsChange;
    std::mutex resultsChange;

    std::thread dbThread;
    bool m_isTerminating = false;

    std::function<void()> calcTotal;
    int &m_recordsTotal;
    int m_order;

    std::vector<DBResults> m_results;
    std::vector<DbRequest> m_requests;

    ProdConsumerIOConnector<EnumParamsChanged> stateChangeAwaiter = {m_isTerminating};
};

//--------------------------------------
// FileListWindow
// -------------------------------------

FileListWindow::FileListWindow(const HApiContainer &api) : m_api(api) {
    m_filesTotal = 0;
    m_showWindow = true;


    selectStatement = std::make_unique<FileListLambdaInst>(filterTextStr, m_filesTotal);
}

bool FileListWindow::draw() {
    ImGui::Begin("File list", &m_showWindow);
    {
        if (ImGui::Button("Import filelist.csv...")) {
            importCSV();
        }
        ImGui::SameLine();
        if (ImGui::Button("Scan repository...")) {

        }
        if (ImGui::InputText("Filter: ", filterText.data(), filterText.size()-1)) {
            filterText[filterText.size()-1] = 0;
            filterTextStr = filterText.data();
            filterTextStr = "%"+filterTextStr+"%";
            m_fileRecCache.clear();
        }

        if (ImGui::BeginTable("FileListTable", 3, ImGuiTableFlags_Resizable |
                                              ImGuiTableFlags_Reorderable |
                                              ImGuiTableFlags_Sortable |
                                              ImGuiTableFlags_NoHostExtendX |
                                              ImGuiTableFlags_NoHostExtendY |
                                              ImGuiTableFlags_ScrollX, ImVec2(-1, -1))) {

            ImGui::TableSetupColumn("FileDataId", ImGuiTableColumnFlags_None);
            ImGui::TableSetupColumn("FileName", ImGuiTableColumnFlags_None);
            ImGui::TableSetupColumn("FileType", ImGuiTableColumnFlags_None);
            ImGui::TableSetupScrollFreeze(0, 1); // Make top row always visible
            ImGui::TableHeadersRow();

            auto dbResults = selectStatement->getResults();
            bool needRequest = false;
            std::vector<DbRequest> newRequests = {};
            {
                ImGuiListClipper clipper;
                clipper.Begin(m_filesTotal);
                while (clipper.Step()) {
                    auto f_amount = clipper.DisplayEnd-clipper.DisplayStart;
                    auto f_offset = clipper.DisplayStart;

                    std::vector<FileListDB::FileRecord> fileRecords;
                    for (auto &dbResult : dbResults) {
                        if ((dbResult.offset - f_offset) <= f_amount) {
                            int pre = std::max<int>(dbResult.offset - f_offset, 0);
                            int start = std::max<int>(f_offset-dbResult.offset, 0);
                            int size = f_amount - pre - start;
                            int post = f_amount - size;

                            for (int i = 0; i < pre; i++) {
                                ImGui::TableNextRow();
                                ImGui::TableNextColumn();
                                ImGui::Text("");
                                ImGui::TableNextColumn();
                                ImGui::Text("LOADING...");
                                ImGui::TableNextColumn();
                                ImGui::Text("");
                            }

                            for (int i = start; i < size; i++) {
                                auto const &fileItem = fileRecords[i];
                                ImGui::TableNextRow();
                                ImGui::TableNextColumn();
                                ImGui::Text("%d", fileItem.fileDataId);
                                ImGui::TableNextColumn();
                                ImGui::Text(fileItem.fileName.c_str());
                                ImGui::TableNextColumn();
                                ImGui::Text(fileItem.fileType.c_str());
                            }
                            for (int i = 0; i < post; i++) {
                                ImGui::TableNextRow();
                                ImGui::TableNextColumn();
                                ImGui::Text("");
                                ImGui::TableNextColumn();
                                ImGui::Text("LOADING...");
                                ImGui::TableNextColumn();
                                ImGui::Text("");
                            }

                            newRequests.emplace_back()
                            if (pre > 0 || post > 0) needRequest = true;
                            break;
                        }
                    }
                }
            }
            selectStatement->makeRequest(f_offset, f_amount);

            m_fileRecCache = newCache;
            ImGui::EndTable();
        }
    }
    ImGui::End();

    return m_showWindow;
}

void FileListWindow::importCSV() {
//    auto csv = new io::CSVReader<2, io::trim_chars<' '>, io::no_quote_escape<';'>>("listfile.csv");
//
//    int currentFileDataId;
//    std::string currentFileName;
//
//    using namespace sqlite_orm;
//    auto statement = m_storage.prepare(
//        select(
//            columns(&FileListDB::FileRecord::fileType), from<FileListDB::FileRecord>(),
//            where(is_equal(&FileListDB::FileRecord::fileDataId, std::ref(currentFileDataId)))
//        )
//    );
//
//    while (csv->read_row(currentFileDataId, currentFileName)) {
//        auto typeFromDB = m_storage.execute(statement);
//
//        std::string fileType = "";
//        if (!typeFromDB.empty()) {
//            fileType = get<0>(typeFromDB[0]);
//        }
//
//        m_storage.replace(FileListDB::FileRecord({currentFileDataId, currentFileName, fileType}));
//    }
//
//    m_filesTotal = m_storage.count<FileListDB::FileRecord>();
//
//    delete csv;
}
