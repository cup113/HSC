/**
 * @file hsc.cpp Homework Showing Compiler
 * @version 0.0.2 (Dev)
 * @author Jason M. Li
 * @date 2022.2.16
 */

#include <cstdio>
#include <iostream>
#include <fstream>
#include <stack>
#include <string>
#include <vector>
#include <map>
#define LINE_BUF 4096

enum SP_CH {
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
		short sp_ch;
		std::string tempstr = "";
		int tempint = 0;
		Tag(short _sp_ch) {
			sp_ch == _sp_ch;
		}
};

class Modification {
	public:
		std::string bg_color = "yellow";
		std::string modified_time = "--:--";
		Modification() {}
		Modification(std::string _bg_color, std::string _modified_time) {
			bg_color = _bg_color;
			modified_time = _modified_time;
		}
};

class Subject {
	public:
		std::string name;
		std::vector<std::string> items;
		Subject() {}
		Subject(std::string _name) {
			name = _name;
		}
};

class OutputFile {
	public:
		std::string date = "--------";
		std::string created_time = "--:--";
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
		std::string tempattr = ' ' + attr->first + "=\"" + attr->second + '"';
		result += tempattr;
	}
	result += text;
	result += "</" + tagName + '>';
	return result;
}

int main(int argc, char *argv[]) {
	if (argc < 2) {
		printf("Error 0x0001: No File Input.");
		return 0x0001;
	}
	std::string filename(argv[1]);
	std::ifstream reader;
	std::ofstream writer;
	reader.open(filename);
	writer.open(filename.substr(0, filename.size() - 5) + ".html");
	if (!reader.is_open()) {
		printf("Error 0x0010: Read File Failed.");
		return 0x0010;
	}
	std::stack<Tag> tags;
	tags.push(Tag(SP_CH::ROOT));
	tags.push(Tag(SP_CH::ROOT)); // 根元素，防溢出
	OutputFile outfile;
	int number_counter = 0;
	char linebuffer[LINE_BUF];
	std::string strbuffer;
	int linelength;
	while (reader.getline(linebuffer, LINE_BUF)) {
		linelength = strlen(linebuffer);
		strbuffer.copy(linebuffer, 0, linelength);
		// 监测是否为行级元素
		{
			char first_ch = strbuffer[0];
			if (first_ch == '+') {
				std::string next_five_ch = strbuffer.substr(1, 45);
				if (next_five_ch.compare("date ") == 0) {
					tags.push(Tag(SP_CH::META_DATE));
				}
				if (next_five_ch.compare("crea ") == 0) {
					tags.push(Tag(SP_CH::META_CREATED));
				}
				else if (next_five_ch.compare("modi ") == 0) {
					tags.push(Tag(SP_CH::META_MODIFIED));
				}
				strbuffer = strbuffer.substr(6);
			}
			else if (first_ch == '-') {
				if (strbuffer[1] == ' ') {
					tags.push(Tag(SP_CH::START_ITEM));
					strbuffer = strbuffer.substr(2);
				}
				else if (strbuffer.substr(1, 2).compare("--") == 0) {
					tags.push(Tag(SP_CH::START_NOTE));
					strbuffer = "";
				}
			}
			else if (first_ch == ':') {
				tags.push(Tag(SP_CH::START_SUBJECT));
				strbuffer = strbuffer.substr(1);
			}
		}
		// 行内元素监测
		for (int i=0; i<linelength; i++) {
			unsigned char ch = linebuffer[i];
			int tagsize = tags.size();
			int top_sp_ch = tags.top().sp_ch;
			if (tagsize == 0) {
				printf("ERROR 0x0100: no tags left.");
				return 0x0100;
			}
			if (tagsize == 1) {
				printf("WARNING: The last tag is ROOT tag.");
			}
			// 转义
			if (top_sp_ch == SP_CH::START_ESCAPE) {
				tags.pop();
				if (ch == 'n') tags.top().tempstr += '\n';
				else if (ch == 't') tags.top().tempstr += '\t';
				else tags.top().tempstr += ch;
			}
			else if (ch == '\\' && top_sp_ch != SP_CH::START_EMPHASIZE) {
				tags.push(Tag(SP_CH::START_ESCAPE));
			}
			// 注释
			else if (ch == '<' && i < linelength - 1 && linebuffer[i+1] == '?') {
				tags.push(Tag(SP_CH::START_NOTE_SYMBOL));
				++i;
			}
			else if ((ch == '+' || ch == '-') && top_sp_ch == SP_CH::START_NOTE) {
				tags.top().tempint = 44 - ch;
				tags.push(Tag(SP_CH::START_NOTE_SYMBOL_TEXT));
			}
			else if (ch >= '0' && ch <= '9' && top_sp_ch == SP_CH::START_NOTE_SYMBOL_TEXT) {
				tags.top().tempstr += ch;
			}
			else if (ch == '>' && top_sp_ch== SP_CH::START_NOTE) {
				number_counter += 1;
				tags.pop();
				tags.top().tempstr += add_label(std::to_string(number_counter), "strong");
			}
			else if (ch == '>' && top_sp_ch == SP_CH::START_NOTE_SYMBOL_TEXT) {
				number_counter += 1;
				int note_number = number_counter + tags.top().tempint * atoi(tags.top().tempstr.c_str());
				tags.pop();
				tags.pop();
				tags.top().tempstr += add_label(std::to_string(note_number), "strong");
			}
			// 强调
			else if (ch == '*' && top_sp_ch != SP_CH::START_EMPHASIZE_TEXT) {
				if (i < linelength - 1 && linebuffer[i+1] == ':') {
					tags.push(Tag(SP_CH::START_EMPHASIZE));
					++i;
				}
				else {
					tags.push(Tag(SP_CH::START_EMPHASIZE));
					tags.push(Tag(SP_CH::START_EMPHASIZE_TEXT));
				}
			}
			else if (ch >= '0' && ch <= '9' && top_sp_ch == SP_CH::START_EMPHASIZE) {
				tags.top().tempstr += ch;
			}
			else if (ch == ':' && top_sp_ch == SP_CH::START_EMPHASIZE) {
				int version = atoi(tags.top().tempstr.c_str());
				tags.top().tempint = version;
				tags.top().tempstr.clear();
				tags.push(SP_CH::START_EMPHASIZE_TEXT);
			}
			else if (ch == '*' && top_sp_ch == SP_CH::START_EMPHASIZE_TEXT) {
				std::string text = tags.top().tempstr;
				tags.pop();
				int version = tags.top().tempint;
				tags.pop();
				if (version == 0 || outfile.modified_times.size() < version) {
					tags.top().tempstr += add_label(text, "strong");
				}
				else {
					std::map<std::string, std::string> attrs;
					attrs["style"] = "--color: " + outfile.modified_times[version - 1].bg_color;
					tags.top().tempstr += add_label(text, "strong", attrs);
				}
			}
			// 超链接
			else if (ch == '[') {
				tags.push(Tag(SP_CH::START_HYPERLINK));
			}
			else if (ch == ']' && top_sp_ch == SP_CH::START_HYPERLINK) {
				tags.top().tempint = 1;
			}
			else if (ch == '(' && top_sp_ch == SP_CH::START_HYPERLINK && tags.top().tempint == 1) {
				tags.push(Tag(SP_CH::START_HYPERLINK_TEXT));
			}
			else if (ch == ')' && top_sp_ch == SP_CH::START_HYPERLINK && tags.top().tempint == 1) {
				tags.pop();
				std::string url = tags.top().tempstr;
				tags.pop();
				std::map<std::string, std::string> attrs;
				attrs["href"] = url;
				tags.top().tempstr += add_label(url, "a", attrs);
			}
			else if (ch == ')' && top_sp_ch == SP_CH::START_EMPHASIZE_TEXT) {
				std::string text = tags.top().tempstr;
				tags.pop();
				std::string url = tags.top().tempstr;
				tags.pop();
				std::map<std::string, std::string> attrs;
				attrs["href"] = url;
				tags.top().tempstr += add_label(text, "a", attrs);
			}
			// 空格分割的数据声明
			else if (ch == ' ') {
				if (top_sp_ch == SP_CH::META_MODIFIED) {
					tags.push(Tag(SP_CH::META_MODIFIED_COLOR));
				}
			}
		}
		// 监测堆栈/最近的行级项
		{
			int top_sp_ch = tags.top().sp_ch;
			if (top_sp_ch == SP_CH::META_DATE) {
				outfile.date = tags.top().tempstr;
				tags.pop();
			}
			else if (top_sp_ch == SP_CH::META_CREATED) {
				outfile.created_time = tags.top().tempstr;
				tags.pop();
			}
			else if (top_sp_ch == SP_CH::META_MODIFIED) {
				outfile.modified_times.push_back(Modification("yellow", tags.top().tempstr));
				tags.pop();
			}
			else if (top_sp_ch == SP_CH::META_MODIFIED_COLOR) {
				std::string color = tags.top().tempstr;
				tags.pop();
				outfile.modified_times.push_back(Modification(color, tags.top().tempstr));
				tags.pop();
			}
			else if (top_sp_ch == SP_CH::START_NOTE) {
				number_counter = 0; // 复位
				outfile.subjects.push_back(Subject("备注"));
			}
			else if (top_sp_ch == SP_CH::START_SUBJECT) {
				outfile.subjects.push_back(Subject(tags.top().tempstr));
				tags.pop();
			}
			else if (top_sp_ch == SP_CH::START_ITEM) {
				outfile.subjects.back().items.push_back(tags.top().tempstr);
				tags.pop();
			}
		}
	}
	reader.close();
	return 0;
}