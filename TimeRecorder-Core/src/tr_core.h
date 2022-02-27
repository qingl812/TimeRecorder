#ifndef TR_CORE_H__
#define TR_CORE_H__
#include "sqlite3.h"
#include <json.hpp>
#include <string>
#include <vector>
#include "api.h"

namespace tr {
struct activity_s {
    enum class TYPE {
        NONE = 0, // 获取程序信息失败，用户长时间无操作则不计使用时间等
        PROGRAM = 1,            // 普通程序
        PROGRAM_WITH_CHILD = 2, // 带有标签页的程序，如浏览器，VSCODE等
        MUSIC = 3               // 后台播放的音乐程序
    };
    activity_s::TYPE type;
    char path[260];      // 程序文件路径
    char title[260];     // 窗口标题
    char total_time[16]; // 总计运行时间
};

// 时间格式
// 2021/01/23 19:01:01:001
struct heartbeat_s {
    activity_s::TYPE type;
    char path[260];  // 程序文件路径
    char title[260]; // 窗口标题
    // 时间格式
    size_t julianday; // 儒略日数, 自公元前 4714 年 11 月 24 日正午算起的天数
    size_t begin_msecs; // 自今天开始过去的毫秒数
    size_t end_msecs;
};

struct record_s {
    size_t id;
    int total_time;
    // 最后一次运行时间
    int last_begin_time;
    int last_end_time;
};
} // namespace tr

// string 都使用 UTF-8 编码
namespace tr {
class Core {
public:
    Core();
    ~Core();

    void interact(); // 等待用户输入指令, 并执行对应操作
    int exec();      // 进入事件循环

private:
    sqlite3* m_sqlite;

private:
    // 获取前台（焦点）窗口的程序信息, 并调用 insert_heartbeat 插入到数据库
    void heartbeat_foreground_program();
    // 定时调用此函数，检测焦点窗口, 检测越频繁，记录越精确
    void heartbeat();
    // 显示所有记录的前台活动程序及其使用时间
    void show_all_foreground_program();

    static void GBKToUTF8(const wchar_t* src, char* dst);

private:
    /* 关于数据量, 假设每 100ms 产生一条数据
     * 10/s
     * 864,000/day
     * 315,360,000/year
     * sqlite3 整数最大值 2^63 - 1 == 9,223,372,036,854,775,807
     * 所以完全可以靠一个表存储所有数据
     * 关于查找效率
     * 由于 activity_record 表中 id 是递增的
     * 所以说 id 越大 begin_time, end_time 越大
     * 单独建一个表，用于记录 id 对应的事件，这样可以提高按时间查找的效率
     * 再建一个表记录某时间段内，某程序的时间统计，这样可以提高按程序的统计效率
     */

    void init_sql(); // 初始化 sql
    // 把 activity_s 插入到数据库中
    void insert_heartbeat(tr::heartbeat_s);
    void get_all_activities_by_path(std::vector<activity_s>* activities);
    void get_all_activities(std::vector<activity_s>* activities);

    static inline void get_db_path(char* const db_path);
    static inline void get_statement_init(char* const sql);
    static inline void
    get_statement_insert_activity_record(const tr::heartbeat_s& hb,
                                         char* const sql);
    static inline void get_statement_select_all_activities(char* const sql);
    static inline void
    get_statement_select_all_activities_by_path(char* const sql);

    // callback
    static int get_all_activities_callback(void*, int, char**, char**);

private:
    // 退出程序
    static void exit(int code);
    // 获取当前日期和时间, 参数分别是: 儒略日数, 自今天开始过去的毫秒数
    static void get_now_time(size_t* julianday, size_t* msecs);
    // 毫秒转为字符串, 102:02:01
    static void msec_to_string(size_t msec, char* str);
};
} // namespace tr
#endif // !TR_CORE_H__0