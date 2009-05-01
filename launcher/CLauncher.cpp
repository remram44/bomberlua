#include "CLauncher.h"

CLauncher::CLauncher(QWidget *pParent)
  : QWidget::QWidget(pParent)
{
    // Configuration of the window
    setWindowTitle(tr("BomberLua launcher"));

    // Horizontal layout
    {
        QHBoxLayout *layout = new QHBoxLayout;

        // List of bots
        m_pBotsList = new QListWidget;
        layout->addWidget(m_pBotsList);

        // Vertical layout
        {
            QVBoxLayout *column = new QVBoxLayout;

            // Button to add a bot
            QPushButton *add = new QPushButton(tr("Add a bot"));
            connect(add, SIGNAL(clicked()), this, SLOT(addBot()));
            column->addWidget(add);

            // Button to remove a bot
            QPushButton *remove = new QPushButton(tr("Remove this bot"));
            connect(remove, SIGNAL(clicked()), this, SLOT(removeBot()));
            column->addWidget(remove);

            // Allows to choose whether to run graphically or not
            m_pGraphic = new QCheckBox(tr("Graphics"));
            m_pGraphic->setCheckState(Qt::Checked);
            column->addWidget(m_pGraphic);

            // Button to start the game
            QPushButton *startButton = new QPushButton(tr("Start"));
            connect(startButton, SIGNAL(clicked()), this, SLOT(start()));
            column->addWidget(startButton);

            layout->addLayout(column);
        }

        setLayout(layout);
    }
}

void CLauncher::addBot()
{
    QStringList bots = QFileDialog::getOpenFileNames(this, tr("Add a bot"),
        "../bots", tr("Bots (*.lua)"));
    QStringList::iterator it = bots.begin();
    for(; it != bots.end(); it++)
    {
        //QFileInfo fi(*it);
        //m_pBotsList->addItem(fi.fileName());
        m_pBotsList->addItem(*it);
    }
}

void CLauncher::removeBot()
{
    delete m_pBotsList->takeItem(m_pBotsList->currentRow());
}

void CLauncher::start()
{
    QStringList bots;
    int i;
    for(i = 0; i < m_pBotsList->count(); i++)
    {
        bots << m_pBotsList->item(i)->text();
    }
    new ExecWindow(this, bots, m_pGraphic->isChecked());
}

ExecWindow::ExecWindow(QWidget *pParent, const QStringList &bots, bool graphic)
  : QDialog::QDialog(pParent)
{
    setWindowTitle(tr("BomberLua"));

    QVBoxLayout *layout = new QVBoxLayout;

    m_pOutput = new QTextEdit;
    m_pOutput->setReadOnly(true);

    layout->addWidget(m_pOutput);

    setLayout(layout);
    show();

    m_pBomberLua = new QProcess(this);
    QStringList args(bots);
    if(graphic)
        args.insert(args.begin(), "-i");
    QString blDir = QDir::cleanPath(QDir::currentPath() + "/..");
    m_pBomberLua->setWorkingDirectory(blDir);
    connect(m_pBomberLua, SIGNAL(readyReadStandardOutput()),
        this, SLOT(read()));
    connect(m_pBomberLua, SIGNAL(readyReadStandardError()), this, SLOT(read()));
    QMessageBox::information(this, "wd", m_pBomberLua->workingDirectory());
#ifdef __WIN32__
    m_pBomberLua->start(blDir + "/bomberlua.exe", args);
#else
    m_pBomberLua->start("./bomberlua", args);
#endif
}

void ExecWindow::read()
{
    m_pOutput->append(m_pBomberLua->readAllStandardError());
    m_pOutput->append(m_pBomberLua->readAllStandardOutput());
}
