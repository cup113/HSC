# .hsf格式说明

## 行级格式

### +{Name} {content}

声明元数据

### +date {yyyymmdd}

声明作业日期

### +crea {hh:mm}

声明创建时间

### +modi {hh:mm} {version} {color}

声明更正时间 (默认version=1, color=yellow)

### :{学科}

表示学科的开始。

### ---

表示备注的开始

### - {text}

表示一项作业/备注

## 行内格式

\[{URL}]{text})
            表示特定文本的超链接(a标签)

\[{URL}])   表示超链接文本直接显示URL(a标签)

\*{text}*   表示强调(strong标签)

\*{version}-{text}-*
            表示强调(自定义版本)

<?>         表示注释(strong标签)

<?{+/-}{x}> 表示注释，但序号向前/后推进x个

## 转义

\\+任一字符：转义为下一字符