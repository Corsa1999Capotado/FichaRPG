#pragma once

#include <QJsonObject>
#include <QString>

// Paleta de cores usada pra montar a folha de estilo (QSS) global do app.
struct Tema
{
    QString nome;

    QString corFundo;
    QString corFundoAlt; // painéis/áreas secundárias (ex: abas)
    QString corCard;
    QString corCardHover;
    QString corBorda;
    QString corTexto;
    QString corTextoSecundario;
    QString corAccent;
    QString corAccentHover;
    QString corSucesso;
    QString corAlerta;
    QString corPerigo;

    QJsonObject toJson() const;
    static Tema fromJson(const QJsonObject &obj);

    static Tema darkProfissional();
    static Tema darkGamer();
    static Tema lightClean();
};
