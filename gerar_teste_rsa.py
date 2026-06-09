# Script de teste para validar o "descriptografar externo"
# Gera uma mensagem cifrada com chaves conhecidas e salva em teste_cifrado.txt
#
# Como usar:
#   py gerar_teste_rsa.py
#
# Depois, no sistema web:
#   1. Vá em Criptografar → aba "descriptografar externo"
#   2. Cole o conteúdo de teste_cifrado.txt (ou envie o arquivo)
#   3. Digite p, q, e conforme mostrado abaixo
#   4. Clique em Descriptografar — deve aparecer a mensagem original

# ── Algoritmos RSA puros (mesmos do menu.c) ──────────────────────────────────

def mod_pow(base, exponent, modulus):
    """Exponenciação modular — idêntica ao mod_pow() do menu.c."""
    resultado = 1
    base = base % modulus
    while exponent > 0:
        if exponent % 2 == 1:
            resultado = resultado * base % modulus
        exponent //= 2
        base = base * base % modulus
    return resultado


def mdc(a, b):
    while b != 0:
        a, b = b, a % b
    return a


def euclides_estendido(a, b):
    if a == 0:
        return 0, 1
    s1, t1 = euclides_estendido(b % a, a)
    return t1 - (b // a) * s1, s1


def encontrar_d(e, p, q):
    tot = (p - 1) * (q - 1)
    assert mdc(e, tot) == 1, "mdc(e, φ(n)) ≠ 1 — escolha outro e"
    d, _ = euclides_estendido(e, tot)
    return d % tot


def primo(n):
    if n < 2:
        return False
    for i in range(2, int(n**0.5) + 1):
        if n % i == 0:
            return False
    return True


# ── Parâmetros do teste ───────────────────────────────────────────────────────
# Primos grandes o suficiente para cobrir ASCII (n > 255)
# e pequenos o suficiente para serem fáceis de digitar no sistema

P = 9001          # primo
Q = 9013          # primo
E = 65537         # expoente público padrão

# ── Validações ────────────────────────────────────────────────────────────────
assert primo(P), f"{P} não é primo"
assert primo(Q), f"{Q} não é primo"
N = P * Q
TOT = (P - 1) * (Q - 1)
assert mdc(E, TOT) == 1, f"mdc({E}, {TOT}) ≠ 1"
D = encontrar_d(E, P, Q)

# ── Mensagem de teste ─────────────────────────────────────────────────────────
MENSAGEM = "Ola equipe! Esta mensagem foi cifrada com RSA. Voces conseguem decifrar?"

# ── Cifrar ────────────────────────────────────────────────────────────────────
cifrado = [mod_pow(ord(c), E, N) for c in MENSAGEM]
cifrado_str = " ".join(str(v) for v in cifrado)

# ── Verificar que descriptografa corretamente ─────────────────────────────────
decifrado = "".join(chr(mod_pow(v, D, N)) for v in cifrado)
assert decifrado == MENSAGEM, "ERRO: descriptografia falhou no teste interno!"

# ── Salvar arquivo ────────────────────────────────────────────────────────────
with open("teste_cifrado.txt", "w", encoding="utf-8") as f:
    f.write(cifrado_str)

# ── Saída ─────────────────────────────────────────────────────────────────────
print(f"mensagem : {MENSAGEM}")
print(f"p        : {P}")
print(f"q        : {Q}")
print(f"e        : {E}")
print(f"\nmensagem cifrada:\n{cifrado_str}")
print(f"\ncifra também salva em: teste_cifrado.txt")
