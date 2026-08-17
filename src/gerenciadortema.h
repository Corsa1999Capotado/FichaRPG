#pragma once

#include <QObject>
#include <QVector>

#include "tema.h"

class QWidget;

// Dono do tema ativo do app: carrega/salva em config.json, aplica a folha de
// estilo (QSS) global via qApp->setStyleSheet(), e avisa quem quiser saber
// quando o tema muda (pra recalcular cores que não vêm só do QSS).
class GerenciadorTema : public QObject
{
    Q_OBJECT

public:
    static GerenciadorTema &instancia();

    Tema temaAtual() const { return m_temaAtual; }
    QVector<Tema> temasDisponiveis() const; // presets + customizados salvos

    void aplicarTema(const Tema &tema);
    void salvarComoTemaCustomizado(const Tema &tema); // adiciona/atualiza e já aplica

    // Reaplica o QSS num widget depois que uma propriedade dinâmica dele muda
    // em tempo de execução (ex: [calculado="true"], [invalido="true"]).
    static void repolir(QWidget *widget);

signals:
    void temaAlterado(const Tema &tema);

private:
    GerenciadorTema();

    void carregarConfig();
    void salvarConfig() const;
    QString gerarFolhaEstilo(const Tema &t) const;

    Tema m_temaAtual;
    QVector<Tema> m_temasCustomizados;
};
