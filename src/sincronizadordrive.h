#pragma once

#include <QString>

class QWidget;

// Orquestra "Enviar tudo" / "Baixar tudo" entre a pasta local de dados do app
// e uma pasta "FichaRPG" no Google Drive do usuário. Fase 1: sincronização
// manual, disparada pelo usuário — sem automação em background nem resolução
// de conflito ainda (baixar/enviar tudo sobrescreve arquivos de mesmo nome).
namespace SincronizadorDrive
{
// Devolve mensagem de erro (vazia = sucesso). `pai` só parenta o diálogo de progresso.
QString enviarTudo(QWidget *pai);
QString baixarTudo(QWidget *pai);

// Envia só um arquivo (ficha, template ou imagem) pro Drive, sem diálogo de
// progresso — usado pra sincronizar em segundo plano assim que algo é salvo
// localmente. Não faz nada (devolve erro) se não houver conta conectada.
// caminhoLocal precisa estar dentro de uma das pastas que Armazenamento
// gerencia (fichas/<categoria>, templates ou imagens).
QString enviarArquivoUnico(const QString &caminhoLocal);
}
