/* TimeRecorderCore api
 * version 0.1.0
 * last modify: 2022/02/06
 */

#ifndef API_H__
#define API_H__
#include "json.hpp"
#include <vector>

namespace tr {
namespace api {
struct application {
    char name[256];
    char recently[16];
    char today[16];
    char total[16];
};

// 二维数组 第一个元素表示总计 从第二个开始为每个窗口的信息
using _apps = std::vector<std::vector<application>>;
} // namespace api
} // namespace tr

// json convert
namespace nlohmann {
template <> struct adl_serializer<tr::api::application> {
    static void to_json(json& j, const tr::api::application& target) {
        j = {{"name", target.name},
             {"recently", target.recently},
             {"today", target.today},
             {"total", target.total}};
    }
    static void from_json(const json& j, tr::api::application& target) {
        strcpy(target.name, j["name"].get<std::string>().c_str());
        strcpy(target.recently, j["recently"].get<std::string>().c_str());
        strcpy(target.today, j["today"].get<std::string>().c_str());
        strcpy(target.total, j["total"].get<std::string>().c_str());
    }
};
} // namespace nlohmann

#endif // !API_H__