# HSC 的使用说明

HSC - Homework Showing Compiler : hsf (Homework Showing File) 文件编译器。

## hsf 文件格式说明

### 行级格式

> 行级格式需要单独占满一行。

- **+{name} {content}**
  声明元数据
  - **+date {yyyymmdd}**
    声明作业日期
  - **+modi {hh:mm} \[color=yellow]**
    声明一次修改的时间及背景高亮颜色
  - **\# {学科}**
    表示学科的开始
- **\---**
  表示备注的开始
- **{text}**
  表示一项作业/备注

### 行内格式

> 行内格式可以互相嵌套，包含于行级格式中。

- \[{URL}]\({text})
  表示特定文本的超链接(a标签)
- \[{URL}]\()
  表示超链接文本直接显示URL(a标签)
- \*{text}*
  表示强调(strong标签)
- \*:{version}:{text}*
  表示强调并对应修改版本号
- \<?>
  表示注释(顺序编号递增)
- \<?{+/-}{x}>
  表示注释(顺序编号不参与递增，按照相对位置推进)
- 转义
  \\{char}：转义为char的原始字符