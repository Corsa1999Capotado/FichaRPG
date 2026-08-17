# Tutorial rápido — FichaRPG

## Criando uma ficha

1. Na tela inicial, clique no botão **+** (canto inferior direito).
2. Escolha uma opção:
   - **Importar de .txt** — cola/aponta um arquivo de texto com a ficha (nome, atributos, inventário etc.) e o app tenta preencher tudo sozinho.
   - **Usar template** — parte de um modelo salvo antes.
   - **Criar do zero** — ficha em branco.
3. Preencha os campos e clique em **Salvar**.

### Formato aceito no "Importar de .txt"

O app entende tanto um formato estruturado (com `====Atributos====`, `====Sub-Atributos====`, `Habilidades:`) quanto texto solto. Em ambos os casos ele reconhece uma seção de inventário assim:

```
Inventário: Peso 8.5

Revolver 38 (6/6) 42m. [1d6] PESO 1
2 kit médicos PESO 2

bloco de notas:
-folha com símbolos desenhados
```

- Quantidade no início da linha (`2 kit médicos`), `PESO x` em qualquer parte, e linhas terminando em `:` viram um item cujas linhas seguintes com `-` formam a descrição.

## Organizando em pastas

Use as abas no topo (ao lado de "Personagens") para categorias diferentes (ex: por campanha). O ícone de engrenagem ao lado gerencia (cria/renomeia/exclui) essas pastas.

## Temas e aparência

- **🌗** alterna claro/escuro rapidamente.
- **🎨 Temas** deixa escolher ou criar uma paleta de cores personalizada.

## Sincronizando entre PCs (Google Drive)

1. Clique em **"Conta: não conectado"** no menu principal.
2. Cole o **Client ID** e **Client Secret** (gerados no [Google Cloud Console](https://console.cloud.google.com/), API do Google Drive, credencial tipo "App para computador") e clique em **Salvar credenciais**.
3. Clique em **🔐 Conectar conta do Google** e autorize no navegador.
4. Pronto — a partir daí, **toda ficha que você salvar é enviada pro Drive automaticamente**. No outro PC, repita os passos 1–3 com a mesma conta e use **"☁️ Google Drive" → "📥 Baixar tudo do Drive"** pra trazer o que já existe.
5. Pra sair da conta, volte em **Conta → 🔓 Sair da conta**.

## Backup e exportação

- Cada ficha guarda até 5 versões anteriores automaticamente (restaurável na tela de edição).
- Use **Exportar** (na tela de visualização da ficha) pra gerar um PDF/imagem pra compartilhar fora do app.
