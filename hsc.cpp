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
#include <sys/time.h>
#define LINE_BUF 4096

enum SP_CH: short {
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
		std::string tempstr = "";
		int tempint = 0;
		Tag(short _sp_ch) {
			sp_ch = _sp_ch;
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

class Item {
	public:
		std::string text;
		int number;
		Item() {}
		Item(std::string _text) {
			text = _text;
		}
		Item(std::string _text, int _number) {
			text = _text;
			number = _number;
		}
};

class Subject {
	public:
		std::string name;
		std::vector<Item> items;
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
	result += ">";
	result += text;
	result += "</" + tagName + '>';
	return result;
}

int main(int argc, char *argv[]) {
	struct timeval tv_begin, tv_end, tv_program_start;
	gettimeofday(&tv_program_start, NULL);
	if (argc < 2) {
		printf("Error 0x0001: No File Input.");
		return 0x0001;
	}
	std::string filename(argv[1]);
	std::ifstream reader;
	std::ofstream writer;
	reader.open(filename);
	if (!reader.is_open()) {
		printf("Error 0x0010: Read File Failed.");
		return 0x0010;
	}
	std::stack<Tag> tags;
	tags.push(Tag(SP_CH::ROOT));
	tags.push(Tag(SP_CH::ROOT)); // 根元素，防溢出
	OutputFile outfile;
	int note_counter = 0,
	number_counter = 0,
	line_counter = 0;
	char linebuffer[LINE_BUF];
	std::string strbuffer;
	int linelength;
	std::map<std::string, std::string> attr_withsn,
	attr_withoutsn,
	attr_border,
	attr_note,
	attr_note_part;
	attr_withsn["class"] = "withsn";
	attr_withoutsn["class"] = "withoutsn";
	attr_border["border"] = "1";
	attr_note["class"] = "note";
	attr_note_part["class"] = "note-part";
	gettimeofday(&tv_begin, NULL);
	while (reader.getline(linebuffer, LINE_BUF)) {
		linelength = strlen(linebuffer);
		strbuffer = linebuffer;
		// 监测是否为行级元素
		{
			char first_ch = strbuffer[0];
			if (first_ch == '+') {
				std::string next_five_ch = strbuffer.substr(1, 5);
				if (next_five_ch.compare("date ") == 0) {
					tags.push(Tag(SP_CH::META_DATE));
				}
				else if (next_five_ch.compare("crea ") == 0) {
					tags.push(Tag(SP_CH::META_CREATED));
				}
				else if (next_five_ch.compare("modi ") == 0) {
					tags.push(Tag(SP_CH::META_MODIFIED));
				}
				strbuffer = strbuffer.substr(6);
				linelength -= 6;
			}
			else if (first_ch == '-') {
				if (strbuffer[1] == ' ') {
					tags.push(Tag(SP_CH::START_ITEM));
					strbuffer = strbuffer.substr(2);
					linelength -= 2;
				}
				else if (strbuffer.substr(1, 2).compare("--") == 0) {
					tags.push(Tag(SP_CH::START_NOTE));
					strbuffer = "";
					linelength = 0;
				}
			}
			else if (first_ch == '#') {
				tags.push(Tag(SP_CH::START_SUBJECT));
				int start = 1;
				if (strbuffer[1] == ' ') start = 2;
				strbuffer = strbuffer.substr(start);
				linelength -= start;
			}
		}
		// 行内元素监测
		for (int i=0; i<linelength; i++) {
			unsigned char ch = strbuffer[i];
			int tagsize = tags.size();
			int top_sp_ch = tags.top().sp_ch;
			if (tagsize == 0) {
				printf("ERROR 0x0020: no tags left.");
				return 0x0020;
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
			else if (ch == '<' && i < linelength - 1 && strbuffer[i+1] == '?') {
				tags.push(Tag(SP_CH::START_NOTE_SYMBOL));
				++i;
			}
			else if ((ch == '+' || ch == '-') && top_sp_ch == SP_CH::START_NOTE_SYMBOL) {
				tags.top().tempint = 44 - ch;
				tags.push(Tag(SP_CH::START_NOTE_SYMBOL_TEXT));
			}
			else if (ch >= '0' && ch <= '9' && top_sp_ch == SP_CH::START_NOTE_SYMBOL_TEXT) {
				tags.top().tempstr += ch;
			}
			else if (ch == '>' && top_sp_ch== SP_CH::START_NOTE_SYMBOL) {
				note_counter += 1;
				tags.pop();
				tags.top().tempstr += add_label("[注" + std::to_string(note_counter) + ']', "span", attr_note);
			}
			else if (ch == '>' && top_sp_ch == SP_CH::START_NOTE_SYMBOL_TEXT) {
				note_counter += 1;
				int note_number = atoi(tags.top().tempstr.c_str());
				tags.pop();
				note_number *= tags.top().tempint;
				note_number += note_counter;
				tags.pop();
				tags.top().tempstr += add_label("[注" + std::to_string(note_number) + ']', "span", attr_note);
				note_counter -= 1;
			}
			// 强调
			else if (ch == '*' && top_sp_ch != SP_CH::START_EMPHASIZE_TEXT) {
				if (i < linelength - 1 && strbuffer[i+1] == ':') {
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
				tags.push(Tag(SP_CH::START_EMPHASIZE_TEXT));
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
					attrs["style"] = "--bgc: " + outfile.modified_times[version - 1].bg_color;
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
				std::string url = tags.top().tempstr;
				tags.pop();
				std::map<std::string, std::string> attrs;
				attrs["href"] = url;
				tags.top().tempstr += add_label(url, "a", attrs);
			}
			else if (ch == ')' && top_sp_ch == SP_CH::START_HYPERLINK_TEXT) {
				std::string text = tags.top().tempstr;
				tags.pop();
				std::string url = tags.top().tempstr;
				tags.pop();
				std::map<std::string, std::string> attrs;
				attrs["href"] = url;
				tags.top().tempstr += add_label(text, "a", attrs);
			}
			// 空格分割的数据声明
			else if (ch == ' ' && top_sp_ch == SP_CH::META_MODIFIED) {
				tags.push(Tag(SP_CH::META_MODIFIED_COLOR));
			}
			else {
				tags.top().tempstr += ch;
			}
		}
		while (tags.top().sp_ch >= SP_CH::START_HYPERLINK && tags.top().sp_ch <= SP_CH::START_ESCAPE) {
			// 过滤堆栈未使用内容
			std::string tempstr = tags.top().tempstr;
			int tempint = tags.top().tempint;
			short top_sp_ch = tags.top().sp_ch;
			tags.pop();
			switch (top_sp_ch) {
				case (SP_CH::START_EMPHASIZE):
					tags.top().tempstr += "*:";
					break;
				case (SP_CH::START_EMPHASIZE_TEXT):
					if (tags.top().tempint != 0) {
						int tempver = tags.top().tempint;
						tags.pop();
						tags.top().tempstr += "*:";
						tags.top().tempstr += std::to_string(tempver);
						tags.top().tempstr += ':';
					}
					else {
						tags.pop();
						tags.top().tempstr += '*';
					}
					break;
				case (SP_CH::START_ESCAPE):
					tags.top().tempstr += '\\';
					break;
				case (SP_CH::START_HYPERLINK):
					tags.top().tempstr += '[';
					if (tempint == 1) {
						tags.top().tempstr += tempstr;
						tags.top().tempstr += ']';
						tempstr.clear();
					}
					break;
				case (SP_CH::START_HYPERLINK_TEXT):
					{
						Tag temptag = tags.top();
						tags.pop();
						tags.top().tempstr += '[';
						tags.top().tempstr += temptag.tempstr;
						tags.top().tempstr += "](";
						tempstr.clear();
					}
					break;
				case (SP_CH::START_NOTE_SYMBOL):
					tags.top().tempstr += "<?";
					break;
				case (SP_CH::START_NOTE_SYMBOL_TEXT):
					tags.pop();
					tags.top().tempstr += "<?";
					tags.top().tempstr += (char)(44 + tempint);
					break;
			}
			tags.top().tempstr += tempstr;
		}
		{
			// 监测堆栈/最近的行级项
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
				outfile.subjects.push_back(Subject(add_label("备注", "span", attr_note_part)));
			}
			else if (top_sp_ch == SP_CH::START_SUBJECT) {
				outfile.subjects.push_back(Subject(tags.top().tempstr));
				tags.pop();
			}
			else if (top_sp_ch == SP_CH::START_ITEM) {
				number_counter += 1;
				outfile.subjects.back().items.push_back(Item(tags.top().tempstr, number_counter));
				tags.pop();
			}
		}
		line_counter += 1;
		if (line_counter % 10 == 0) printf("Read line %d successfully\r", line_counter);
	}
	reader.close();
	{
		printf("Read line %d successfully\n", line_counter);
		gettimeofday(&tv_end, NULL);
		unsigned timediff = (tv_end.tv_sec - tv_begin.tv_sec) * 1000000 + (tv_end.tv_usec - tv_begin.tv_usec);
		printf("Read successfully in %d.%03d ms.\n", timediff / 1000, timediff % 1000);
		tv_begin = tv_end;
	}
	writer.open(filename.substr(0, filename.size() - 4) + ".html");
	if (!writer.is_open()) {
		printf("Error 0x0011: Write File Failed.");
		return 0x0011;
	}
	{
		// 写文件
		writer << "<!DOCTYPE html>\n<html lang='zh-cn'>\n<head>\n<meta name='charset' content='utf-8'>\n<meta name='generator' content='Visual Studio Code'>\n<meta name='author' content='Jason Li'>\n<meta name='robots' content='noindex'>\n<title>作业</title>\n<link rel='shortcut icon' href='img/cube.ico' type='image/x-icon'>\n<link rel='stylesheet' href='style.css'>\n</head>\n<body>\n";
		std::string table = "<thead>\n<tr>\n<td colspan='3'>" + outfile.date + "作业</td>\n</tr>\n<tr>\n<td>科目</td>\n<td>序号</td>\n<td>项目</td>\n</tr>\n</thead>",
		tbody = "";
		int subject_length = outfile.subjects.size();
		for (int i=0; i<subject_length; i++) {
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
				tbody += add_label(td1 + td2 + td3, "tr", attr_withsn);
			}
			for (int j=1; j<item_length; j++) {
				item = subject.items[j];
				std::string td1 = add_label(std::to_string(item.number), "td");
				std::string td2 = add_label(item.text, "td");
				tbody += add_label(td1 + td2, "tr", attr_withoutsn);
			}
		}
		table += add_label(tbody, "tbody");
		table = add_label(table, "table", attr_border);
		table += '\n';
		writer << table;
		std::string create_modi_note;
		create_modi_note += outfile.created_time + " 发布";
		int modi_length = outfile.modified_times.size();
		for (int i=0; i<modi_length; i++) {
			Modification modification = outfile.modified_times[i];
			std::map<std::string, std::string> attrs;
			attrs["style"] = "--bgc: " + modification.bg_color;
			create_modi_note += " / " + add_label(modification.modified_time + " 更正", "strong", attrs);
		}
		create_modi_note = add_label(create_modi_note, "span");
		create_modi_note += '\n';
		writer << create_modi_note;
		writer << "</body></html>\n";
	}
	writer.close();
	{
		gettimeofday(&tv_end, NULL);
		unsigned timediff = (tv_end.tv_sec - tv_begin.tv_sec) * 1000000 + (tv_end.tv_usec - tv_begin.tv_usec),
		timediff2 = (tv_end.tv_sec - tv_program_start.tv_sec) * 1000000 + (tv_end.tv_usec - tv_program_start.tv_usec);
		printf("Write successfully in %d.%03d ms.\n", timediff / 1000, timediff % 1000);
		printf("Program has run successfully in %d.%03dms\n", timediff2 / 1000, timediff2 % 1000);
	}
	return 0;
}
