import random
import math
import time
import sys

def is_prime(n, k=5):
    if n < 2: return False
    if n == 2 or n == 3: return True
    if n % 2 == 0: return False

    r, s = 0, n - 1
    while s % 2 == 0:
        r += 1
        s //= 2
    for _ in range(k):
        a = random.randrange(2, n - 1)
        x = pow(a, s, n)
        if x == 1 or x == n - 1:
            continue
        for _ in range(r - 1):
            x = pow(x, 2, n)
            if x == n - 1:
                break
        else:
            return False
    return True

def generate_prime_candidate(length):
    p = random.getrandbits(length)
    p |= (1 << length - 1) | 1
    return p

def generate_prime_number(length=16):
    p = 4
    while not is_prime(p):
        p = generate_prime_candidate(length)
    return p

def simular_periodo_quantico_com_progresso(a, N):
    r = 1
    start_time = time.time()
    last_print = start_time
    
    print(f"  [Info] Iniciando busca de força bruta pelo período 'r'...")
    print(f"  [Info] Isso seria instantâneo num computador quântico.")
    
    while True:
        if pow(a, r, N) == 1:
            return r
        r += 1
        
        if r % 500000 == 0:
            current_time = time.time()
            if current_time - last_print > 1.0:
                print(f"  ... testando r = {r:,} (tempo decorrido: {current_time - start_time:.1f}s)")
                last_print = current_time

def shor_algorithm_stress_test(bits):
    print(f"--- Gerando primos de {bits//2} bits para teste de {bits} bits ---")
    p = generate_prime_number(bits // 2)
    q = generate_prime_number(bits // 2)
    while p == q: # Garante primos distintos
        q = generate_prime_number(bits // 2)
    
    N = p * q
    print(f"Alvo gerado: N = {N} ({N.bit_length()} bits)")
    print(f"Fatores secretos (para conferência): {p} e {q}")
    print("-" * 50)

    attempts = 0
    while True:
        attempts += 1
        a = random.randint(2, N - 1)
        
        g = math.gcd(a, N)
        if g > 1:
            print(f"Tentativa {attempts}: [Sorte] MDC({a}, {N}) = {g}. Fatorado sem Shor!")
            return g, N // g
        
        print(f"Tentativa {attempts}: Usando a={a}. Buscando período...")
        
        r = simular_periodo_quantico_com_progresso(a, N)
        
        print(f"  -> Período encontrado: r={r}")
        
        if r % 2 != 0:
            print("  -> Falha: Período ímpar.")
            continue
        
        x = pow(a, r // 2, N)
        if x == N - 1:
            print("  -> Falha: Solução trivial.")
            continue
            
        p_calc = math.gcd(x - 1, N)
        q_calc = math.gcd(x + 1, N)
        
        if p_calc == 1 or p_calc == N: 
            if q_calc != 1 and q_calc != N: p_calc = N // q_calc
        if q_calc == 1 or q_calc == N:
             if p_calc != 1 and p_calc != N: q_calc = N // p_calc

        if p_calc * q_calc == N and p_calc != 1 and p_calc != N:
            print(f"\n[SUCESSO] Fatores encontrados: {p_calc} e {q_calc}")
            return p_calc, q_calc
        else:
            print("  -> Falha matemática. Tentando novo 'a'.")

bits_desejados = 28
shor_algorithm_stress_test(bits_desejados)
