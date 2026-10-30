# Arquitetura da base

Fatos extraídos do código em 2026-10-29. As regras normativas estão no contrato [`base`](../contracts/base/v002.json); este documento só explica e vincula.

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
- Sem proteção contra esquecimento catastrófico (lacuna aberta no contrato).

## Decisões futuras em aberto

Fora do contrato [`base`](../contracts/base/v002.json) (que cobre só a base) e sem contrato próprio ainda; viram contrato quando forem abordadas:

- Arquitetura do modelo além do MLP de janela fixa de 16 bytes (por exemplo, atenção) e como medir "capacidade".
- Estratégia contra o esquecimento catastrófico ao alimentar uma ramificação com dados novos.
- Compatibilidade entre versões do formato de checkpoint ao evoluir a arquitetura.
