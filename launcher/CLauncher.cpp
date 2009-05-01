#include "CLauncher.h"

CLauncher::CLauncher(QWidget *pParent)
  : QWidget::QWidget(pParent)
{
    // Paramétrage de la fenêtre
    setWindowTitle(tr("BomberLua launcher"));

    // Layout horizontal
    {
        QHBoxLayout *layout = new QHBoxLayout;

        // Liste des bots
        m_pBotsList = new QListWidget;
        layout->addWidget(m_pBotsList);

        // Layout vertical
        {
            QVBoxLayout *column = new QVBoxLayout;

            // Bouton pour ajouter un bot
            QPushButton *add = new QPushButton(tr("Ajouter un bot"));
            connect(add, SIGNAL(clicked()), this, SLOT(addBot()));
            column->addWidget(add);

            // Bouton pour enlever un bot
            QPushButton *remove = new QPushButton(tr("Retirer ce bot"));
            connect(remove, SIGNAL(clicked()), this, SLOT(removeBot()));
            column->addWidget(remove);

            // Permet de choisir si on est graphique ou pas
            m_pGraphic = new QCheckBox(tr("Graphique"));
            m_pGraphic->setCheckState(Qt::Checked);
            column->addWidget(m_pGraphic);

            // Bouton pour lancer le bazar
            QPushButton *startButton = new QPushButton(tr("Lancer"));
            connect(startButton, SIGNAL(clicked()), this, SLOT(start()));
            column->addWidget(startButton);

            layout->addLayout(column);
        }

        setLayout(layout);
    }
}

void CLauncher::addBot()
{
    QStringList bots = QFileDialog::getOpenFileNames(this, tr("Ajout d'un bot"),
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
