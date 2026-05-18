# Requisitos de Implementacao em C (RSA)

Este documento descreve o que os alunos precisam implementar em C para o projeto de criptografia RSA, usando o arquivo unico `menu.c`.

## Objetivo

Implementar um RSA basico que criptografa e descriptografa textos usando exponenciacao modular. A implementacao deve operar por caractere (ASCII), sem padding.

## Requisitos gerais

- Linguagem: C (C99 ou superior).
- Usar tipos inteiros suficientes para evitar overflow: preferencialmente `long long`.
- Trabalhar com entrada de texto via `fgets`.
- Salvar a criptografia em arquivo texto com numeros separados por espaco.
- Na descriptografia, ler o arquivo com esses numeros e reconstituir a mensagem.
- O arquivo entregue deve se chamar `menu.c`.

## Restricoes importantes do RSA por caractere

- Cada caractere e convertido para ASCII (`m`).
- Para funcionar corretamente, o modulo `n` precisa ser maior que todos os valores ASCII usados.
- Recomendacao: escolha `p` e `q` de modo que `n = p * q > 255`.

## Funcoes obrigatorias

### 1. `mod_pow`

Deve calcular exponenciacao modular de forma eficiente.

Assinatura sugerida:

```c
long long mod_pow(long long base, long long exponent, long long modulus);
```

### 2. `encriptar`

Deve receber o texto, calcular `c = m^e mod n` para cada caractere e preencher o array de criptografia.

Assinatura sugerida:

```c
void encriptar(char *mensagem, long long mensagemencriptada[], long long n, long long e);
```

### 3. `descriptografar`

Deve receber o array criptografado e aplicar `m = c^d mod n`.

Assinatura sugerida:

```c
void descriptografar(long long mensagemencriptada[], int tamanho, long long d, long long n);
```

### 4. `encotrarD`

Calcular `d` de forma simples com busca incremental, tal que:

$$d * e \equiv 1 \pmod{(p-1)(q-1)}$$

Assinatura sugerida:

```c
long long encotrarD(long long e, long long p, long long q);
```

### 5. Conversao e utilitarios

- Converter string com numeros separados por espaco para `long long[]`.
- Converter `long long[]` descriptografado de volta para `char[]`.

## Arquivo: `menu.c`

O arquivo `menu.c` deve concentrar as rotinas de gerar chave, criptografar e descriptografar.

### Funcoes exportadas obrigatorias

Essas funcoes sao chamadas pelo backend via `ctypes` e precisam existir exatamente com esses nomes:

```c
int gerarChavePub(long long primo1, long long primo2, long long expoente);
int encriptarMenu(char *mensagem, long long n, long long e);
int desencriptarMenu(long long p, long long q, int e);
```

### Fluxo esperado


1. `gerarChavePub` valida os primos, valida o `e` e grava `chavePub.txt`.
2. `encriptarMenu` criptografa a mensagem e grava `textEncript.txt`.
3. `desencriptarMenu` le `textEncript.txt`, calcula `d` e grava `textDencript.txt`.

### Estrutura minima sugerida

```c
int gerarChavePub(long long primo1, long long primo2, long long expoente);
int encriptarMenu(char *mensagem, long long n, long long e);
int desencriptarMenu(long long p, long long q, int e);

long long mod_pow(long long base, long long exponent, long long modulus);
void encriptar(char *mensagem, long long mensagemencriptada[], long long n, long long e);
void descriptografar(long long mensagemencriptada[], int tamanho, long long d, long long n);
long long encontrarD(long long e, long long p, long long q);
```

### Recomendacoes

- Remover o `\n` do final da mensagem para evitar criptografar quebra de linha.
- O modulo `n` deve ser `p * q`.
- A mensagem descriptografada pode ser impressa no terminal e deve ser salva em `textDencript.txt`.

## Erros comuns

- Usar `p` e `q` pequenos demais (faz `n` pequeno e quebra a conversao ASCII).
- Ler ou escrever o arquivo com separador errado.
- Misturar valores de `n` e `e` de chaves diferentes.
- Esquecer de remover o `\n` no final da mensagem.

## Entregavel

- `menu.c` compila como biblioteca compartilhada sem warnings graves.
- O fluxo completo deve funcionar com uma mensagem simples e `p`, `q` adequados.
