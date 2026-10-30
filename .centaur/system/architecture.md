# Arquitetura

O objetivo do projeto é criar uma **versão bem simplificada do cérebro**. Este documento separa o que **existe hoje** (a base MLP, que passa a ser a referência/legado) do que está **planejado**. As regras normativas estão nos contratos [`base`](../contracts/base/v002.json) (o MLP atual) e [`cerebro`](../contracts/cerebro/v001.json) (a direção nova); este documento só explica e vincula. A pesquisa que fundamenta a mudança está em [pesquisa-cerebro.md](pesquisa-cerebro.md).

# Parte 1 — Estado atual: a base MLP (referência)

Fatos extraídos do código em 2026-10-29. Não é um neurônio: é uma rede de camadas que prevê o próximo byte. Continua no repositório como referência e oráculo de testes até ser substituída.

## Fluxo

```
fonte (arquivo | pasta | stdin)
   │  captação  ─ src/capture/capture.cpp
   ▼
bytes ──► janela de 16 bytes ──► rede ─ src/model/model.cpp ──► P(próximo byte)
                                  │
                                  ▼ aprendizado (learn)
                           parâmetros atualizados
                                  │  armazenamento ─ src/store/store.cpp
                                  ▼
                      checkpoint (.child) + linhagem
```

## Módulos e dependências

| Módulo | Arquivos | Responsabilidade | Depende de |
|---|---|---|---|
| Captação | [capture.hpp](../../src/capture/capture.hpp), [capture.cpp](../../src/capture/capture.cpp) | Fonte → bytes determinísticos | nada do projeto |
| Processamento | [model.hpp](../../src/model/model.hpp), [model.cpp](../../src/model/model.cpp), [rng.hpp](../../src/model/rng.hpp) | Rede, retropropagação, SGD, `learn` | nada do projeto |
| Armazenamento | [store.hpp](../../src/store/store.hpp), [store.cpp](../../src/store/store.cpp) | Checkpoint, checksum, linhagem, `fork` | `model.hpp` (só `Config`) |
| CLI | [main.cpp](../../src/main.cpp) | `init`, `fork`, `info`, `feed`, `sample` | os três acima |

Processamento e captação não se conhecem; só o CLI os une.

## Modelo

Embeddings dos 16 bytes anteriores concatenados → `tanh(W1·x + b1)` → softmax(`W2·h + b2`). Vocabulário fixo de 256 (um byte por token). Configuração padrão: embed 32, contexto 16, hidden 128, o que dá 106.880 parâmetros guardados num único vetor plano (ordem: embeddings, W1, b1, W2, b2). O byte 0 é o preenchimento antes do início e o separador entre arquivos.

## Formato do checkpoint (versão 1, little-endian)

`"CHLD"` · versão `u32` · embed/context/hidden `u32` · id/pai/passos `u64` · tamanho do nome `u32` + nome · nº de parâmetros `u64` + `float32[]` · FNV-1a 64 bits de tudo o que precede. A escrita usa arquivo `.tmp` seguido de `rename`.

## Limites conhecidos (observados, não requisitos)

- Aprendizado de cerca de 9 mil bytes/s numa thread, medido em 22 KB de código.
- Janela fixa de 16 bytes, sem memória além dela.
- Sem proteção contra esquecimento catastrófico (prevista no roteiro, via hipocampo e replay).

# Parte 2 — Direção: uma versão simplificada do cérebro (planejado)

Nada abaixo está implementado. O primeiro degrau tem contrato aprovado ([`cerebro`](../contracts/cerebro/v001.json)); os demais ainda não têm contrato.

## Princípios (decididos)

- **Fidelidade em degraus**: neurônio de ponto AdEx primeiro; dendritos humanos depois.
- **Aprendizado por regras locais e online** (STDP e três fatores com dopamina), sem retropropagação no código novo.
- **Nomes pelas regiões do cérebro**: cada pasta tem um arquivo principal de mesmo nome; identificadores em inglês.
- **CUDA é um backend opcional**; o núcleo não o conhece e o build padrão exige só C++20 e CMake.

## Estrutura-alvo (proposta de nomes)

| Pasta | Estrutura do cérebro | Responsabilidade | Substitui |
|---|---|---|---|
| `src/thalamus/` | tálamo, relé sensorial | Filtrar e codificar bytes em disparos | `capture/` |
| `src/neuron/` | neurônio | Dinâmica AdEx: integrar, disparar, reiniciar, adaptar | parte de `model/` |
| `src/synapse/` | sinapse | Peso e plasticidade STDP | parte de `model/` |
| `src/dopamine/` | dopamina | Sinal de recompensa, o terceiro fator | novo |
| `src/hippocampus/` | hipocampo | Memória rápida e repetição (replay) | novo |
| `src/engram/` | engrama, o traço físico da memória | Checkpoint, linhagem e `fork` | `store/` |

As dependências seguem a regra de dentro para fora: neurônio e sinapse são o núcleo e não conhecem tálamo nem engrama. As pastas legadas (`capture/`, `model/`, `store/`) saem conforme cada substituta for entregue.

## Roteiro

1. **`cerebro`** (contrato aprovado): neurônio AdEx, sinapse com STDP, dopamina como terceiro fator e um neurônio que aprende um padrão.
2. **Neurônio humano**: dendritos com NMDA e picos de cálcio, com o critério de que um único neurônio resolve XOR, comparado ao modelo da [ModelDB 260178](https://modeldb.science/260178).
3. **Hipocampo e replay**: aprendizado rápido e consolidação, contra o esquecimento catastrófico.
4. **Escala e GPU**: estado por neurônio e sinapse, backend CUDA opcional e medição na RTX 3060.
5. **Engrama**: persistência do estado dos neurônios, preservando linhagem, atomicidade e detecção de corrupção.
