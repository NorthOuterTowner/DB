#include "highlighttextedit.h"
#include <QTextDocument>
#include <QTextCursor>
#include <QRegularExpression>
#include <QRegularExpressionMatchIterator>
#include <QDebug>
#include <QKeyEvent>
HighLightTextEdit::HighLightTextEdit(QWidget *parent) : QTextEdit(parent)
{
    sqlKeywords << "SELECT" << "FROM" << "WHERE" << "INSERT"
                << "INTO" << "VALUES" << "UPDATE" << "SET"
                << "DELETE" << "CREATE" << "TABLE" << "ALTER"
                << "DROP" << "INDEX" << "VIEW" << "JOIN"
                << "INNER" << "OUTER" << "LEFT" << "RIGHT"
                << "DATABASE" << "USER" << "ON"
                << "GROUP" << "BY" << "ORDER" << "HAVING" << "DISTINCT";

    initHighlightRules();

    QFont font("Consolas", 10);
    font.setStyleHint(QFont::Monospace);
    setFont(font);

    connect(this, &QTextEdit::textChanged, this, &HighLightTextEdit::highlightAll);
}

void HighLightTextEdit::initHighlightRules()
{
    highlightingRules.clear();
    QTextCharFormat keywordFormat;
    keywordFormat.setForeground(Qt::blue);
    QFont keywordFont("Consolas", 10, QFont::Bold);
    keywordFormat.setFont(keywordFont);

    for (const QString &keyword : sqlKeywords) {
        HighlightingRule rule;
        QString pattern = QString("\\b%1\\b").arg(keyword);
        rule.pattern = QRegularExpression(pattern, QRegularExpression::CaseInsensitiveOption);
        rule.format = keywordFormat;
        highlightingRules.append(rule);
    }

    QTextCharFormat stringFormat;
    stringFormat.setForeground(Qt::darkGreen);
    HighlightingRule stringRule;
    stringRule.pattern = QRegularExpression("(\"[^\"]*\")|('[^']*')");
    stringRule.format = stringFormat;
    highlightingRules.append(stringRule);

    QTextCharFormat numberFormat;
    numberFormat.setForeground(Qt::darkMagenta);
    HighlightingRule numberRule;
    numberRule.pattern = QRegularExpression("\\b\\d+\\b");
    numberRule.format = numberFormat;
    highlightingRules.append(numberRule);

    QTextCharFormat commentFormat;
    commentFormat.setForeground(Qt::gray);
    commentFormat.setFontItalic(true);

    HighlightingRule singleLineCommentRule;
    singleLineCommentRule.pattern = QRegularExpression("--[^\n]*");
    singleLineCommentRule.format = commentFormat;
    highlightingRules.append(singleLineCommentRule);

    HighlightingRule multiLineCommentRule;
    multiLineCommentRule.pattern = QRegularExpression("/\\*.*?\\*/", QRegularExpression::DotMatchesEverythingOption);
    multiLineCommentRule.format = commentFormat;
    highlightingRules.append(multiLineCommentRule);
}

void HighLightTextEdit::highlightAll()
{
    QTextDocument *doc = document();
    QTextCursor cursor(doc);
    uppercaseKeywords(cursor);
    QList<QTextEdit::ExtraSelection> extraSelections;

    for (const HighlightingRule &rule : highlightingRules) {
        QRegularExpressionMatchIterator matchIterator = rule.pattern.globalMatch(doc->toPlainText());
        while (matchIterator.hasNext()) {
            QRegularExpressionMatch match = matchIterator.next();

            QTextEdit::ExtraSelection selection;
            selection.cursor = cursor;
            selection.cursor.setPosition(match.capturedStart());
            selection.cursor.setPosition(match.capturedEnd(), QTextCursor::KeepAnchor);
            selection.format = rule.format;
            extraSelections.append(selection);
        }
    }

    setExtraSelections(extraSelections);
}

void HighLightTextEdit::uppercaseKeywords(QTextCursor &cursor)
{
    cursor.movePosition(QTextCursor::Start);
    QString text = cursor.document()->toPlainText();
    QRegularExpression wordRegex("\\b\\w+\\b");
    QRegularExpressionMatchIterator it = wordRegex.globalMatch(text);

    QMap<int, QString> replacements;

    while (it.hasNext()) {
        QRegularExpressionMatch match = it.next();
        QString word = match.captured();

        if (isKeyword(word) && word != word.toUpper()) {
            replacements[match.capturedStart()] = word.toUpper();
        }
    }

    QMapIterator<int, QString> i(replacements);
    i.toBack();
    while (i.hasPrevious()) {
        i.previous();
        cursor.setPosition(i.key());
        cursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, i.value().length());
        cursor.insertText(i.value());
    }
}

bool HighLightTextEdit::isKeyword(const QString &word) const
{
    return sqlKeywords.contains(word, Qt::CaseInsensitive);
}

void HighLightTextEdit::keyPressEvent(QKeyEvent *e)
{
    QTextEdit::keyPressEvent(e);

    if (e->text() == " " || e->text() == "\n") {
        QTextCursor cursor = textCursor();
        cursor.movePosition(QTextCursor::Left, QTextCursor::MoveAnchor, 1);
        highlightAll();
    }
}
