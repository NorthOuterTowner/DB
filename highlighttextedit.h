#ifndef HIGHLIGHTTEXTEDIT_H
#define HIGHLIGHTTEXTEDIT_H

#include <QTextEdit>
#include <QRegularExpression>

class HighLightTextEdit : public QTextEdit
{
    Q_OBJECT

public:
    explicit HighLightTextEdit(QWidget *parent = nullptr);

protected:
    void keyPressEvent(QKeyEvent *e) override;
    void uppercaseCurrentWord();

private:
    struct HighlightingRule {
        QRegularExpression pattern;
        QTextCharFormat format;
    };

    QVector<HighlightingRule> highlightingRules;
    QStringList sqlKeywords;

    void initHighlightRules();
    void highlightAll();
    void uppercaseKeywords(QTextCursor &cursor);
    bool isKeyword(const QString &word) const;
};

#endif // HIGHLIGHTTEXTEDIT_H
