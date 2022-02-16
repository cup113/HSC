/**
 * @file hsc.cpp Homework Showing Compiler
 * @version 0.0.1 (Dev)
 * @author Jason M. Li
 * @date 2022.2.16
 */

#include <cstdio>
#include <iostream>
#include <fstream>
#include <stack>
#include <string>
#include <vector>
#define LINE_BUF 4096

enum SP_CH {
	META_DATE = 0, // 日期声明开始
	META_CREATED, // 创建声明开始
	META_MODIFIED, // 更正声明开始
	START_SUBJECT, // 学科声明开始
	START_NOTE, // 注释部分开始
	START_ITEM, // 创建项目开始
	START_HYPERLINK, // 超链接开始
	START_HYPERLINK_TEXT, // 超链接文本开始(若指定文本)
	START_EMPHASIZE, // 突出开始
	START_EMPHASIZE_TEXT, // 突出文本开始(若指定版本)
	START_NOTE_SYMBOL, // 注释提示开始
	START_NOTE_SYMBOL_TEXT, // 注释文本开始(若指定提前或延后)
	START_ESCAPE // 转义开始
};

std::pair<short, std::pair<int, std::string> > to_pair(short typenum, int a1, std::string a2) {
	std::pair<short, std::pair<int, std::string> > result;
	result.first = typenum;
	result.second.first = a1;
	result.second.second = a2;
}

class Modification {
	public:
		int version = 1;
		std::string bg_color = "yellow";
		std::string modified_time = "--:--";
		Modification() {}
		Modification(int _version, std::string _bg_color, std::string _modified_time) {
			version = _version;
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
	std::stack<std::pair<short, std::pair<int, std::string> > > tags;
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
				std::string next_four_ch = strbuffer.substr(1, 4);
				if (next_four_ch.compare("crea") == 0) {
				}
			}
		}
	}
	reader.close();
	return 0;
}