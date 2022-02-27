#include "tr_core.h"

#include <Windows.h>
#include <codecvt>
#include <conio.h>
#include <iostream>
#include <locale>
#include <sstream>
#include <string>
#include <thread>
#include <time.h>

#include <Psapi.h>
#include <TlHelp32.h>

using namespace tr;

Core::Core() {
    char path[260];
    get_db_path(path);

    if (sqlite3_open(path, &m_sqlite) != SQLITE_OK) {
        std::cout << "open sqlite3 file faliled\n";
        exit(-1);
    }
    init_sql();
}

Core::~Core() {
    while (sqlite3_close(m_sqlite) == SQLITE_BUSY)
        ; // 数据库关闭失败
}

int Core::exec() {
    // 异步检测
    std::thread trecord([this] {
        while (true) {
            this->heartbeat();
            Sleep(100);
        }
    });

    // 等待用户输入指令
    interact();
    trecord.join();
    return 0;
}

void Core::heartbeat() { heartbeat_foreground_program(); }

void Core::interact() {
    while (true) {
        std::string str;
        std::stringstream sstream;
        std::string cmd;
        std::string para;
        int n;

        std::cin >> str;
        sstream << str;
        sstream >> cmd;

        if (cmd == "ls") {
            show_all_foreground_program();
        } else if (cmd == "lsa") {
            std::vector<activity_s> activities;
            get_all_activities(&activities);

            system("cls");
            std::cout << "all time records:\n";
            std::cout << "path\t\t\ttitle\t\t\ttime\n";
            for (auto& i : activities) {
                std::cout << i.path << "\t\t\t" << i.title << "\t\t\t"
                          << i.total_time << std::endl;
            }
        }
    }
}

namespace ns {
// a simple struct to model a person
struct person {
    std::string name;
    std::string address;
    int age;
};
} // namespace ns

ns::person p = {"Ned Flanders", "744 Evergreen Terrace", 60};

void Core::show_all_foreground_program() {
    std::vector<activity_s> activities;
    get_all_activities(&activities);
    tr::api::_apps apps;

    for (auto& target : activities) {
        bool sign = false;
        for (auto& i : apps) {
            if (strcmp(i[0].name, target.path) == 0) {
                tr::api::application app;
                strcpy(app.name, target.title);
                strcpy(app.total, target.total_time);
                app.recently[0] = 0;
                app.today[0] = 0;
                i.push_back(app);
                sign = true;
                break;
            }
        }
        if (sign == false) {
            std::vector<tr::api::application> single;
            tr::api::application app;
            strcpy(app.name, target.path);
            strcpy(app.total, target.total_time);
            app.recently[0] = 0;
            app.today[0] = 0;
            single.push_back(app);
            apps.push_back(single);
        }
    }

    for (auto& i : apps) {
        for (auto& j : i) {
            std::string name = j.name;
            auto pos = name.find_last_of('\\');
            strcpy(j.name,
                   name.substr(pos + 1, name.size() - 4 - pos - 1).c_str());
        }
    }

    std::cout << nlohmann::json(apps);
}

void Core::GBKToUTF8(const wchar_t* src, char* dst) {
    static std::wstring_convert<std::codecvt_utf8<wchar_t>> wcv; // GBK 转 utf-8
    strcpy(dst, wcv.to_bytes(src).c_str());
}

void Core::heartbeat_foreground_program() {
    static bool static_init = false;
    static size_t begin_day, end_day, begin_msec, end_msec;
    if (static_init == false) {
        get_now_time(&end_day, &end_msec);
        static_init = true;
    }

    begin_day = end_day;
    begin_msec = end_msec;

    static bool sign; // 程序路径获取成功?
    static heartbeat_s program;
    static LPWSTR buffer = new WCHAR[260]; // 供获取程序路径使用的缓冲区
    static DWORD pid;
    static HWND hwnd;
    static HANDLE hProcess;
    // 获取前台窗口信息
    hwnd = GetForegroundWindow();
    GetWindowThreadProcessId(hwnd, &pid);
    // 获取 exepath
    hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid);
    sign = K32GetModuleFileNameExW(hProcess, NULL, buffer, 260);
    if (sign) {
        program.type = activity_s::TYPE::PROGRAM;
        GBKToUTF8(buffer, program.path);
        // 获取窗口标题
        if (GetWindowTextW(hwnd, buffer, 260))
            GBKToUTF8(buffer, program.title);
        else
            program.title[0] = '\0';
    }

    get_now_time(&end_day, &end_msec);
    // 如果获取程序路径成功
    if (sign) {
        if (begin_day == end_day) {
            program.julianday = begin_day;
            program.begin_msecs = begin_msec;
            program.end_msecs = end_msec;
            insert_heartbeat(program);
        } else {
            // begin_time end_time 不在同一天
            program.julianday = begin_day;
            program.begin_msecs = begin_msec;
            program.end_msecs = 1000 * 3600 * 24;
            insert_heartbeat(program);

            program.julianday = end_day;
            program.begin_msecs = 0;
            program.end_msecs = end_msec;
            insert_heartbeat(program);
        }
    }
}

void Core::init_sql() {
    char sql[4096];
    get_statement_init(sql);

    char* errmsg = nullptr;
    if (sqlite3_exec(m_sqlite, sql, nullptr, nullptr, &errmsg) != SQLITE_OK) {
        std::cout << "create sql table faliled\n";
        std::cout << errmsg << "\n";
        exit(-1);
    }
}

void Core::insert_heartbeat(heartbeat_s activity) {
    char sql[4096];
    char* errmsg = NULL;
    if (activity.type == activity_s::TYPE::PROGRAM) {
        get_statement_insert_activity_record(activity, sql);
        if (sqlite3_exec(m_sqlite, sql, NULL, NULL, &errmsg) != SQLITE_OK) {
            std::cout << "insert activity record failed\n";
            std::cout << errmsg << "\n";
            exit(-1);
        }
    }
}

void Core::get_all_activities_by_path(std::vector<activity_s>* activities) {
    char sql[4096];
    char* errmsg = NULL;
    get_statement_select_all_activities_by_path(sql);

    activities->clear();

    if (sqlite3_exec(m_sqlite, sql, get_all_activities_callback,
                     (void*)activities, &errmsg) != SQLITE_OK) {
        std::cout
            << "select all activities group by path activity record failed\n";
        std::cout << errmsg << "\n";
        exit(-1);
    }
}

void Core::get_all_activities(std::vector<activity_s>* activities) {
    char sql[4096];
    char* errmsg = NULL;
    get_statement_select_all_activities(sql);

    activities->clear();

    if (sqlite3_exec(m_sqlite, sql, get_all_activities_callback,
                     (void*)activities, &errmsg) != SQLITE_OK) {
        std::cout << "select all activities group by path, title activity "
                     "record failed\n";
        std::cout << errmsg << "\n";
        exit(-1);
    }
}

int Core::get_all_activities_callback(void* para, int columenCount,
                                      char** columnValue, char** columnName) {
    auto activities = (std::vector<activity_s>*)para;
    activity_s activity;
    activity.type = activity_s::TYPE::PROGRAM;
    activity.path[0] = 0;
    activity.title[0] = 0;
    activity.total_time[0] = 0;

    for (int i = 0; i < columenCount; i++) {
        if (strcmp(columnName[i], "path") == 0) {
            strcpy(activity.path, columnValue[i]);
        } else if (strcmp(columnName[i], "title") == 0) {
            strcpy(activity.title, columnValue[i]);
        } else if (strcmp(columnName[i], "msec") == 0) {
            auto msec = atoi(columnValue[i]);
            msec_to_string(msec, activity.total_time);
        }
    }
    activities->push_back(activity);
    return 0;
}

void Core::exit(int code) { ::exit(code); }

void Core::get_now_time(size_t* julianday, size_t* msecs) {
    SYSTEMTIME st;
    GetLocalTime(&st);
    if (julianday != nullptr) {
        // 公历（格里历）转儒略日
        // https://zh.wikipedia.org/wiki/%E5%84%92%E7%95%A5%E6%97%A5
        size_t a = (14 - st.wMonth) / 12;
        size_t y = st.wYear + 4800 - a;
        size_t m = st.wMonth + 12 * a - 3;
        *julianday = st.wDay + (153 * m + 2) / 5 + 365 * y + y / 4 - y / 100 +
                     y / 400 - 32045;
    }
    if (msecs != nullptr) {
        *msecs = st.wMilliseconds + st.wSecond * 1000 + st.wMinute * 60000 +
                 st.wHour * 3600000;
    }
}

void Core::get_db_path(char* const db_path) {
    wchar_t buffer[MAX_PATH];
    GetModuleFileNameW(NULL, buffer, MAX_PATH);
    GBKToUTF8(buffer, db_path);

    std::string str(db_path);
    auto pos = str.find_last_of("\\");
    strcpy(db_path, str.substr(0, pos).c_str());

    strcat(db_path, "\\tr_core.db");
}

void Core::get_statement_init(char* const sql) {
    const char* pre_sql = R"(
            CREATE TABLE IF NOT EXISTS activity_record (
                id INTEGER NOT NULL PRIMARY KEY,
                path TEXT(260) NOT NULL,
                title TEXT(260) NOT NULL,
                julianday INTEGER  NOT NULL,
                begin_msec INTEGER NOT NULL,
                end_msec INTEGER NOT NULL
            );

            CREATE TRIGGER IF NOT EXISTS insert_activity_record
            AFTER INSERT ON activity_record
            WHEN EXISTS (
            	SELECT path, title, julianday, begin_msec as msec FROM activity_record WHERE id = NEW.id
            	INTERSECT
            	SELECT path, title, julianday, end_msec as msec FROM activity_record WHERE id = NEW.id - 1)
            begin
            	UPDATE activity_record SET end_msec = NEW.end_msec WHERE id = NEW.id - 1;
            	DELETE FROM activity_record WHERE id = NEW.id;
            end;
        )";
    strcpy(sql, pre_sql);
}

void Core::get_statement_insert_activity_record(const tr::heartbeat_s& hb,
                                                char* const sql) {
    const char* pre_sql = R"(
            INSERT INTO activity_record (path, title, julianday, begin_msec, end_msec)
            VALUES('%s', '%s', %zu, %zu, %zu);
        )";

    sprintf(sql, pre_sql, hb.path, hb.title, hb.julianday, hb.begin_msecs,
            hb.end_msecs);
}

void Core::get_statement_select_all_activities(char* const sql) {
    const char* pre_sql = R"(
            SELECT path, title, SUM(end_msec - begin_msec) AS msec
            FROM activity_record
            GROUP BY path, title;
        )";
    strcpy(sql, pre_sql);
}

void Core::get_statement_select_all_activities_by_path(char* const sql) {
    const char* pre_sql = R"(
            SELECT path, SUM(end_msec - begin_msec) AS msec
            FROM activity_record
            GROUP BY path;
        )";
    strcpy(sql, pre_sql);
}

void Core::msec_to_string(size_t msec, char* str) {
    int second = msec / 1000;
    int minute = second / 60;
    int hour = minute / 60;
    second %= 60;
    minute %= 60;

    sprintf(str, "%d:%02d:%02d", hour, minute, second);
}