#include <stdio.h>
#include <stdlib.h>
#include <string.h>


////////////////////////////////////////////////////
// HASH TABLE — cache de ciphertext -> plaintext
//
// Por que aqui e não na criptografia?
// Na criptografia já usamos um array direto de 256 slots (índice = valor ASCII),
// que é O(1) perfeito sem colisão. Para a descriptografia o cenário é diferente:
// os valores de ciphertext podem se repetir mas não têm índice natural,
// então guardamos até 256 pares {chave cifrada, char original} e fazemos lookup.
//
// Estrutura: array linear com busca por comparação (max 256 entradas = O(1) prático)
////////////////////////////////////////////////////

#define MAX_CACHE 256

typedef struct {
    long long key;
    long long value;
    int used;
} CacheEntry;

static CacheEntry decrypt_cache[MAX_CACHE];
static int cache_size = 0;

static void cache_init(void)
{
    cache_size = 0;
    memset(decrypt_cache, 0, sizeof(decrypt_cache));
}

static int cache_get(long long key, long long *out)
{
    for (int i = 0; i < cache_size; i++)
    {
        if (decrypt_cache[i].used && decrypt_cache[i].key == key)
        {
            *out = decrypt_cache[i].value;
            return 1;
        }
    }
    return 0;
}

static void cache_set(long long key, long long value)
{
    if (cache_size >= MAX_CACHE) return;
    decrypt_cache[cache_size].key   = key;
    decrypt_cache[cache_size].value = value;
    decrypt_cache[cache_size].used  = 1;
    cache_size++;
}


////////////////////////////////////////////////////
// PROTOTIPOS DAS FUNCOES EXPORTADAS (chamadas pelo Django via ctypes)
// Parametros são strings decimais: Python passa "12345" e o C converte com atoll().
////////////////////////////////////////////////////

int gerarChavePub(const char *p_str, const char *q_str, const char *e_str);
int encriptarMenu(const char *mensagem, const char *n_str, const char *e_str);
int desencriptarMenu(const char *p_str, const char *q_str, const char *e_str);


////////////////////////////////////////////////////
// FUNCOES MATEMATICAS
////////////////////////////////////////////////////


/*terence:
Se n tem um divisor, ele sempre vem em par: n = a * b
Se a > raiz de n, então obrigatoriamente b < raiz de n, o menor sempre fica abaixo da raiz
Logo, se nenhum número até √n divide n, não existe divisor — n é primo
Usar i*i <= n evita importar math.h e é equivalente a i <= raiz de n
*/
int primo(long long n)
{
    if (n < 2) return 0;

    for (long long i = 2; i * i <= n; i++)
    {
        if (n % i == 0) return 0;
    }

    return 1;
}

//cauet: mdc implementado
/*
A função verifica qual o maximo divisor comum entre dois numeros grandes, utilizando o algoritmo de euclides tradicional.
A função usa aritmetica modular para simplificar a expressão ao máximo.
Após a simplificação, encontra o mdc que é igual ao mdc da expressão original e retorna o resultado.
*/
long long mdc(long long n1, long long n2)
{
    while (n2 != 0)
    {
        long long resto = n1 % n2;
        n1 = n2;
        n2 = resto;
    }

    return n1;
}

/*
FUNCAO MOD POW

terence: vou fazer uma maracutaia pra fazer a exponenciação ser mais eficiente
ela se baseia no fato de que qualquer expoente pode ser escrito em binario, ex: 7 = 111 (binario)
a cada iteracao, "expoente/2" descarta o bit da iteracao atual e expoente%2 le o valor (0 ou 1)
se for 0, nao inclui na soma
a base é elevada ao quadrado a cada passo, o mod em cada produto serve pra o numero nunca crescer demais
pq é melhor : se for elevando no seco fica O(n) multiplicacoes, essa solucao bacana ai faz com que seja O(logn), ideal pra quando o expoente crescer muito.
exemplo : testa com o expoente sendo um milhao e ve.
*/
long long mod_pow(long long base, long long exponent, long long modulus)
{
    long long resultado = 1;
    base = base % modulus;

    while (exponent > 0)
    {
        if (exponent % 2 == 1)
            resultado = (__int128)resultado * base % modulus;
        exponent /= 2;
        base = (__int128)base * base % modulus;
    }

    return resultado;
}

//cauet: função tot_euler feita, mas precisa ser revisada.
//Função necessária para calcular a chave pública e privada.
long long tot_euler(long long p, long long q)
{
    return (p - 1) * (q - 1);
}

//Função encontrarD implementada juntamente com o euclides estendido.
//A função encontrarD usa o euclides estendido para calcular o valor d, pois d é o inverso multiplicativo de e
void euclidesEstendido(long long a, long long b, long long *s, long long *t)
{
    // caso base
    if (a == 0)
    {
        *s = 0;
        *t = 1;
        return;
    }

    long long s1, t1;
    euclidesEstendido(b % a, a, &s1, &t1);

    // s = t1 - (b/a) * s1
    *s = t1 - (b / a) * s1;
    *t = s1;
}

long long encotrarD(long long e, long long p, long long q)
{
    long long tot = tot_euler(p, q);
    long long j, k;

    if (mdc(e, tot) != 1)
    {
        printf("Erro: mdc != 1, nao existe d!");
        return -1;
    }

    euclidesEstendido(e, tot, &j, &k);

    long long d = j % tot;
    if (d < 0) d += tot;

    return d;
}


////////////////////////////////////////////////////
// FUNCOES DE ARQUIVO
////////////////////////////////////////////////////


static void criarChavePub(long long n, long long e)
{
    FILE *file = fopen("chavePub.txt", "w");
    if (!file) return;
    fprintf(file, "%lld %lld", n, e);
    fclose(file);
}

static int salvarEmArquivo(long long *enc, int tamanho)
{
    FILE *file = fopen("textEncript.txt", "w");
    if (file == NULL) return 1;

    for (int i = 0; i < tamanho; i++)
        fprintf(file, "%lld ", enc[i]);

    fclose(file);
    return 0;
}

static int salvarEmArquivoD(const char *mensagem, int tamanho)
{
    FILE *file = fopen("textDencript.txt", "w");

    if (file == NULL)
    {
        perror("Erro ao abrir o arquivo");
        return 1;
    }

    for (int i = 0; i < tamanho; i++)
    {
        if (mensagem[i] != ' ') fprintf(file, "%c", mensagem[i]);
        else fprintf(file, " ");
    }

    fclose(file);
    return 0;
}


////////////////////////////////////////////////////
// FUNCOES DE CRIPTOGRAFIA
////////////////////////////////////////////////////


// Lookup table de 256 entradas — um slot por valor ASCII possível (0–255)
// Cada valor é calculado no máximo uma vez, por mais longa que seja a mensagem.
// Complexidade: O(256 × log e) setup + O(n) lookups, em vez de O(n × log e) sem cache.
static void encriptar(const char *mensagem, long long *enc, long long n, long long e)
{
    long long tabela[256];
    int computed[256];
    memset(computed, 0, sizeof(computed));

    for (int i = 0; mensagem[i] != '\0'; i++)
    {
        unsigned char c = (unsigned char)mensagem[i];
        if (!computed[c])
        {
            tabela[c] = mod_pow((long long)c, e, n);
            computed[c] = 1;
        }
        enc[i] = tabela[c];
    }
}

// Cache de ciphertext -> plaintext
// Cada valor criptografado único é revertido apenas uma vez via mod_pow.
// Chars repetidos são resolvidos por lookup O(1).
static void descriptografar(long long *enc, int tamanho, long long d, long long n)
{
    cache_init();

    for (int i = 0; i < tamanho; i++)
    {
        long long pt;
        if (!cache_get(enc[i], &pt))
        {
            pt = mod_pow(enc[i], d, n);
            cache_set(enc[i], pt);
        }
        enc[i] = pt;
    }
}

static void convertascii(char *mensagem, long long *enc, int tamanho)
{
    for (int i = 0; i < tamanho; i++)
        mensagem[i] = (char)enc[i];
}


////////////////////////////////////////////////////
// FUNCOES EXPORTADAS (interface com o Django)
////////////////////////////////////////////////////


int gerarChavePub(const char *p_str, const char *q_str, const char *e_str)
{
    long long p = atoll(p_str);
    long long q = atoll(q_str);
    long long e = atoll(e_str);

    if (!primo(p)) return 1;
    if (!primo(q)) return 2;

    long long n = p * q;
    if (n <= 255) return 1;

    long long phi = tot_euler(p, q);
    if (mdc(e, phi) != 1) return 3;

    criarChavePub(n, e);
    return 0;
}

int encriptarMenu(const char *mensagem, const char *n_str, const char *e_str)
{
    long long n = atoll(n_str);
    long long e = atoll(e_str);
    int tamanho = (int)strlen(mensagem);

    // Heap em vez de VLA: VLA grande estoura stack silenciosamente em mensagens longas
    long long *enc = malloc((size_t)tamanho * sizeof(long long));
    if (!enc) return 1;

    encriptar(mensagem, enc, n, e);
    salvarEmArquivo(enc, tamanho);

    free(enc);
    return 0;
}

int desencriptarMenu(const char *p_str, const char *q_str, const char *e_str)
{
    long long p = atoll(p_str);
    long long q = atoll(q_str);
    long long e = atoll(e_str);
    long long n = p * q;

    long long d = encotrarD(e, p, q);
    if (d < 0) return 1;

    FILE *file = fopen("textEncript.txt", "r");
    if (file == NULL) return 1;

    fseek(file, 0, SEEK_END);
    long tamArquivo = ftell(file);
    fseek(file, 0, SEEK_SET);

    char *bufArquivo = malloc(tamArquivo + 1);
    if (!bufArquivo) { fclose(file); return 1; }

    fread(bufArquivo, 1, tamArquivo, file);
    bufArquivo[tamArquivo] = '\0';
    fclose(file);

    long long capacidade = (tamArquivo / 4) + 16;
    long long *enc = malloc(capacidade * sizeof(long long));
    if (!enc) { free(bufArquivo); return 1; }

    long long tamanho = 0;
    char *token = strtok(bufArquivo, " \n\r");
    while (token && tamanho < capacidade)
    {
        enc[tamanho++] = atoll(token);
        token = strtok(NULL, " \n\r");
    }
    free(bufArquivo);

    descriptografar(enc, (int)tamanho, d, n);

    char *mensagem = malloc(tamanho + 1);
    if (mensagem)
    {
        convertascii(mensagem, enc, (int)tamanho);
        salvarEmArquivoD(mensagem, (int)tamanho);
        free(mensagem);
    }

    free(enc);
    return 0;
}
