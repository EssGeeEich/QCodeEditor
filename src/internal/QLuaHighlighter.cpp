// QCodeEditor
#include <QLanguage>
#include <QLuaHighlighter>
#include <QSyntaxStyle>

// Qt
#include <QFile>

QLuaHighlighter::QLuaHighlighter(QTextDocument *document)
    : QStyleSyntaxHighlighter(document), m_highlightRules(), m_highlightBlockRules(),
      m_requirePattern(QRegularExpression(R"(require\s*([("'][a-zA-Z0-9*._]+['")]))")),
      m_functionPattern(QRegularExpression(R"(\b([A-Za-z0-9_]+(?:\s+|::))*([A-Za-z0-9_]+)(?=\())")),
      m_defTypePattern(QRegularExpression(R"(\b([A-Za-z0-9_]+)\s+[A-Za-z]{1}[A-Za-z0-9_]+\s*[=])"))
{
    Q_INIT_RESOURCE(qcodeeditor_resources);
    QFile fl(":/languages/lua.xml");

    if (!fl.open(QIODevice::ReadOnly))
    {
        return;
    }

    QLanguage language(&fl);

    if (!language.isLoaded())
    {
        return;
    }

    auto keys = language.keys();
    for (auto &&key : keys)
    {
        auto names = language.names(key);
        for (auto &&name : names)
        {
            m_highlightRules.append({QRegularExpression(QString(R"(\b\s{0,1}%1\s{0,1}\b)").arg(name)), key});
        }
    }

    // Numbers
    m_highlightRules.append({QRegularExpression(R"(\b(0b|0x){0,1}[\d.']+\b)"), "Number"});

    // Strings
    m_highlightRules.append({QRegularExpression(R"(["'][^\n"]*["'])"), "String"});

    // Preprocessor
    m_highlightRules.append({QRegularExpression(R"(#\![a-zA-Z_]+)"), "Preprocessor"});

    // Single line
    m_highlightRules.append({QRegularExpression(R"(--[^\n]*)"), "Comment"});

	for(int charCount = 0; charCount < 16; ++charCount) {
		// Multiline comments
		m_highlightBlockRules.append({
			QRegularExpression(
				QString("--\\[") + QString(charCount,QChar('=')) + "\\["
			),
			QRegularExpression(
				QString("--\\]") + QString(charCount,QChar('=')) + "\\]"
			),
			"Comment"
		});
		
		// Multiline strings
		m_highlightBlockRules.append({
			QRegularExpression(
				QString("\\[") + QString(charCount,QChar('=')) + "\\["
			),
			QRegularExpression(
				QString("\\]") + QString(charCount,QChar('=')) + "\\]"
			),
			"Comment"
		});
	}

    // Comment sequences for toggling support
    m_commentLineSequence = "--";
    m_startCommentBlockSequence = "--[[";
    m_endCommentBlockSequence = "]]";
}

void QLuaHighlighter::highlightBlock(const QString &text)
{
    { // Checking for require
        auto matchIterator = m_requirePattern.globalMatch(text);

        while (matchIterator.hasNext())
        {
            auto match = matchIterator.next();

            setFormat(match.capturedStart(), match.capturedLength(), syntaxStyle()->getFormat("Preprocessor"));

            setFormat(match.capturedStart(1), match.capturedLength(1), syntaxStyle()->getFormat("String"));
        }
    }
    { // Checking for function
        auto matchIterator = m_functionPattern.globalMatch(text);

        while (matchIterator.hasNext())
        {
            auto match = matchIterator.next();

            setFormat(match.capturedStart(), match.capturedLength(), syntaxStyle()->getFormat("Type"));

            setFormat(match.capturedStart(2), match.capturedLength(2), syntaxStyle()->getFormat("Function"));
        }
    }
    { // checking for type
        auto matchIterator = m_defTypePattern.globalMatch(text);

        while (matchIterator.hasNext())
        {
            auto match = matchIterator.next();

            setFormat(match.capturedStart(1), match.capturedLength(1), syntaxStyle()->getFormat("Type"));
        }
    }

    for (auto &rule : m_highlightRules)
    {
        auto matchIterator = rule.pattern.globalMatch(text);

        while (matchIterator.hasNext())
        {
            auto match = matchIterator.next();

            setFormat(match.capturedStart(), match.capturedLength(), syntaxStyle()->getFormat(rule.formatName));
        }
    }

    setCurrentBlockState(0);
    int startIndex = 0;
    int highlightRuleId = previousBlockState();
    if (highlightRuleId < 1 || highlightRuleId > m_highlightBlockRules.size())
    {
        for (int i = 0; i < m_highlightBlockRules.size(); ++i)
        {
            startIndex = text.indexOf(m_highlightBlockRules.at(i).startPattern);
            if (startIndex >= 0)
            {
                highlightRuleId = i + 1;
                break;
            }
        }
    }

    while (startIndex >= 0)
    {
        const auto &blockRules = m_highlightBlockRules.at(highlightRuleId - 1);
        auto match = blockRules.endPattern.match(text, startIndex);

        int endIndex = match.capturedStart();
        int matchLength = 0;

        if (endIndex == -1)
        {
            setCurrentBlockState(highlightRuleId);
            matchLength = text.length() - startIndex;
        }
        else
        {
            matchLength = endIndex - startIndex + match.capturedLength();
        }

        setFormat(startIndex, matchLength, syntaxStyle()->getFormat(blockRules.formatName));
        startIndex = text.indexOf(blockRules.startPattern, startIndex + matchLength);
    }
}

QString QLuaHighlighter::startToggleCommentBlockSequence(const QString &contents) const
{
	if(contents.startsWith("--[")) {
		for(int chars = 3; chars < contents.size(); ++chars) {
			if(contents[chars] == '[') {
				return contents.left(chars+1);
			}
		}
	}
	
	QString testString;
	// Don't try too hard...
	for(int iUsedChars = 0; iUsedChars < 16; ++iUsedChars) {
		testString = "--]";
		testString += QString(iUsedChars, QChar('='));
		testString += "]";
		
		if(contents.indexOf(testString) >= 0)
			continue;
		
		testString = "--[";
		testString += QString(iUsedChars, QChar('='));
		testString += "[";
		return testString;
	}
	
	return "--[[";
}

QString QLuaHighlighter::endToggleCommentBlockSequence(const QString &start) const
{
	QString copy = start;
	copy.replace('[', ']');
	return copy;
}
