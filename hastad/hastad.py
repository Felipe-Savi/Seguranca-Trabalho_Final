from functools import reduce

e = 3
m_real = 25
N = [629, 2173, 1159]

c = [pow(m_real, e, n) for n in N]

print(f"Ciphertexts interceptados: {c}")

def extended_gcd(a, b):
    if a == 0: return b, 0, 1
    else:
        gcd, x, y = extended_gcd(b % a, a)
        return gcd, y - (b // a) * x, x

def modinv(a, m):
    gcd, x, y = extended_gcd(a, m)
    if gcd != 1: raise Exception('Inverso modular não existe')
    return x % m

def solve_crt(rem, mod):
    full_product = reduce(lambda x, y: x * y, mod)
    result = 0
    for r, m in zip(rem, mod):
        partial_product = full_product // m
        inverse = modinv(partial_product, m)
        result += r * partial_product * inverse
    return result % full_product

C_reconstruido = solve_crt(c, N)

m_calculado = int(round(C_reconstruido ** (1/e)))

print(f"Valor reconstruído via CRT: {C_reconstruido}")
print(f"Mensagem recuperada: {m_calculado}")
print(f"Ataque bem sucedido? {m_calculado == m_real}")