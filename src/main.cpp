#include <QApplication>
#include <QIcon>

#include "gerenciadortema.h"
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setWindowIcon(QIcon(":/icone.ico"));
    GerenciadorTema::instancia(); // carrega e aplica o tema salvo antes de montar a janela

    MainWindow janela;
    janela.show();

    return app.exec();
}
