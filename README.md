# RSA Project (Matematica Discreta)

Projeto academico de criptografia RSA com interface web em Django e a logica criptografica implementada em C.

## Stack

- Python 3
- Django 4.1
- HTML + CSS com templates do Django
- C para a camada RSA
- SQLite, que e o banco padrao do Django neste projeto

## Arquitetura

### Frontend

O frontend fica nos templates em `projetomd/mdproject/templates/mdproject/`.

As telas principais sao:

- `index.html`: menu inicial
- `save.html`: formulario para gerar e salvar chave publica
- `crypt.html`: formulario para criptografar uma mensagem
- `decrypt.html`: formulario para descriptografar uma mensagem

O layout visual e controlado por `projetomd/mdproject/static/css/styles.css`.

### Backend

O backend e um app Django chamado `mdproject`.

As rotas principais sao:

- `/mdproject/`
- `/mdproject/save/`
- `/mdproject/crypt/`
- `/mdproject/descrypt/`

As views recebem os dados dos formularios, convertem os campos para inteiro quando necessario e chamam funcoes da biblioteca C via `ctypes`.

O banco SQLite existe na estrutura do projeto, mas nao ha models com persistencia real sendo usados no fluxo atual.

### Camada C

O arquivo C central para a versao web e `projetomd/mdproject/c_code/menu.c`.

Ele concentra as funcoes para:

- validar primos e calcular `mdc`
- gerar chave publica
- criptografar
- descriptografar
- gravar resultados em arquivos texto

Os arquivos de saida usados pelo fluxo sao:

- `chavePub.txt`
- `textEncript.txt`
- `textDencript.txt`

## Arquivos C necessarios

Para o programa web funcionar, o essencial e:

- `projetomd/mdproject/c_code/menu.c`
- a biblioteca compilada a partir dele, como `menu.dll` no Windows ou `menu.so` no Linux

Os demais arquivos C do repositório nao sao necessarios para o fluxo web atual:

- `encriptar.c`
- `desencriptar.c`
- `menu.c` na raiz do projeto
- `publicKeyTests/public.c`

Esses arquivos sao versoes CLI, testes ou prototipos do mesmo algoritmo.

## Como rodar o projeto

### 1. Criar o ambiente virtual

```bash
python3 -m venv .venv
source .venv/bin/activate
```

### 2. Instalar as dependencias

```bash
pip install -r requirements.txt
```

### 3. Enviar o zip com os arquivos C

Na tela inicial (`/mdproject/`), envie um arquivo `.zip` contendo **obrigatoriamente**:

- `encriptar.c`
- `desencriptar.c`

O sistema vai descompactar, substituir os arquivos atuais e compilar automaticamente os executaveis usando `gcc`.

### 4. Ajustar o caminho da biblioteca no backend

O arquivo `projetomd/mdproject/views.py` carrega a biblioteca C por meio de `ctypes.CDLL(...)`.

No Linux, ele agora procura automaticamente por `menu.so` dentro de `projetomd/mdproject/c_code/`.

### 5. Preparar o banco e iniciar o servidor

```bash
python manage.py migrate
python manage.py runserver
```

Depois abra:

```bash
http://127.0.0.1:8000/mdproject/
```

## Para generalizar para alunos

Se a ideia e deixar o aluno focar apenas na logica do RSA, o melhor caminho e manter o Django somente como interface e pedir que o aluno mexa apenas em `projetomd/mdproject/c_code/menu.c`.

Se quiser simplificar ainda mais, o mesmo RSA pode ser separado em um projeto C puro, deixando o frontend/backend apenas como uma camada opcional.

## Observacoes

- O projeto foi escrito com uma dependencia de biblioteca C carregada por `ctypes`, entao o caminho da DLL ou da SO precisa bater com o sistema operacional.
- O nome da rota de descriptografia esta como `descrypt` no codigo atual.

---

## MUDANÇAS E IMPLEMENTAÇÕES (TERENCE(MEU REPOSITORIO)/GRUPO)

### Interface web (frontend)

Os quatro templates foram redesenhados com visual dark-green terminal, usando apenas HTML e CSS inline (sem arquivos externos). As principais mudanças por tela:

- **index.html**: adicionado painel de progresso com badges nas etapas (salvar chave → criptografar → descriptografar), botão "nova criptografia" e acordeão para envio de ZIP com o `menu.c` do aluno.
- **save.html**: corrigido bug onde o toast de sucesso era disparado também em caso de erro (condição `{% if status %}` foi trocada para verificar a string exata de sucesso). Adicionada exibição de erros em vermelho com link de volta ao início.
- **crypt.html**: campo de mensagem alterado de `<input>` para `<textarea>` para suportar textos longos sem travar o navegador. Adicionados toast de sucesso e barra de progresso animada com redirecionamento automático ao index após criptografia bem-sucedida.
- **decrypt.html**: adicionado link de volta ao início e exibição do texto descriptografado na própria tela.

### Backend Django (views.py / urls.py / settings.py)

- A interface com a biblioteca C foi migrada de parâmetros `long long` para `const char*`: Python agora passa strings com `.encode()` e a biblioteca converte internamente com `atoll()`. Isso elimina problemas de alinhamento de tipos entre Python e C.
- Adicionada rota e view `reset/` que apaga `textEncript.txt`, `textDencript.txt` e `chavePub.txt` no servidor quando o botão "nova criptografia" é clicado.
- `DATA_UPLOAD_MAX_MEMORY_SIZE` aumentado para 20 MB em `settings.py` para suportar textos longos como a Bíblia de Gutenberg.

### Camada C (menu.c)

**Correção de stack overflow**: os arrays de saída em `encriptarMenu` e `desencriptarMenu` eram declarados com tamanho fixo na stack. Textos longos (~800k caracteres) causavam crash silencioso. Substituídos por alocação dinâmica com `malloc`.

**Cache de criptografia (lookup table 256 entradas)**: como ASCII tem no máximo 256 valores distintos, a função `encriptar` agora calcula `mod_pow` para cada caractere único no máximo uma vez e reutiliza o resultado para repetições. Complexidade cai de O(n × log e) para O(256 × log e) de setup + O(n) de lookup.

**Cache de descriptografia (array linear, máx. 256 entradas)**: a função `descriptografar` usa uma estrutura `CacheEntry {key, value, used}` para guardar pares `{ciphertext → plaintext}`. Cada valor cifrado único é revertido via `mod_pow` apenas uma vez; caracteres repetidos são resolvidos por busca linear O(1) na prática (ASCII tem no máximo 256 valores distintos por mensagem).





