// QCodeEditor
#include <QLanguage>
#include <QPythonCompleter>

// Qt
#include <QFile>
#include <QStringListModel>

QPythonCompleter::QPythonCompleter(QObject *parent) : QCodeEditorCompleter(parent)
{
    // Setting up Python types
    QStringList list;

    Q_INIT_RESOURCE(qcodeeditor_resources);
    QFile fl(":/languages/python.xml");

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
        list.append(names);
    }
	
	list.removeDuplicates();

    setModel(new QStringListModel(list, this));
    setCompletionColumn(0);
    setModelSorting(QCompleter::CaseInsensitivelySortedModel);
    setCaseSensitivity(Qt::CaseSensitive);
    setWrapAround(true);
}
