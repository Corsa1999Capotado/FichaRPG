#pragma once

#include <QWidget>

#include "character.h"

// Card clicável exibido no grid do menu: foto com nome sobreposto (gradiente
// escuro no rodapé pra legibilidade), selo de status e uma linha rápida de
// info (idade + vida com barrinha) embaixo.
class CardFicha : public QWidget
{
    Q_OBJECT

public:
    explicit CardFicha(const QString &caminhoArquivo, const CharacterSheet &ficha, const QString &caminhoImagem, QWidget *parent = nullptr);

    QString caminhoArquivo() const { return m_caminhoArquivo; }

signals:
    void clicado(const QString &caminhoArquivo);
    void editarSolicitado(const QString &caminhoArquivo);
    void visualizarSolicitado(const QString &caminhoArquivo);
    void duplicarSolicitado(const QString &caminhoArquivo);
    void moverSolicitado(const QString &caminhoArquivo);
    void exportarSolicitado(const QString &caminhoArquivo);
    void excluirSolicitado(const QString &caminhoArquivo);

protected:
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    void abrirMenuAcoes();

    QString m_caminhoArquivo;
};
