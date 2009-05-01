#ifndef CSCRIPT_H
#define CSCRIPT_H

#include <QtGui>

/**
 * The launcher.
 */
class CLauncher : public QWidget {

    Q_OBJECT

private:
    QListWidget *m_pBotsList;
    QCheckBox *m_pGraphic;

public:
    CLauncher(QWidget *pParent = NULL);

public slots:
    void addBot();
    void removeBot();
    void start();

};

/**
 * The game window, displaying the output.
 */
class ExecWindow : public QDialog {

    Q_OBJECT

private:
    QTextEdit *m_pOutput;
    QProcess *m_pBomberLua;

public:
    ExecWindow(QWidget *pParent, const QStringList &bots, bool graphic);

public slots:
    void read();

};

#endif
