/**
 * @file hsc.cpp Homework Showing Compiler
 * @version 0.2.4 (Beta)
 * @author Jason M. Li
 * @date 2022.3.1
 */

#include <cstdio>
#include <iostream>
#include <fstream>
#include <stack>
#include <string>
#include <cstring>
#include <vector>
#include <map>
#include <ctime>
#define LINE_BUF 4096

enum SP_CH : short {
    META_DATE = 0, // 日期声明开始
    META_CREATED, // 创建声明开始
    META_MODIFIED, // 更正声明开始
    META_MODIFIED_COLOR, // 更正声明[参数]颜色开始
    START_SUBJECT, // 学科声明开始
    START_NOTE, // 注释部分开始
    START_ITEM, // 创建项目开始
    START_HYPERLINK, // 超链接开始
    START_HYPERLINK_TEXT, // 超链接文本开始(若指定文本)
    START_EMPHASIZE, // 突出开始
    START_EMPHASIZE_TEXT, // 突出文本开始(若指定版本)
    START_NOTE_SYMBOL, // 注释提示开始
    START_NOTE_SYMBOL_TEXT, // 注释文本开始(若指定提前或延后)
    START_ESCAPE, // 转义开始
    ROOT // 根元素/占位符
};

class Tag {
public:
    short sp_ch = -1;
    std::string temp_str = "";
    int temp_int = 0;
    Tag(short _sp_ch) {
        this->sp_ch = _sp_ch;
    }
};

class Modification {
public:
    std::string bg_color = "yellow";
    std::string modified_time = "--:--";
    Modification() {}
    Modification(std::string _bg_color, std::string _modified_time) {
        this->bg_color = _bg_color;
        this->modified_time = _modified_time;
    }
};

class Item {
public:
    std::string text;
    int number;
    Item() {}
    Item(std::string _text) {
        this->text = _text;
    }
    Item(std::string _text, int _number) {
        this->text = _text;
        this->number = _number;
    }
};

class Subject {
public:
    std::string name;
    std::vector<Item> items;
    Subject() {}
    Subject(std::string _name) {
        this->name = _name;
    }
};

class OutputFile {
public:
    std::string date = "--------";
    std::string created_time = "--:--";
    std::string published_time = "--:--:--";
    std::vector<Modification> modified_times;
    std::vector<Subject> subjects;
    OutputFile() {}
};

std::string add_label(std::string text, std::string tagName) {
    return '<' + tagName + '>' + text + "</" + tagName + '>';
}

std::string add_label(std::string text, std::string tagName, std::map<std::string, std::string> attrs) {
    std::string result = '<' + tagName;
    auto attr = attrs.begin();
    for (; attr != attrs.end(); attr++) {
        std::string temp_attr = ' ' + attr->first + "=\"" + attr->second + '"';
        result += temp_attr;
    }
    result += ">";
    result += text;
    result += "</" + tagName + '>';
    return result;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printf("Error 0x0001: No File Input.\n");
        return 0x0001;
    }
    std::string filename(argv[1]);
    std::ifstream reader;
    std::ofstream writer;
    reader.open(filename);
    if (!reader.is_open()) {
        printf("Error 0x0010: Read File Failed.\n");
        return 0x0010;
    }
    std::stack<Tag> tags;
    tags.push(Tag(SP_CH::ROOT));
    tags.push(Tag(SP_CH::ROOT)); // 根元素，防溢出
    OutputFile outfile;
    int note_counter = 0,
        number_counter = 0,
        line_counter = 0;
    char line_buffer[LINE_BUF];
    std::string str_buffer;
    int line_len;
    while (reader.getline(line_buffer, LINE_BUF)) {
        line_len = strlen(line_buffer);
        str_buffer = line_buffer;
        // 监测是否为行级元素
        {
            char first_ch = str_buffer[0];
            if (first_ch == '+') {
                std::string next_five_ch = str_buffer.substr(1, 5);
                if (next_five_ch.compare("date ") == 0) {
                    tags.push(Tag(SP_CH::META_DATE));
                }
                else if (next_five_ch.compare("crea ") == 0) {
                    tags.push(Tag(SP_CH::META_CREATED));
                }
                else if (next_five_ch.compare("modi ") == 0) {
                    tags.push(Tag(SP_CH::META_MODIFIED));
                }
                str_buffer = str_buffer.substr(6);
                line_len -= 6;
            }
            else if (first_ch == '-') {
                if (str_buffer[1] == ' ') {
                    tags.push(Tag(SP_CH::START_ITEM));
                    str_buffer = str_buffer.substr(2);
                    line_len -= 2;
                }
                else if (str_buffer.substr(1, 2).compare("--") == 0) {
                    tags.push(Tag(SP_CH::START_NOTE));
                    str_buffer = "";
                    line_len = 0;
                }
            }
            else if (first_ch == '#') {
                tags.push(Tag(SP_CH::START_SUBJECT));
                int start = 1;
                if (str_buffer[1] == ' ') start = 2;
                str_buffer = str_buffer.substr(start);
                line_len -= start;
            }
        }
        // 行内元素监测
        for (int i = 0; i < line_len; i++) {
            unsigned char ch = str_buffer[i];
            int tag_size = tags.size();
            int top_sp_ch = tags.top().sp_ch;
            if (tag_size == 0) {
                printf("ERROR 0x0020: no tags left.\n");
                return 0x0020;
            }
            if (tag_size == 1) {
                printf("WARNING: The last tag is ROOT tag.\n");
            }
            // 转义
            if (top_sp_ch == SP_CH::START_ESCAPE) {
                tags.pop();
                if (ch == 'n') tags.top().temp_str += '\n';
                else if (ch == 't') tags.top().temp_str += '\t';
                else tags.top().temp_str += ch;
            }
            else if (ch == '\\' && top_sp_ch != SP_CH::START_EMPHASIZE) {
                tags.push(Tag(SP_CH::START_ESCAPE));
            }
            // 注释
            else if (ch == '<' && i < line_len - 1 && str_buffer[i + 1] == '?') {
                tags.push(Tag(SP_CH::START_NOTE_SYMBOL));
                ++i;
            }
            else if ((ch == '+' || ch == '-') && top_sp_ch == SP_CH::START_NOTE_SYMBOL) {
                tags.top().temp_int = 44 - ch;
                tags.push(Tag(SP_CH::START_NOTE_SYMBOL_TEXT));
            }
            else if (ch >= '0' && ch <= '9' && top_sp_ch == SP_CH::START_NOTE_SYMBOL_TEXT) {
                tags.top().temp_str += ch;
            }
            else if (ch == '>' && top_sp_ch == SP_CH::START_NOTE_SYMBOL) {
                note_counter += 1;
                tags.pop();
                tags.top().temp_str += add_label("[注" + std::to_string(note_counter) + ']', "span", { {"class", "note"} });
            }
            else if (ch == '>' && top_sp_ch == SP_CH::START_NOTE_SYMBOL_TEXT) {
                note_counter += 1;
                int note_number = atoi(tags.top().temp_str.c_str());
                tags.pop();
                note_number *= tags.top().temp_int;
                note_number += note_counter;
                tags.pop();
                tags.top().temp_str += add_label("[注" + std::to_string(note_number) + ']', "span", { {"class", "note"} });
                note_counter -= 1;
            }
            // 强调
            else if (ch == '*' && top_sp_ch != SP_CH::START_EMPHASIZE_TEXT) {
                if (i < line_len - 1 && str_buffer[i + 1] == ':') {
                    tags.push(Tag(SP_CH::START_EMPHASIZE));
                    ++i;
                }
                else {
                    tags.push(Tag(SP_CH::START_EMPHASIZE));
                    tags.push(Tag(SP_CH::START_EMPHASIZE_TEXT));
                }
            }
            else if (ch >= '0' && ch <= '9' && top_sp_ch == SP_CH::START_EMPHASIZE) {
                tags.top().temp_str += ch;
            }
            else if (ch == ':' && top_sp_ch == SP_CH::START_EMPHASIZE) {
                int version = atoi(tags.top().temp_str.c_str());
                tags.top().temp_int = version;
                tags.top().temp_str.clear();
                tags.push(Tag(SP_CH::START_EMPHASIZE_TEXT));
            }
            else if (ch == '*' && top_sp_ch == SP_CH::START_EMPHASIZE_TEXT) {
                std::string text = tags.top().temp_str;
                tags.pop();
                int version = tags.top().temp_int;
                tags.pop();
                if (version == 0 || outfile.modified_times.size() < version) {
                    tags.top().temp_str += add_label(text, "strong");
                }
                else {
                    std::map<std::string, std::string> attrs;
                    attrs["style"] = "--bgc: " + outfile.modified_times[version - 1].bg_color;
                    tags.top().temp_str += add_label(text, "strong", attrs);
                }
            }
            // 超链接
            else if (ch == '[') {
                tags.push(Tag(SP_CH::START_HYPERLINK));
            }
            else if (ch == ']' && top_sp_ch == SP_CH::START_HYPERLINK) {
                tags.top().temp_int = 1;
            }
            else if (ch == '(' && top_sp_ch == SP_CH::START_HYPERLINK && tags.top().temp_int == 1) {
                tags.push(Tag(SP_CH::START_HYPERLINK_TEXT));
            }
            else if (ch == ')' && top_sp_ch == SP_CH::START_HYPERLINK && tags.top().temp_int == 1) {
                std::string url = tags.top().temp_str;
                tags.pop();
                std::map<std::string, std::string> attrs;
                attrs["href"] = url;
                tags.top().temp_str += add_label(url, "a", attrs);
            }
            else if (ch == ')' && top_sp_ch == SP_CH::START_HYPERLINK_TEXT) {
                std::string text = tags.top().temp_str;
                tags.pop();
                std::string url = tags.top().temp_str;
                tags.pop();
                std::map<std::string, std::string> attrs;
                attrs["href"] = url;
                tags.top().temp_str += add_label(text, "a", attrs);
            }
            // 空格分割的数据声明
            else if (ch == ' ' && top_sp_ch == SP_CH::META_MODIFIED) {
                tags.push(Tag(SP_CH::META_MODIFIED_COLOR));
            }
            else {
                tags.top().temp_str += ch;
            }
        }
        while (tags.top().sp_ch >= SP_CH::START_HYPERLINK && tags.top().sp_ch <= SP_CH::START_ESCAPE) {
            // 过滤堆栈未使用内容
            std::string temp_str = tags.top().temp_str;
            int temp_int = tags.top().temp_int;
            short top_sp_ch = tags.top().sp_ch;
            tags.pop();
            switch (top_sp_ch) {
            case (SP_CH::START_EMPHASIZE):
                tags.top().temp_str += "*:";
                break;
            case (SP_CH::START_EMPHASIZE_TEXT):
                if (tags.top().temp_int != 0) {
                    int temp_ver = tags.top().temp_int;
                    tags.pop();
                    tags.top().temp_str += "*:";
                    tags.top().temp_str += std::to_string(temp_ver);
                    tags.top().temp_str += ':';
                }
                else {
                    tags.pop();
                    tags.top().temp_str += '*';
                }
                break;
            case (SP_CH::START_ESCAPE):
                tags.top().temp_str += '\\';
                break;
            case (SP_CH::START_HYPERLINK):
                tags.top().temp_str += '[';
                if (temp_int == 1) {
                    tags.top().temp_str += temp_str;
                    tags.top().temp_str += ']';
                    temp_str.clear();
                }
                break;
            case (SP_CH::START_HYPERLINK_TEXT):
            {
                Tag temp_tag = tags.top();
                tags.pop();
                tags.top().temp_str += '[';
                tags.top().temp_str += temp_tag.temp_str;
                tags.top().temp_str += "](";
                temp_str.clear();
            }
            break;
            case (SP_CH::START_NOTE_SYMBOL):
                tags.top().temp_str += "<?";
                break;
            case (SP_CH::START_NOTE_SYMBOL_TEXT):
                tags.pop();
                tags.top().temp_str += "<?";
                tags.top().temp_str += (char)(44 + temp_int);
                break;
            }
            tags.top().temp_str += temp_str;
        }
        {
            // 监测堆栈/最近的行级项
            int top_sp_ch = tags.top().sp_ch;
            if (top_sp_ch == SP_CH::META_DATE) {
                outfile.date = tags.top().temp_str;
                tags.pop();
            }
            else if (top_sp_ch == SP_CH::META_CREATED) {
                outfile.created_time = tags.top().temp_str;
                tags.pop();
            }
            else if (top_sp_ch == SP_CH::META_MODIFIED) {
                outfile.modified_times.push_back(Modification("yellow", tags.top().temp_str));
                tags.pop();
            }
            else if (top_sp_ch == SP_CH::META_MODIFIED_COLOR) {
                std::string color = tags.top().temp_str;
                tags.pop();
                outfile.modified_times.push_back(Modification(color, tags.top().temp_str));
                tags.pop();
            }
            else if (top_sp_ch == SP_CH::START_NOTE) {
                number_counter = 0; // 复位
                outfile.subjects.push_back(Subject(add_label("备注", "span", { {"class", "note-part"} })));
            }
            else if (top_sp_ch == SP_CH::START_SUBJECT) {
                outfile.subjects.push_back(Subject(tags.top().temp_str));
                tags.pop();
            }
            else if (top_sp_ch == SP_CH::START_ITEM) {
                number_counter += 1;
                outfile.subjects.back().items.push_back(Item(tags.top().temp_str, number_counter));
                tags.pop();
            }
        }
        line_counter += 1;
    }
    reader.close();
    writer.open(filename.substr(0, filename.size() - 4) + ".html");
    if (!writer.is_open()) {
        printf("Error 0x0011: Write File Failed.\n");
        return 0x0011;
    }
    {
        // 确定发布时间
        time_t t = time(NULL);
        struct tm* local = localtime(&t);
        char temp_str[64] = { "\0" };
        strftime(temp_str, sizeof(temp_str), "%m-%d %H:%M:%S", local);
        outfile.published_time = temp_str;
    }
    {
        // 写文件
        writer << "<!DOCTYPE html>" << std::endl << "<html lang='zh-cn'><head><meta name='charset' content='utf-8'><meta name='generator' content='Visual Studio Code'><meta name='author' content='Jason Li'><meta name='robots' content='noindex'><title>作业</title><link rel='shortcut icon' href='img/cube.ico' type='image/x-icon'><link rel='stylesheet' href='style.css'></head>" << std::endl << "<body>";
        std::string published_note = add_label("发布于 " + outfile.published_time, "span");
        writer << published_note << std::endl;
        std::string table = "<thead><tr><td colspan='3'>" + outfile.date + "作业</td></tr><tr><td>科目</td><td>序号</td><td>项目</td></tr></thead>";
        std::string tbody = "";
        int subject_length = outfile.subjects.size();
        for (int i = 0; i < subject_length; i++) {
            Subject subject = outfile.subjects[i];
            int item_length = subject.items.size();
            Item item;
            if (item_length != 0) {
                item = subject.items[0];
                std::map<std::string, std::string> attrs;
                attrs["rowspan"] = std::to_string(item_length);
                std::string td1 = add_label(subject.name, "td", attrs),
                    td2 = add_label(std::to_string(item.number), "td"),
                    td3 = add_label(item.text, "td");
                tbody += add_label(td1 + td2 + td3, "tr", { {"class", "withsn"} });
            }
            for (int j = 1; j < item_length; j++) {
                item = subject.items[j];
                std::string td1 = add_label(std::to_string(item.number), "td");
                std::string td2 = add_label(item.text, "td");
                tbody += add_label(td1 + td2, "tr", { {"class", "withoutsn"} });
            }
        }
        table += add_label(tbody, "tbody");
        table = add_label(table, "table", { {"border", "1"} });
        writer << table << std::endl;
        std::string create_modi_note = outfile.created_time + " 创建";
        int modi_length = outfile.modified_times.size();
        for (int i = 0; i < modi_length; i++) {
            Modification modi = outfile.modified_times[i];
            create_modi_note += " / " + add_label(modi.modified_time + " 更正", "strong", { {"--bgc", modi.bg_color} });
        }
        create_modi_note = add_label(create_modi_note, "span");
        writer << create_modi_note << std::endl;
        writer << "</body></html>" << std::flush;
    }
    writer.close();
    return 0;
}
