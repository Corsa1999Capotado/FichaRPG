#pragma once

#include <QByteArray>
#include <QString>
#include <QStringList>

// Funções de apoio para onde as fichas (.json, organizadas por categoria/pasta),
// os templates e as imagens dos personagens ficam guardados em disco (pasta de
// dados do app do usuário).
namespace Armazenamento
{
QString pastaFichas();
QString pastaCategoria(const QString &categoria);
QString pastaImagens();
QString pastaTemplates();

// Lista as pastas/categorias existentes, na ordem definida por reordenarCategorias()
// (cria "Personagens" se ainda não houver nenhuma)
QStringList listarCategorias();
bool criarCategoria(const QString &nome, const QString &icone = QString());
bool excluirCategoria(const QString &categoria); // apaga a pasta inteira, com tudo dentro
bool renomearCategoria(const QString &nomeAtual, const QString &novoNome);
void reordenarCategorias(const QStringList &novaOrdem);
QString iconeCategoria(const QString &categoria);
bool definirIconeCategoria(const QString &categoria, const QString &icone);

// Move todos os arquivos .json de uma categoria pra outra (usado ao excluir uma
// pasta que ainda tem fichas dentro). Não move os backups.
bool moverTodasFichas(const QString &categoriaOrigem, const QString &categoriaDestino);

QStringList listarArquivosFichas(const QString &categoria);
QStringList listarArquivosTemplates();

// Gera um caminho de arquivo .json ainda não usado, baseado no nome do personagem
QString gerarNomeArquivoUnico(const QString &categoria, const QString &baseNome);
QString gerarNomeArquivoTemplateUnico(const QString &baseNome);

bool excluirFicha(const QString &caminhoFicha);       // apaga o .json e os backups dele
bool excluirTemplate(const QString &caminhoTemplate);

// Copia a imagem escolhida para a pasta de imagens do app e devolve só o nome do arquivo
QString copiarImagemParaPasta(const QString &caminhoOrigem, const QString &nomeBase);

// Backups: guarda até 5 versões comprimidas por ficha, numa pasta .backups/
// ao lado do arquivo original.
bool salvarBackup(const QString &caminhoFicha);
QStringList listarBackups(const QString &caminhoFicha); // mais recente primeiro
bool restaurarBackup(const QString &caminhoBackup, QByteArray &jsonDestino);
}
