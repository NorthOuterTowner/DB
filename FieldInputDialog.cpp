#include "FieldInputDialog.h"
#include <QMessageBox>
#include <QFile>
#include <QTextStream>

FieldInputDialog::FieldInputDialog(QWidget *parent) : QDialog(parent) {
    setWindowTitle(tr("增加字段"));
    setMinimumSize(500, 800);

    mainLayout = new QVBoxLayout(this);

    // 标签页控件
    tabWidget = new QTabWidget(this);
    tabWidget->setTabsClosable(false);
    mainLayout->addWidget(tabWidget);

    // 按钮布局
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    addButton = new QPushButton(tr("+ 添加字段"), this);
    removeButton = new QPushButton(tr("- 删除字段"), this);
    removeButton->setEnabled(false); // 初始禁用

    buttonLayout->addWidget(addButton);
    buttonLayout->addWidget(removeButton);
    mainLayout->addLayout(buttonLayout);

    // 按钮组
    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, this);
    mainLayout->addWidget(buttonBox);

    // 添加初始字段页
    addFieldTab();

    // 连接信号槽
    connect(addButton, &QPushButton::clicked, this, &FieldInputDialog::addFieldTab);
    connect(removeButton, &QPushButton::clicked, this, &FieldInputDialog::removeCurrentTab);
    connect(tabWidget, &QTabWidget::currentChanged, this, &FieldInputDialog::tabChanged);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

FieldInputDialog::FieldInputDialog(const QString& tableName, const QString& fieldName, QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(tr("修改字段信息 - %1").arg(fieldName));
    setMinimumSize(200, 400);

    mainLayout = new QVBoxLayout(this);

    // 标签页控件
    tabWidget = new QTabWidget(this);
    tabWidget->setTabsClosable(false);
    mainLayout->addWidget(tabWidget);

    // 按钮布局
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    addButton = new QPushButton(tr("+ 添加字段"), this);
    removeButton = new QPushButton(tr("- 删除字段"), this);
    removeButton->setEnabled(false); // 初始禁用

    buttonLayout->addWidget(addButton);
    buttonLayout->addWidget(removeButton);
    mainLayout->addLayout(buttonLayout);

    // 新增删除按钮
    deleteButton = new QPushButton(tr("删除字段"), this);
    buttonLayout->addWidget(deleteButton);

    // 按钮组
    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, this);
    mainLayout->addWidget(buttonBox);

    // 添加初始字段页
    addFieldTab();

    // 加载字段数据
    loadFieldData(tableName, fieldName);

    // 隐藏添加/删除按钮
    addButton->hide();
    removeButton->hide();

    // 禁用标签页切换功能（因为只有一个标签页）
    tabWidget->tabBar()->hide();

    // 连接信号槽
    connect(tabWidget, &QTabWidget::currentChanged, this, &FieldInputDialog::tabChanged);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

QList<FieldData> FieldInputDialog::getFields() const
{
    QList<FieldData> fields;
    for (int i = 0; i < tabWidget->count(); ++i) {
        QWidget *tab = tabWidget->widget(i);

        // 获取各个控件
        QLineEdit *nameEdit = tab->findChild<QLineEdit*>("nameEdit");
        QComboBox *typeCombo = tab->findChild<QComboBox*>("typeCombo");
        QLineEdit *paramEdit = tab->findChild<QLineEdit*>("paramEdit");
        QLineEdit *defaultEdit = tab->findChild<QLineEdit*>("defaultEdit");
        QLineEdit *constraintEdit = tab->findChild<QLineEdit*>("constraintEdit");

        if (nameEdit &&!nameEdit->text().isEmpty()) {
            FieldData field;
            field.name = nameEdit->text();
            field.type = typeCombo->currentText();
            field.param = paramEdit->text().toInt();
            field.defaultValue = defaultEdit->text();
            field.constraint = constraintEdit->text();
            fields.append(field);
        }
    }
    return fields;
}

void FieldInputDialog::addFieldTab() {
    QWidget *tab = new QWidget(this);
    QVBoxLayout *tabLayout = new QVBoxLayout(tab);

    // 字段名
    QHBoxLayout *nameLayout = new QHBoxLayout();
    QLabel *nameLabel = new QLabel(tr("字段名:"), tab);
    QLineEdit *nameEdit = new QLineEdit(tab);
    nameEdit->setObjectName("nameEdit");
    nameLayout->addWidget(nameLabel);
    nameLayout->addWidget(nameEdit);
    tabLayout->addLayout(nameLayout);

    // 字段类型
    QHBoxLayout *typeLayout = new QHBoxLayout();
    QLabel *typeLabel = new QLabel(tr("字段类型:"), tab);
    QComboBox *typeCombo = new QComboBox(tab);
    typeCombo->setObjectName("typeCombo");
    typeCombo->addItems({"INT", "VARCHAR", "TEXT", "DATETIME", "BOOLEAN"});
    typeLayout->addWidget(typeLabel);
    typeLayout->addWidget(typeCombo);
    tabLayout->addLayout(typeLayout);

    // 字段参数
    QHBoxLayout *paramLayout = new QHBoxLayout();
    QLabel *paramLabel = new QLabel(tr("字段参数:"), tab);
    QLineEdit *paramEdit = new QLineEdit(tab);
    paramEdit->setObjectName("paramEdit");
    paramEdit->setPlaceholderText(tr("例如:VARCHAR长度"));
    paramLayout->addWidget(paramLabel);
    paramLayout->addWidget(paramEdit);
    tabLayout->addLayout(paramLayout);

    // 默认值
    QHBoxLayout *defaultLayout = new QHBoxLayout();
    QLabel *defaultLabel = new QLabel(tr("默认值:"), tab);
    QLineEdit *defaultEdit = new QLineEdit(tab);
    defaultEdit->setObjectName("defaultEdit");
    defaultLayout->addWidget(defaultLabel);
    defaultLayout->addWidget(defaultEdit);
    tabLayout->addLayout(defaultLayout);

    // 约束条件
    QHBoxLayout *constraintLayout = new QHBoxLayout();
    QLabel *constraintLabel = new QLabel(tr("约束条件:"), tab);
    QLineEdit *constraintEdit = new QLineEdit(tab);
    constraintEdit->setObjectName("constraintEdit");
    constraintEdit->setPlaceholderText(tr("例如:NOT NULL"));
    constraintLayout->addWidget(constraintLabel);
    constraintLayout->addWidget(constraintEdit);
    tabLayout->addLayout(constraintLayout);

    // 添加间距
    tabLayout->addStretch();

    // 添加标签页
    int index = tabWidget->addTab(tab, tr("字段 %1").arg(tabWidget->count() + 1));
    tabWidget->setCurrentIndex(index);

    // 根据类型设置默认值占位符
    connect(typeCombo, &QComboBox::currentTextChanged, [=](const QString &type) {
        if (type == "INT" || type == "BOOLEAN") {
            defaultEdit->setPlaceholderText(tr("数值"));
        } else if (type == "VARCHAR" || type == "TEXT") {
            defaultEdit->setPlaceholderText(tr("字符串值"));
        } else if (type == "DATETIME") {
            defaultEdit->setPlaceholderText(tr("YYYY-MM-DD HH:MM:SS"));
        }
    });

    // 字段名变化时更新标签页标题
    connect(nameEdit, &QLineEdit::textChanged, [=](const QString &text) {
        updateTabText(index, text);
    });

    // 启用删除按钮
    removeButton->setEnabled(true);
}

void FieldInputDialog::removeCurrentTab() {
    int currentIndex = tabWidget->currentIndex();
    if (currentIndex >= 0 && tabWidget->count() > 1) {
        tabWidget->removeTab(currentIndex);

        // 更新标签页标题
        updateAllTabTexts();

        // 如果没有标签页了，添加一个
        if (tabWidget->count() == 0) {
            addFieldTab();
        }
    } else if (tabWidget->count() == 1) {
        QMessageBox::information(this, tr("提示"), tr("至少保留一个字段"));
    }
}

void FieldInputDialog::tabChanged(int index) {
    // 启用或禁用删除按钮
    removeButton->setEnabled(tabWidget->count() > 1);
}

void FieldInputDialog::updateTabText(int index, const QString &fieldName) {
    if (index >= 0 && index < tabWidget->count()) {
        if (!fieldName.isEmpty()) {
            tabWidget->setTabText(index, fieldName);
        } else {
            tabWidget->setTabText(index, tr("字段 %1").arg(index + 1));
        }
    }
}

void FieldInputDialog::updateAllTabTexts() {
    for (int i = 0; i < tabWidget->count(); ++i) {
        QWidget *tab = tabWidget->widget(i);
        QLineEdit *nameEdit = tab->findChild<QLineEdit*>("nameEdit");

        if (nameEdit) {
            updateTabText(i, nameEdit->text());
        } else {
            tabWidget->setTabText(i, tr("字段 %1").arg(i + 1));
        }
    }
}

void FieldInputDialog::setName(const QString& name) {
    for (int i = 0; i < tabWidget->count(); ++i) {
        QWidget *tab = tabWidget->widget(i);
        QLineEdit *nameEdit = tab->findChild<QLineEdit*>("nameEdit");
        if (nameEdit) {
            nameEdit->setText(name);
            updateTabText(i, name);
            break;
        }
    }
}

void FieldInputDialog::setFieldDataToCurrentTab(const FieldData& data)
{
    QWidget *tab = tabWidget->currentWidget();
    if (!tab) return;

    // 获取各个控件
    QLineEdit *nameEdit = tab->findChild<QLineEdit*>("nameEdit");
    QComboBox *typeCombo = tab->findChild<QComboBox*>("typeCombo");
    QLineEdit *paramEdit = tab->findChild<QLineEdit*>("paramEdit");
    QLineEdit *defaultEdit = tab->findChild<QLineEdit*>("defaultEdit");
    QLineEdit *constraintEdit = tab->findChild<QLineEdit*>("constraintEdit");

    if (nameEdit) nameEdit->setText(data.name);
    if (typeCombo) typeCombo->setCurrentText(data.type);
    if (paramEdit) paramEdit->setText(QString::number(data.param));
    if (defaultEdit) defaultEdit->setText(data.defaultValue);
    if (constraintEdit) constraintEdit->setText(data.constraint);
}

bool FieldInputDialog::loadFieldData(const QString& tableName, const QString& fieldName)
{
    auto fieldData = getFieldDataFromDefinition(tableName, fieldName);
    if (fieldData.has_value()) {
        setFieldDataToCurrentTab(fieldData.value());
        return true;
    }
    return false;
}

std::optional<FieldData> FieldInputDialog::getFieldDataFromDefinition(const QString& tableName, const QString& fieldName)
{
    std::string tableDefFilePath = "../../res/" + tableName.toStdString() + ".tdf.txt";
    QFile tableDefFile(QString::fromStdString(tableDefFilePath));

    if (tableDefFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&tableDefFile);
        QString line;

        while ((line = in.readLine()) != "") {
            QStringList parts = line.split(' ');
            if (parts.size() >= 8 && parts[2] == fieldName) {
                FieldData data;
                data.name = fieldName;
                data.type = parts[4];
                data.param = parts[5].toInt();
                data.constraint = parts[7];
                return data;
            }
        }
        tableDefFile.close();
    }

    return std::nullopt;
}
