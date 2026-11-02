# Arquitetura

O objetivo do projeto é criar uma **versão bem simplificada do cérebro**. Este documento separa o que **existe hoje** (a base MLP, que passa a ser a referência/legado) do que está **planejado**. As regras normativas estão nos contratos [`base`](../contracts/base/v002.json) (o MLP atual), [`cerebro`](../contracts/cerebro/v002.json) (a direção nova), [`engrama`](../contracts/engrama/v001.json), [`reprodutibilidade`](../contracts/reprodutibilidade/v001.json) e [`base-aberta`](../contracts/base-aberta/v001.json). Este documento só explica e vincula. A pesquisa está em [docs/](../../docs/README.md), e o roteiro, na [síntese](../../docs/sintese.md).

# Parte 1 — Estado atual: a base MLP (referência)

Fatos extraídos do código em 2026-10-29 e atualizados em 2026-11-02 (engrama em seções, build portátil). Não é um neurônio: é uma rede de camadas que prevê o próximo byte. Continua no repositório como referência e oráculo de testes até ser substituída.

## Fluxo

```
fonte (arquivo | pasta | stdin)
   │  captação  ─ src/capture/capture.cpp
   ▼
bytes ──► janela de 16 bytes ──► rede ─ src/model/model.cpp ──► P(próximo byte)
                                  │
                                  ▼ aprendizado (learn)
                           parâmetros atualizados
                                  │  engrama ─ src/engram/engram.cpp
                                  ▼
                        engrama (.child) + linhagem
```

## Módulos e dependências

| Módulo | Arquivos | Responsabilidade | Depende de |
|---|---|---|---|
| Captação | [capture.hpp](../../src/capture/capture.hpp), [capture.cpp](../../src/capture/capture.cpp) | Fonte → bytes determinísticos | nada do projeto |
| Processamento | [model.hpp](../../src/model/model.hpp), [model.cpp](../../src/model/model.cpp), [rng.hpp](../../src/model/rng.hpp) | Rede, retropropagação, SGD, `learn` | nada do projeto |
| Engrama | [engram.hpp](../../src/engram/engram.hpp), [engram.cpp](../../src/engram/engram.cpp) | Engrama em seções, checksum, linhagem, `fork` | `model.hpp` (só `Config`) |
| CLI | [main.cpp](../../src/main.cpp) | `init`, `fork`, `info`, `feed`, `sample` | os três acima |

Processamento e captação não se conhecem; só o CLI os une.

## Modelo

Embeddings dos 16 bytes anteriores concatenados → `tanh(W1·x + b1)` → softmax(`W2·h + b2`). Vocabulário fixo de 256 (um byte por token). Configuração padrão: embed 32, contexto 16, hidden 128, o que dá 106.880 parâmetros guardados num único vetor plano (ordem: embeddings, W1, b1, W2, b2). O byte 0 é o preenchimento antes do início e o separador entre arquivos.

## Formato do engrama (versão 2, little-endian)

`"CHLD"` · versão `u32` · id/pai/passos `u64` · tamanho do nome `u32` + nome · nº de seções `u32` · seções (tipo `u32`, versão `u32`, tamanho `u64`, dados) · FNV-1a 64 bits de tudo o que precede. A seção tipo 1 é o MLP: embed/context/hidden `u32`, nº de parâmetros `u64` e `float32[]`. Tipo ou versão de seção desconhecidos são recusados nomeando o tipo; o formato 1 é recusado com instrução para recriar a base. A escrita usa arquivo `.tmp` seguido de `rename`. Regras: [`engrama`](../contracts/engrama/v001.json).

## Reprodutibilidade

O build padrão é portátil (`-O3 -ffp-contract=off`); `-DCHILD_NATIVE=ON` acrescenta `-march=native` e deixa de garantir os mesmos pesos entre máquinas. O teste `reference weights` fixa o hash dos pesos de uma semente e um texto conhecidos. Regras: [`reprodutibilidade`](../contracts/reprodutibilidade/v001.json).

## Limites conhecidos (observados, não requisitos)

- Aprendizado de cerca de 8,6 mil bytes/s numa thread no build portátil (9,2 mil com `-march=native`), medido em 22 KB de código.
- Janela fixa de 16 bytes, sem memória além dela.
- Sem proteção contra esquecimento catastrófico (prevista no roteiro, via hipocampo e replay).

# Parte 2 — Direção: uma versão simplificada do cérebro (planejado)

Nada abaixo está implementado, exceto o engrama em seções e a verificação de nomes (REG-01). O primeiro degrau tem contrato aprovado ([`cerebro` v002](../contracts/cerebro/v002.json)), com homeostase, inibição e limiares numéricos tirados de [parametros.md](../../docs/parametros.md). O roteiro completo e os critérios de cada degrau estão na [síntese](../../docs/sintese.md).

## Princípios (decididos)

- **Fidelidade em degraus**: neurônio de ponto AdEx primeiro; dendritos humanos depois.
- **Aprendizado por regras locais e online** (STDP e três fatores com dopamina), sem retropropagação no código novo.
- **Nomes pelas regiões do cérebro**: cada pasta tem um arquivo principal de mesmo nome; identificadores em inglês.
- **CUDA é um backend opcional**; o núcleo não o conhece e o build padrão exige só C++20 e CMake.

## Estrutura-alvo

| Pasta | Estrutura do cérebro | Responsabilidade | Degrau | Substitui |
|---|---|---|---|---|
| `src/thalamus/` | tálamo, relé sensorial | Filtrar e codificar bytes em disparos | 0 | `capture/` |
| `src/neuron/` | neurônio | Dinâmica AdEx: integrar, disparar, reiniciar, adaptar | 0 | parte de `model/` |
| `src/synapse/` | sinapse | Peso, plasticidade STDP e escala homeostática | 0 | parte de `model/` |
| `src/interneuron/` | interneurônio inibitório | Inibir a população e manter o equilíbrio | 0 | novo |
| `src/dopamine/` | dopamina | Surpresa ao prever o próximo byte, o terceiro fator | 1 | novo |
| `src/cerebellum/` | cerebelo | Leitura do próximo byte: expansão esparsa + regra delta local | 2 | parte de `model/` |
| `src/hippocampus/` | hipocampo | Memória rápida esparsa, CA3 e repetição no `sleep` | 4 | novo |
| `src/engram/` | engrama, o traço físico da memória | Engrama em seções, linhagem e `fork` | entregue | `store/` |

Os segmentos dendríticos (degrau 3) entram em `neuron/`. As dependências seguem a regra de dentro para fora: neurônio e sinapse são o núcleo e não conhecem tálamo, cerebelo nem engrama. As pastas legadas (`capture/`, `model/`) saem conforme cada substituta for entregue; `tests/brain_layout.py` verifica os nomes.

## Roteiro

Resumo; os critérios de aceite de cada degrau estão na [síntese](../../docs/sintese.md#roteiro-consolidado).

0. **`cerebro`**: neurônio AdEx, sinapse com STDP, dopamina como terceiro fator, homeostase e inibição.
1. **Dopamina como surpresa**, conferida contra a perda do MLP.
2. **Leitura cerebelar** do próximo byte.
3. **Segmentos dendríticos**: XOR num neurônio e contextos de sequência.
4. **Hipocampo e `sleep`**, contra o esquecimento catastrófico.
5. **Escala e engrama de estado**: medição na RTX 3060 e `fork` com estado dinâmico.

Dendritos com NMDA e picos de cálcio (ModelDB 260178) ficam depois do degrau 3, como refinamento.
