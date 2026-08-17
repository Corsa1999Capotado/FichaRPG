#include "tema.h"

QJsonObject Tema::toJson() const
{
    QJsonObject obj;
    obj["nome"] = nome;
    obj["corFundo"] = corFundo;
    obj["corFundoAlt"] = corFundoAlt;
    obj["corCard"] = corCard;
    obj["corCardHover"] = corCardHover;
    obj["corBorda"] = corBorda;
    obj["corTexto"] = corTexto;
    obj["corTextoSecundario"] = corTextoSecundario;
    obj["corAccent"] = corAccent;
    obj["corAccentHover"] = corAccentHover;
    obj["corSucesso"] = corSucesso;
    obj["corAlerta"] = corAlerta;
    obj["corPerigo"] = corPerigo;
    return obj;
}

Tema Tema::fromJson(const QJsonObject &obj)
{
    Tema t;
    t.nome = obj.value("nome").toString();
    t.corFundo = obj.value("corFundo").toString();
    t.corFundoAlt = obj.value("corFundoAlt").toString();
    t.corCard = obj.value("corCard").toString();
    t.corCardHover = obj.value("corCardHover").toString();
    t.corBorda = obj.value("corBorda").toString();
    t.corTexto = obj.value("corTexto").toString();
    t.corTextoSecundario = obj.value("corTextoSecundario").toString();
    t.corAccent = obj.value("corAccent").toString();
    t.corAccentHover = obj.value("corAccentHover").toString();
    t.corSucesso = obj.value("corSucesso").toString();
    t.corAlerta = obj.value("corAlerta").toString();
    t.corPerigo = obj.value("corPerigo").toString();
    return t;
}

Tema Tema::darkProfissional()
{
    Tema t;
    t.nome = "Dark Profissional";
    t.corFundo = "#121212";
    t.corFundoAlt = "#171717";
    t.corCard = "#242424";
    t.corCardHover = "#2a2a2a";
    t.corBorda = "#3a3a3a";
    t.corTexto = "#e0e0e0";
    t.corTextoSecundario = "#a0a0a0";
    t.corAccent = "#4a90d9";
    t.corAccentHover = "#5aa0e9";
    t.corSucesso = "#2ecc71";
    t.corAlerta = "#d4af37";
    t.corPerigo = "#e74c3c";
    return t;
}

Tema Tema::darkGamer()
{
    Tema t;
    t.nome = "Dark Gamer";
    t.corFundo = "#0d0d14";
    t.corFundoAlt = "#14141f";
    t.corCard = "#1c1c2b";
    t.corCardHover = "#23233a";
    t.corBorda = "#35354f";
    t.corTexto = "#e6e6ff";
    t.corTextoSecundario = "#9a9ac0";
    t.corAccent = "#a855f7";
    t.corAccentHover = "#c084fc";
    t.corSucesso = "#39ff14";
    t.corAlerta = "#ffd60a";
    t.corPerigo = "#ff2e63";
    return t;
}

Tema Tema::lightClean()
{
    Tema t;
    t.nome = "Light Clean";
    t.corFundo = "#f5f5f7";
    t.corFundoAlt = "#ffffff";
    t.corCard = "#ffffff";
    t.corCardHover = "#eef0f4";
    t.corBorda = "#d8d8dd";
    t.corTexto = "#1a1a1a";
    t.corTextoSecundario = "#6a6a6a";
    t.corAccent = "#2563eb";
    t.corAccentHover = "#3b82f6";
    t.corSucesso = "#16a34a";
    t.corAlerta = "#b45309";
    t.corPerigo = "#dc2626";
    return t;
}
