#pragma once

// QCodeEditor
#include <QHighlightRule>
#include <QStyleSyntaxHighlighter> // Required for inheritance

// Qt
#include <QVector>
#include <QRegularExpression>

class QString;
class QTextDocument;

/**
 * @brief Class, that describes JSON code
 * highlighter.
 */
class QJSONHighlighter : public QStyleSyntaxHighlighter
{
    Q_OBJECT
  public:
    /**
     * @brief Constructor.
     * @param document Pointer to document.
     */
    explicit QJSONHighlighter(QTextDocument *document = nullptr);

  protected:
    void highlightBlock(const QString &text) override;

  private:
    QVector<QHighlightRule> m_highlightRules;
    QRegularExpression m_keyRegex;
};
