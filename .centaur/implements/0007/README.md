# [0007] Neurônio AdEx e sinapse com STDP

**Data:** 2026-11-02
**Status:** Concluído
**Modo:** TDD
**Spec:** `.centaur/specs/0008/` — Tasks 01 e 02

## Solicitação
"Crie a spec do neurônio AdEx e da sinapse" e "pode executar e incluir no PR #2".

## O que foi feito
- `src/neuron/`: neurônio AdEx de ponto (Euler, passo de 0,1 ms), com padrões de Brette & Gerstner 2005.
- `src/synapse/`: STDP em pares com traços exponenciais decaídos sob demanda, padrões de Song 2000 e corte em [0, w_max]. `on_pre`/`on_post` propõem a mudança e `apply` aplica, deixando espaço para o DOP-01.
- Casos `adex neuron` e `stdp synapse` em `tests/tests.cpp`; as duas fontes entraram no `child_core`.
- README, `AGENTS.md` e `architecture.md` registram que neurônio e sinapse existem.

## Arquivos criados
- `src/neuron/neuron.{hpp,cpp}`, `src/synapse/synapse.{hpp,cpp}`

## Arquivos modificados
- `CMakeLists.txt`, `tests/tests.cpp`, `README.md`, `AGENTS.md`, `.centaur/system/architecture.md`

## Decisões técnicas
- Neurônio e sinapse não se conhecem, e nenhum dos dois conhece o MLP ou o engrama.
- Na sinapse, os traços decaem sob demanda, só a cada evento. Assim ela não precisa de relógio próprio, e o custo de uma sinapse parada é zero.
- O disparo é o cruzamento de V_peak dentro do passo; o potencial nunca fica acima dele.
- O hash do `reference weights` não mudou (o MLP não foi tocado).

## Como validar
`ctest --test-dir build`: os casos `adex neuron` e `stdp synapse` imprimem os intervalos entre disparos e a curva do STDP.

## Contrato e realização
`cerebro@2`: NEU-01 (`src/neuron/*`) e SIN-01 (`src/synapse/*`), implementadas e verificadas. PAR-01 parcial. As evidências afetadas por `tests/tests.cpp`, `CMakeLists.txt` e README foram refeitas. Entrega local, no PR #2.

## Resultado da validação
Ciclos vermelho → verde nos dois casos. `ctest` 3/3, build sem avisos.
- Neurônio: 17 disparos em 500 ms a 1000 pA, com intervalos de 13,9 → 22,2 ms.
- STDP: +0,00389 a +5 ms e −0,00409 a −5 ms; satura exatamente em w_max e em 0.
