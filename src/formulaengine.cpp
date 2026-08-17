#include "formulaengine.h"

namespace
{
// Recursive descent: expressao := termo (('+'|'-') termo)*
//                    termo     := fator (('*'|'/') fator)*
//                    fator     := numero | variavel | '(' expressao ')' | ('-'|'+') fator
class Avaliador
{
public:
    Avaliador(const QString &texto, const QMap<QString, double> &variaveis)
        : m_texto(texto)
        , m_variaveis(variaveis)
    {
    }

    double avaliar(bool *ok)
    {
        m_pos = 0;
        m_ok = true;

        const double resultado = expressao();
        pularEspacos();
        if (m_pos != m_texto.length())
            m_ok = false;

        if (ok)
            *ok = m_ok;
        return m_ok ? resultado : 0.0;
    }

private:
    QString m_texto;
    QMap<QString, double> m_variaveis;
    int m_pos = 0;
    bool m_ok = true;

    void pularEspacos()
    {
        while (m_pos < m_texto.length() && m_texto[m_pos].isSpace())
            m_pos++;
    }

    QChar espiar()
    {
        pularEspacos();
        return m_pos < m_texto.length() ? m_texto[m_pos] : QChar();
    }

    double expressao()
    {
        double valor = termo();
        while (m_ok) {
            const QChar c = espiar();
            if (c == QLatin1Char('+')) {
                m_pos++;
                valor += termo();
            } else if (c == QLatin1Char('-')) {
                m_pos++;
                valor -= termo();
            } else {
                break;
            }
        }
        return valor;
    }

    double termo()
    {
        double valor = fator();
        while (m_ok) {
            const QChar c = espiar();
            if (c == QLatin1Char('*')) {
                m_pos++;
                valor *= fator();
            } else if (c == QLatin1Char('/')) {
                m_pos++;
                const double divisor = fator();
                if (!m_ok)
                    return 0.0;
                if (divisor == 0.0) {
                    m_ok = false;
                    return 0.0;
                }
                valor /= divisor;
            } else {
                break;
            }
        }
        return valor;
    }

    double fator()
    {
        const QChar c = espiar();

        if (c.isNull()) {
            m_ok = false;
            return 0.0;
        }
        if (c == QLatin1Char('-')) {
            m_pos++;
            return -fator();
        }
        if (c == QLatin1Char('+')) {
            m_pos++;
            return fator();
        }
        if (c == QLatin1Char('(')) {
            m_pos++;
            const double valor = expressao();
            if (espiar() != QLatin1Char(')')) {
                m_ok = false;
                return 0.0;
            }
            m_pos++;
            return valor;
        }
        if (c.isDigit() || c == QLatin1Char('.'))
            return numero();
        if (c.isLetter() || c == QLatin1Char('_'))
            return variavel();

        m_ok = false;
        return 0.0;
    }

    double numero()
    {
        pularEspacos();
        const int inicio = m_pos;
        while (m_pos < m_texto.length() && (m_texto[m_pos].isDigit() || m_texto[m_pos] == QLatin1Char('.')))
            m_pos++;

        bool ok = false;
        const double valor = m_texto.mid(inicio, m_pos - inicio).toDouble(&ok);
        if (!ok)
            m_ok = false;
        return valor;
    }

    double variavel()
    {
        pularEspacos();
        const int inicio = m_pos;
        while (m_pos < m_texto.length() && (m_texto[m_pos].isLetterOrNumber() || m_texto[m_pos] == QLatin1Char('_')))
            m_pos++;

        const QString nome = m_texto.mid(inicio, m_pos - inicio);

        for (auto it = m_variaveis.constBegin(); it != m_variaveis.constEnd(); ++it) {
            if (it.key().compare(nome, Qt::CaseInsensitive) == 0)
                return it.value();
        }

        m_ok = false;
        return 0.0;
    }
};
}

namespace FormulaEngine
{
double avaliar(const QString &formula, const QMap<QString, double> &variaveis, bool *ok)
{
    if (formula.trimmed().isEmpty()) {
        if (ok)
            *ok = false;
        return 0.0;
    }

    Avaliador a(formula, variaveis);
    return a.avaliar(ok);
}
}
